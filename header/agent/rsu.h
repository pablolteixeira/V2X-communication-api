#ifndef RSU_H
#define RSU_H

#include <vector>

#include "../traits.h"
#include "../autonomous_agent.h"
#include "../period_thread.h"

class RSU: public AutonomousAgent
{
public:
    RSU(EthernetNIC* nic, EthernetProtocol* protocol, std::vector<unsigned char> mac_key_vector);
    ~RSU();

    void send_sync_messages();
    void start() override;
    void stop() override;

private:
    PeriodicThread* _running_thread;
    EthernetCommunicator* _communicator;
    std::vector<unsigned char>* _mac_key_vector;
    unsigned short _quadrant;
};

#endif // RSU_H