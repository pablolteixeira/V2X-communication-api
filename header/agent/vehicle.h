#ifndef VEHICLE_H
#define VEHICLE_H

#include "../autonomous_agent.h"

class SmartData;

class Vehicle: public AutonomousAgent
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
};

#endif // VEHICLE_H