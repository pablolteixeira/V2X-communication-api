#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/vehicle.h"

#include <sys/wait.h>

const unsigned int NUM_VEHICLE = 10;

int main() {
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
            ConsoleLogger::log("Vehicle " + id + " created");
            
            vehicle->stop();

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
    
    for (pid_t child_pid : children_pids) {
        int status;
        waitpid(child_pid, &status, 0);
        ConsoleLogger::log("Child process (PID: " + std::to_string(child_pid) + ") has finished with status: " + std::to_string(WEXITSTATUS(status)));
    }

    //delete protocol;
    
    ConsoleLogger::log("Parent process (PID: " + std::to_string(getpid()) + ") finished.");
    ConsoleLogger::close();

    return 0;
}