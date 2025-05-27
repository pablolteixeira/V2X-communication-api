#ifndef ANTENNA_H
#define ANTENNA_H

#include "../autonomous_agent.h"
#include "../traits.h"

class SmartData;

class Antenna: public AutonomousAgent
{
public:
    Antenna(EthernetNIC* nic, EthernetProtocol* protocol);
};

#endif // VEHICLE_H