#ifndef VEHICLE_H
#define VEHICLE_H

#include <vector>
#include <chrono>

#include "../traits.h"
#include "../autonomous_agent.h"
#include "../component.h"

// class Component;

class Vehicle: public AutonomousAgent
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol, int lifetime);
    ~Vehicle();
    
    void start() override;
    void stop() override;
    void run();

protected:
    std::vector<Component*> _components;
    int _lifetime;
    U64 _start_time;
};

#endif // VEHICLE_H