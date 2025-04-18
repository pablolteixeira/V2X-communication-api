#ifndef VEHICLE_H
#define VEHICLE_H

#include "traits.h"
#include "nic.h"
#include "raw_socket_engine.h"
#include "protocol.h"
#include "communicator.h"
#include "message.h"

#include <thread>

typedef NIC<RawSocketEngine> EthernetNIC;
typedef Protocol<EthernetNIC> EthernetProtocol;
typedef Communicator<EthernetProtocol> EthernetCommunicator;

class Vehicle
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Vehicle();

    void start();
    void stop();
private:
    void run();

private:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    EthernetCommunicator* _communicator;

    bool _running = false;
    std::thread _thread;
};

#endif // VEHICLE_H