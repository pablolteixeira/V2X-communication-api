#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>

struct TestMessage {
    std::string text;
};

Vehicle::Vehicle(int id, EthernetNIC* nic, EthernetProtocol* protocol) : _id(id), _nic(nic), _protocol(protocol) {
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
    
    while (_running) {
        if (_communicator->receive(msg)) {
            
        }
    }

    delete msg;
    std::cout << "Vehicle " << _id << " thread exited." << std::endl;
}