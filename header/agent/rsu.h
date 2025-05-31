#ifndef RSU_H
#define RSU_H

#include "../traits.h"
#include "../autonomous_agent.h"
#include "../period_thread.h"

class RSU: public AutonomousAgent
{
public:
    RSU(EthernetNIC* nic, EthernetProtocol* protocol);
    ~RSU();

    void send_sync_messages();
    void start() override;
    void stop() override;

private:
    PeriodicThread* _running_thread;
    EthernetCommunicator* _communicator;
};

#endif // RSU_H