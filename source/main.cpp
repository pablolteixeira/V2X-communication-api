#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/vehicle.h"

#include <sys/wait.h>

const unsigned int NUM_VEHICLE = 2;

int main() {
    ConsoleLogger::print("STARTING CREATE OF INSTANCES");

    pid_t children_pids[NUM_VEHICLE];

    EthernetProtocol* protocol = EthernetProtocol::get_instance();

    //std::cout << "Father process: " << getpid() << std::endl;

    EthernetNIC *nic1 = new EthernetNIC("1");
    EthernetNIC *nic2 = new EthernetNIC("2");

    Vehicle* vehicle1 = new Vehicle(nic1, protocol);
    Vehicle* vehicle2 = new Vehicle(nic2, protocol);
    
    ConsoleLogger::print("FINISHING CREATION OF INSTANCES");


    std::cout << std::endl;

    
    vehicle1->start();
    vehicle2->start();
    
    // for (int i = 0; i < NUM_VEHICLE; i++) {
    //     pid_t pid = fork();
        
    //     if (pid < 0) {
    //         // Fork failed
    //         std::cout << "Fork failed for process: " << i << std::endl;
    //         continue;
    //     } else if (pid == 0) {
    //         std::cout << "Children process: " << getpid() << std::endl;

    //         std::string id = "NIC" + std::to_string(getpid());
    //         EthernetNIC *nic = new EthernetNIC(id);

    //         Vehicle* vehicle = new Vehicle(nic, protocol);

    //         vehicle->start();
    //         std::cout << "Vehicle " << id << " created" << std::endl;
            
    //         delete vehicle;
    //         delete nic;
    //         exit(0);
    //     } else {
    //         // Parent process
    //         std::cout << "Created child process " << i << " with PID: " << pid << std::endl;
    //         children_pids[i] = pid;
    //     }
    // }

    
    
    // for (pid_t child_pid : children_pids) {
    //     int status;
    //     waitpid(child_pid, &status, 0);
    //     std::cout << "Child process (PID: " << child_pid << ") has finished with status: " 
    //               << WEXITSTATUS(status) << std::endl;
    // }

    // std::cout << "Parent process (PID: " << getpid() << ") finished." << std::endl;

    return 0;
}