#ifndef ANTENNA_H
#define ANTENNA_H

#include "../traits.h"
#include "../autonomous_agent.h"
#include "../period_thread.h"

class Antenna: public AutonomousAgent
{
public:
    Antenna(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Antenna();

    void send_sync_messages();
    void start() override;
    void stop() override;

private:
    PeriodicThread* _running_thread;
    EthernetCommunicator* _communicator;
};

#endif // ANTENNA_H