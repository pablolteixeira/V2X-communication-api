#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/vehicle.h"

#include <sys/wait.h>
#include <execinfo.h>

const unsigned int NUM_VEHICLE = 2;

#include <signal.h>

void signal_handler(int signal_num) {
    std::cerr << "Signal " << signal_num << " received.\n";
    // Print stack trace
    void *array[10];
    size_t size = backtrace(array, 10);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

int main() {
    signal(SIGSEGV, signal_handler);

    ConsoleLogger::init();
    ConsoleLogger::print("STARTING CREATE OF INSTANCES");
    ConsoleLogger::print("Parent process: " + std::to_string(getpid()));
    
    std::cout.setf(std::ios_base::unitbuf);

    pid_t children_pids[NUM_VEHICLE];

    EthernetProtocol* protocol = EthernetProtocol::get_instance();
    
    ConsoleLogger::close();
    for (int i = 0; i < NUM_VEHICLE; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            // Fork failed
            //ConsoleLogger::log("Fork failed for process: " + std::to_string(i));
            continue;
        } else if (pid == 0) {
            ConsoleLogger::close();
            ConsoleLogger::init();
            ConsoleLogger::log("Children process: " + std::to_string(getpid()));

            std::string id = "NIC" + std::to_string(getpid());
            EthernetNIC *nic = new EthernetNIC(id);

            EthernetProtocol* child_protocol = EthernetProtocol::get_instance();

            Vehicle* vehicle = new Vehicle(nic, child_protocol);

            vehicle->start();
            ConsoleLogger::log("Vehicle " + id + " started");
            
            std::this_thread::sleep_for(std::chrono::seconds(15));

            vehicle->stop();
            ConsoleLogger::log("Vehicle " + id + " stopped");

            delete vehicle;
            delete nic;

            ConsoleLogger::close();
            exit(0);
        } else {
            // Parent process
            ConsoleLogger::log("Created child process " + std::to_string(i) + " with PID: " + std::to_string(pid));
            children_pids[i] = pid;
        }
    }
    
    ConsoleLogger::log("Parent waiting for all child processes");
    for (pid_t child_pid : children_pids) {
        int status;
        waitpid(child_pid, &status, 0);
        ConsoleLogger::log("Child process (PID: " + std::to_string(child_pid) + ") has finished with status: " + std::to_string(WEXITSTATUS(status)));
    
        if (WIFEXITED(status)) {
            ConsoleLogger::log("Child process (PID: " + std::to_string(child_pid) + ") exited normally with status: " + std::to_string(WEXITSTATUS(status)));
        } else if (WIFSIGNALED(status)) {
            std::cout << "Child process (PID: " << std::to_string(child_pid) << ") killed by signal: " << std::to_string(WTERMSIG(status)) << std::endl;
            ConsoleLogger::log("Child process (PID: " + std::to_string(child_pid) + ") killed by signal: " + std::to_string(WTERMSIG(status)));
        } else {
            ConsoleLogger::log("Child process (PID: " + std::to_string(child_pid) + ") terminated with unknown status: " + std::to_string(status));
        }
    }

    ConsoleLogger::log("Parent process (PID: " + std::to_string(getpid()) + ") finished.");
    ConsoleLogger::close();

    return 0;
}