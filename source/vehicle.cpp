#include "../header/smart_data.h"
#include "../header/agent/vehicle.h"
#include "../header/component/lidar_component.h"
#include "../header/component/gps_component.h"
#include "../header/component/steering_component.h"
#include "../header/component/controller_component.h"


Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) 
    : AutonomousAgent(nic, protocol) {
    _protocol->register_nic(_nic);
    
    _components.push_back(new LidarComponent(this, 1));
    _components.push_back(new GPSComponent(this, 2));
    _components.push_back(new ControllerComponent(this, 3));
    _components.push_back(new SteeringComponent(this, 4));
}

Vehicle::~Vehicle() {
    for(Component* component: _components) {
        delete component;
    }
    delete _nic;
}

void Vehicle::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    ConsoleLogger::log("Starting Components");
    for(Component* component : _components) {
        component->start();
    }
    ConsoleLogger::log("Components started");    

    _running = true;
    ConsoleLogger::log("Threads running.");
}

void Vehicle::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));
    _running = false;
    
    for(Component* component: _components) {
        component->stop();
    }

    _nic->stop();
}