#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include "raw_socket_engine.h"
#include "nic.h"
#include "protocol.h"
#include "communicator.h"

typedef NIC<RawSocketEngine> EthernetNIC;
typedef Protocol<EthernetNIC> EthernetProtocol;
typedef Communicator<EthernetProtocol> EthernetCommunicator;

#endif // TYPE_DEFINITIONS_H