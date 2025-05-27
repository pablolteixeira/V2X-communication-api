#ifndef VEHICLE_H
#define VEHICLE_H

#include <vector>
#include "../traits.h"
#include "../autonomous_agent.h"
#include "../component.h"

// class Component;

class Vehicle: public AutonomousAgent
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Vehicle();
    
    void start() override;
    void stop() override;

protected:
    std::vector<Component*> _components;
};

#endif // VEHICLE_H