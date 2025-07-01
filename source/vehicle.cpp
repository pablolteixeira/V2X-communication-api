#include "../header/smart_data.h"
#include "../header/agent/vehicle.h"
#include "../header/component/lidar_component.h"
#include "../header/component/gps_component.h"
#include "../header/component/steering_component.h"
#include "../header/component/controller_component.h"
#include "../header/component/accelerometer_component.h"

Vehicle::Vehicle(int id, EthernetNIC* nic, EthernetProtocol* protocol, int lifetime) 
    : AutonomousAgent(nic, protocol), _lifetime(lifetime) {
    _protocol->register_nic(_nic);
    _id = id;
    
    ConsoleLogger::log("Vehicle: dataset id " + std::to_string(_id) + " set");

    _components.push_back(new LidarComponent(this, 1));
    _components.push_back(new GPSComponent(this, 2));
    _components.push_back(new ControllerComponent(this, 3));
    _components.push_back(new SteeringComponent(this, 4));
    _components.push_back(new AccelerometerComponent(this, 5, _id));
}

Vehicle::~Vehicle() {
}

void Vehicle::start() {
    _start_time = std::chrono::system_clock::now();
    _nic->set_quadrant(Traits<Vehicle>::pick_random_quadrant());
    
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
        ConsoleLogger::log("VEHICLE: DELETING COMPONENT BEFORE");
        component->stop();
        delete component;
        ConsoleLogger::log("VEHICLE: DELETING COMPONENT AFTER");
    }
}

void Vehicle::run() {
    auto sleep_duration = std::chrono::seconds(_lifetime);

    while (std::chrono::system_clock::now() - _start_time < sleep_duration) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    
        auto quad = _nic->get_quadrant();
        quad = (quad % Traits<Vehicle>::NUM_RSU) + 1;

        _nic->set_quadrant(quad);
    }
}