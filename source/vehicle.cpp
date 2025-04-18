#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>

struct TestMessage {
    std::string from;
    std::string text;
};

std::string mac_to_string(EthernetNIC::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < sizeof(EthernetNIC::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }
    
    return ss.str();
}

Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) : _nic(nic), _protocol(protocol) {
    _protocol->register_nic(_nic);

    EthernetProtocol::Address addr(_nic->address(), 0);

    _communicator = new EthernetCommunicator(_protocol, addr);
}

Vehicle::~Vehicle() {
    delete _communicator;
}

void Vehicle::start() {
    std::cout << "Starting Vehicle -> " << _id << std::endl;
    
    if (_running) {
        std::cout << "Running: " << _running << std::endl;
        return;
    }
    _running = true;
    _thread = std::thread(&Vehicle::run, this);
    std::cout << "Thread running." << std::endl;

    Message* msg = new Message();
    TestMessage* data = msg->get_data<TestMessage>();
    std::string from = Ethernet::address_to_string(_nic->address());
    data->from = from;
    data->text = "Mensagem de teste";
    msg->size(sizeof(TestMessage));
    _communicator->send(msg);
}

void Vehicle::stop() {
    std::cout << "Stopping Vehicle -> " << _id << std::endl;

    if (!_running) return;
    _running = false;
    if (_thread.joinable()) {
        _thread.join();
    }
}

void Vehicle::run() {
    Message* msg = new Message(); // Buffer to hold incoming messages

    std::string vehicle_mac = Ethernet::address_to_string(_nic->address());

    while (_running) {
        if (_communicator->receive(msg)) {
            TestMessage* data = msg->get_data<TestMessage>();
            std::cout << "Test Message - Vehicle -> " << vehicle_mac << " from: " << data->from << " - text = " << data->text << std::endl;
        }
    }

    delete msg;
    std::cout << "Vehicle " << _id << " thread exited." << std::endl;
}