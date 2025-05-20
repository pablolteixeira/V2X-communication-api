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

enum InterestBroadcastType {
    INTERNAL,
    EXTERNAL,
    BOTH
};

struct InterestData {
    ComponentDataType data_type;
    InterestBroadcastType interest_broadcast_type;
    std::chrono::microseconds period;
    std::chrono::microseconds next_receive;
};

// Bit      31  29   27    24     21     18     15     12      9      6      3      0
//         +--+----+----+------+------+------+------+------+------+------+------+------+
// SI      |1 |NUM |MOD |sr+4  |rad+4 |m+4   |kg+4  |s+4   |A+4   |K+4   |mol+4 |cd+4  |
//         +--+----+----+------+------+------+------+------+------+------+------+------+
// Bits     1   2    2     3      3      3      3      3      3      3      3      3

class ComponentDataTypes
{
public:
    static const ComponentDataType METER_DATATYPE = 0b1 << 31 | 9 << 18;
    static const ComponentDataType POSITION_DATA_TYPE = 0b1 << 31 | 10 << 18;
    static const ComponentDataType ANGLE_DATA_TYPE = 0b1 << 31 | 11 << 18;
    static const ComponentDataType DIGITAL_COMAND_TYPE = 0b0 << 31 | 1 << 24;
};



#endif // TYPE_DEFINITIONS_H