#ifndef AUTONOMOUS_AGENT_H
#define AUTONOMOUS_AGENT_H

#include <thread>
#include <atomic>
#include <random>
#include <iostream>
#include <vector>
#include <pthread.h>

#include "ethernet.h"
#include "component.h"
#include "types.h"

class Component;

class AutonomousAgent
{
public:
    AutonomousAgent(EthernetNIC* nic, EthernetProtocol* protocol);
    ~AutonomousAgent();

    void start();
    void stop();

    EthernetNIC* nic() const;

protected:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    
    std::atomic<bool> _running;

    std::vector<Component*> _components;
    // EthernetCommunicator* _communicator[Traits<Vehicle>::NUM_COMPONENTS];
    // SmartData* _smart_datas[Traits<Vehicle>::NUM_COMPONENTS];
};

#endif // AUTONOMOUS_AGENT_H