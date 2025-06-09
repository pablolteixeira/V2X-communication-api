#ifndef AUTONOMOUS_AGENT_H
#define AUTONOMOUS_AGENT_H

#include <thread>
#include <atomic>
#include <random>
#include <iostream>
#include <pthread.h>

#include "types.h"
#include "nic.h"

class AutonomousAgent
{
public:
    AutonomousAgent(EthernetNIC* nic, EthernetProtocol* protocol);
    virtual ~AutonomousAgent();

    virtual void start() = 0;
    virtual void stop() = 0;

    EthernetNIC* nic() const;

protected:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    
    bool _running;
};

#endif // AUTONOMOUS_AGENT_H