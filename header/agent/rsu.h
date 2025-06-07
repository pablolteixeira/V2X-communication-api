#ifndef RSU_H
#define RSU_H

#include "../traits.h"
#include "../autonomous_agent.h"
#include "../period_thread.h"
#include "../mac_key_table.h"

class RSU: public AutonomousAgent
{
public:
    RSU(EthernetNIC* nic, EthernetProtocol* protocol, MacKeyTable mac_key_table);
    ~RSU();

    void send_sync_messages();
    void start() override;
    void stop() override;

private:
    PeriodicThread* _running_thread;
    EthernetCommunicator* _communicator;
    MacKeyTable _mac_key_table;
    unsigned short _quadrant;
};

#endif // RSU_H