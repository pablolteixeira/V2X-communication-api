#ifndef VEHICLE_H
#define VEHICLE_H

#include "traits.h"
#include "nic.h"
#include "raw_socket_engine.h"
#include "protocol.h"
#include "communicator.h"
#include "message.h"
#include "components/component.h"

#include <thread>
#include <atomic>

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
    void receive();
    void send();

private:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    EthernetCommunicator* _communicator;

    std::atomic<bool> _running;
    
    std::thread _receive_thread;
    std::thread _send_thread;
    std::list<Component*> _components;
};

#endif // VEHICLE_H