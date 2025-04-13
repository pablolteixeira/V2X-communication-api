#include "../header/communicator.h"
#include "../header/protocol.h"
#include "../header/traits.h"
#include "../header/ethernet.h"
#include "../header/nic.h"
#include "../header/raw_socket_engine.h"
#include "../header/vehicle.h"

struct TestMessage {
    std::string text;
};

int main() {
    EthernetProtocol* protocol = EthernetProtocol::get_instance();

    // lista de carros - mensagens iguais (de carro id_x - chegou em carro id_y - mensagem)
    EthernetNIC* nic1 = new EthernetNIC("NIC1");
    EthernetNIC* nic2 = new EthernetNIC("NIC2");

    protocol->register_nic(nic1);
    protocol->register_nic(nic2);

    EthernetProtocol::Address addr1(nic1->address(), 0);
    EthernetProtocol::Address addr2(nic2->address(), 0);

    EthernetCommunicator* communicator1 = new EthernetCommunicator(protocol, addr1);
    EthernetCommunicator* communicator2 = new EthernetCommunicator(protocol, addr2);

    Message* msg = new Message();
    TestMessage* data = msg->get_data<TestMessage>();
    data->text = "Mensagem de teste!";
    communicator1->send(msg);

    //Vehicle v1 = Vehicle(1, nic1);
    //Vehicle v2 = Vehicle(2, nic2);

    //v1.start();
    //v2.start();

    return 0;
}