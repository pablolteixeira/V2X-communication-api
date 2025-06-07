#include "../header/smart_data.h"
#include "../header/agent/vehicle.h"
#include "../header/component/lidar_component.h"
#include "../header/component/gps_component.h"
#include "../header/component/steering_component.h"
#include "../header/component/controller_component.h"


Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol, int lifetime) 
    : AutonomousAgent(nic, protocol), _lifetime(lifetime) {
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
}

void Vehicle::start() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();  
    std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
    _start_time = static_cast<U64>(epoch_time);

    _quadrant = Traits<Vehicle>::pick_random_quadrant();
    
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
}

void Vehicle::run() {
    U64 elapsed_time = 0;
    while(elapsed_time < _lifetime) {
        std::this_thread::sleep_for(std::chrono::seconds(_lifetime/3));
        x = _quadrant.first;

        (_quadrant + 1) % Traits<Vehicle>::NUM_RSU;
        update_quadrant();


        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::time_t epoch_time = std::chrono::system_clock::to_time_t(now);
        U64 epoch = static_cast<U64>(epoch_time);
        elapsed_time = epoch - _start_time;
    }
}