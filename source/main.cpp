#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/vehicle.h"

int main() {
    EthernetProtocol* protocol = EthernetProtocol::get_instance();

    EthernetNIC* nic1 = new EthernetNIC("NIC1");
    //EthernetNIC* nic2 = new EthernetNIC("NIC2");

    Vehicle* v = new Vehicle(1, nic1, protocol);
    //Vehicle* v2 = new Vehicle(2, nic2, protocol);

    v->start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    v->stop();

    return 0;
}