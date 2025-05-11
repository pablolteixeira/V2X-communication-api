#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>
#include <pthread.h>

std::string mac_to_string(Ethernet::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }
    
    return ss.str();
}

Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) : _id(getpid()), _nic(nic), _protocol(protocol), _semaphore(0) {
    _protocol->register_nic(_nic);

    for(int i = 0; i < 5; i++){
        EthernetProtocol::Address component_addr(_nic->address(), i+1);

        _communicator = new EthernetCommunicator(_protocol, component_addr);
        _components[i] = new Component(this, i+1, _communicator);
    }
}

Vehicle::~Vehicle() {
    delete _communicator;
    
    for(Component* component: _components) {
        delete component;
    }
}

void Vehicle::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    for(Component* component : _components) {
        component->start();
    }

    _running = true;
    // _receive_thread = std::thread(&Vehicle::receive, this);
    // _send_thread = std::thread(&Vehicle::send, this);
    ConsoleLogger::log("Threads running.");
}

void Vehicle::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));

    _nic->stop();

    _running = false;

    for(Component* component: _components) {
        component->stop_send();
    }

    for(Component* component: _components) {
        component->stop_receive();
    }
}

EthernetNIC* Vehicle::nic() const { 
    return _nic; 
}