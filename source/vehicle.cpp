#include "../header/vehicle.h"
#include "../header/nic.h"
#include "../header/smart_data.h"
#include "../header/component/lidar_component.h"

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

    for(int i = 0; i < Traits<Vehicle>::NUM_COMPONENTS; i++){
        ConsoleLogger::log("New component");
        EthernetProtocol::Address component_addr(_nic->address(), i+1);

        _communicator[i] = new EthernetCommunicator(_protocol, component_addr);
        _components[i] = new LidarComponent(this, i+1);
        _smart_datas[i] = new SmartData(_components[i], _communicator[i]);
    }
}

Vehicle::~Vehicle() {
    for(Component* component: _components) {
        delete component;
    }
    for(SmartData* smart_data: _smart_datas) {
        delete smart_data;
    }
    for(EthernetCommunicator* communicator: _communicator) {
        delete communicator;
    }
}

void Vehicle::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    ConsoleLogger::log("Starting Components");
    for(int i = 0; i < Traits<Vehicle>::NUM_COMPONENTS; i++) {
        _components[i]->start();
        _smart_datas[i]->start();
    }
    ConsoleLogger::log("Components started");

    _running = true;
    ConsoleLogger::log("Threads running.");
}

void Vehicle::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));

    _nic->stop();

    _running = false;

    for(Component* component: _components) {
        component->stop();
    }
}

EthernetNIC* Vehicle::nic() const { 
    return _nic; 
}