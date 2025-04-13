#ifndef VEHICLE_H
#define VEHICLE_H

#include "traits.h"
#include "nic.h"
#include "raw_socket_engine.h"
#include "protocol.h"
#include "communicator.h"
#include "message.h"

typedef NIC<RawSocketEngine> EthernetNIC;
typedef Protocol<EthernetNIC> EthernetProtocol;
typedef Communicator<EthernetProtocol> EthernetCommunicator;

class Vehicle
{
public:
    Vehicle(int id, EthernetNIC* nic);
    ~Vehicle();

    void start();
private:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    EthernetCommunicator* _communicator;
};

#endif // VEHICLE_H