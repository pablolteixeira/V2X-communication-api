#include <sys/wait.h>
#include <execinfo.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <random>

#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/agent/vehicle.h"
#include "../header/agent/rsu.h"
#include "../header/mac_key_table.h"

constexpr int MAX_RUNTIME_SECONDS = 5; // total simulation time for the parent process (e.g., 5 min)
constexpr int SPAWN_INTERVAL_MS = 500;  // interval between spawns (in milliseconds)
constexpr int MIN_LIFETIME = 4;         // min vehicle lifetime (in seconds)
constexpr int MAX_LIFETIME = 6;         // max vehicle lifetime (in seconds)

int main() {
    srand(time(nullptr));
    ConsoleLogger::init();
    ConsoleLogger::print("DYNAMIC VEHICLE SPAWNER STARTED");
    ConsoleLogger::print("Parent process: " + std::to_string(getpid()));
    std::cout.setf(std::ios_base::unitbuf);

    auto start_time = std::chrono::steady_clock::now();

    std::cout << "\n---------------------------------------------------------------" << std::endl;
    std::cout << "Running MAIN Simulation" << std::endl;
    std::cout << "PARAMETERS:" << std::endl;
    std::cout << "Simulation runtime: " << MAX_RUNTIME_SECONDS << "s" << std::endl;
    std::cout << "Vehicle spawn interval: " << SPAWN_INTERVAL_MS << "ms" << std::endl;
    std::cout << "Vehicle lifetime span: " << MIN_LIFETIME << " | " << MAX_LIFETIME << " s" << std::endl;
    std::cout << "\n---------------------------------------------------------------\n" << std::endl;
    
    std::vector<unsigned char>* mac_key_vector;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    mac_key_vector->clear()
    mac_key_vector->reserve(Traits<RSU>::NUM_RSU);

    for (int i = 0; i < Traits<RSU>::NUM_RSU; ++i) {
        Ethernet::MAC_KEY key;

        for (size_t j = 0; j < Ethernet::MAC_BYTE_SIZE; ++j) {
            key[j] = static_cast<unsigned char>(distrib(gen));
        }

        mac_key_vector->push_back(key);
    }

    for (unsigned short i = 0; i < Traits<RSU>::NUM_RSU; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            ConsoleLogger::log("Fork failed.");
        } else if (pid == 0) {
            ConsoleLogger::close();
            ConsoleLogger::init();

            pid_t child_pid = getpid();
            ConsoleLogger::log("Child process created: PID = " + std::to_string(child_pid));
            
            std::string id = "NIC" + std::to_string(child_pid);
            EthernetNIC* nic = new EthernetNIC(id, i);
            EthernetProtocol* protocol = EthernetProtocol::get_instance();  
            RSU* rsu = new RSU(nic, protocol, mac_key_vector);
            rsu->start();
            std::this_thread::sleep_for(std::chrono::seconds(MAX_RUNTIME_SECONDS));
            rsu->stop();

            delete rsu;
            delete nic;
            ConsoleLogger::close();
            exit(0);
        }       
    }
    
    while (true) {
        // Check if the parent should stop
        auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now() - start_time).count();
        
        if (elapsed >= MAX_RUNTIME_SECONDS) {
            ConsoleLogger::log("Max runtime reached. Stopping dynamic spawner.");
            break;
        }
        
        pid_t pid = fork();
        
        if (pid < 0) {
            ConsoleLogger::log("Fork failed.");
        } else if (pid == 0) {
            // --- Child process ---
            ConsoleLogger::close();
            ConsoleLogger::init();
            
            pid_t child_pid = getpid();
            ConsoleLogger::log("Child process created: PID = " + std::to_string(child_pid));
            
            std::string id = "NIC" + std::to_string(child_pid);
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> distrib(1, Traits<Vehicle>::NUM_RSU);
            
            EthernetNIC* nic = new EthernetNIC(id, static_cast<unsigned short>(distrib(gen)));
            EthernetProtocol* child_protocol = EthernetProtocol::get_instance();
            
            int lifetime = MIN_LIFETIME + rand() % (MAX_LIFETIME - MIN_LIFETIME + 1);
            ConsoleLogger::log("Vehicle " + id + " will live for " + std::to_string(lifetime) + " seconds");
            Vehicle* vehicle = new Vehicle(nic, child_protocol, lifetime);
            
            vehicle->start();
            ConsoleLogger::log("Vehicle " + id + " started");
            
            vehicle->run();

            
            vehicle->stop();
            ConsoleLogger::log("Vehicle " + id + " stopped");
            
            delete nic;
            delete vehicle;
            
            ConsoleLogger::log("Vehicle " + id + " destroyed after " + std::to_string(lifetime) + " seconds");
            
            ConsoleLogger::close();
            exit(0);
        } else {
            // --- Parent process ---
            ConsoleLogger::log("Spawned vehicle process with PID: " + std::to_string(pid));
            std::cout << "Spawned vehicle process with PID: " << std::to_string(pid) << std::endl;
            
            // Reap any finished child processes to avoid zombies
            int status;
            while (waitpid(-1, &status, WNOHANG) > 0) {
                if (WIFEXITED(status)) {
                    ConsoleLogger::log("Child exited normally with status: " + std::to_string(WEXITSTATUS(status)));
                } else if (WIFSIGNALED(status)) {
                    ConsoleLogger::log("Child killed by signal: " + std::to_string(WTERMSIG(status)));
                } else {
                    ConsoleLogger::log("Child exited with unknown status.");
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(SPAWN_INTERVAL_MS));
    }
    
    // Wait for all remaining children before exiting
    ConsoleLogger::log("Waiting for remaining child processes...");
    int status;
    while (waitpid(-1, &status, 0) > 0) {
        if (WIFEXITED(status)) {
            ConsoleLogger::log("Child exited normally with status: " + std::to_string(WEXITSTATUS(status)));
        } else if (WIFSIGNALED(status)) {
            ConsoleLogger::log("Child killed by signal: " + std::to_string(WTERMSIG(status)));
        } else {
            ConsoleLogger::log("Child exited with unknown status.");
        }
    }
        
    ConsoleLogger::log("All child processes terminated.");
    ConsoleLogger::log("Parent process (PID: " + std::to_string(getpid()) + ") finished.");
    ConsoleLogger::close();
    
    return 0;
}
