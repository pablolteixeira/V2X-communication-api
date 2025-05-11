#ifndef TYPE_DEFINITIONS_H
#define TYPE_DEFINITIONS_H

#include "raw_socket_engine.h"
#include "nic.h"
#include "protocol.h"
#include "communicator.h"

typedef NIC<RawSocketEngine> EthernetNIC;
typedef Protocol<EthernetNIC> EthernetProtocol;
typedef Communicator<EthernetProtocol> EthernetCommunicator;

typedef unsigned int ComponentDataType;

// Bit      31  29   27    24     21     18     15     12      9      6      3      0
//         +--+----+----+------+------+------+------+------+------+------+------+------+
// SI      |1 |NUM |MOD |sr+4  |rad+4 |m+4   |kg+4  |s+4   |A+4   |K+4   |mol+4 |cd+4  |
//         +--+----+----+------+------+------+------+------+------+------+------+------+
// Bits     1   2    2     3      3      3      3      3      3      3      3      3

#endif // TYPE_DEFINITIONS_H