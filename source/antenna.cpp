#include "../header/agent/antenna.h"

Antenna::Antenna(EthernetNIC* nic, EthernetProtocol* protocol)
    : AutonomousAgent(nic, protocol) {
    _protocol->register_nic(_nic);

    //_components.push_back((this, 1));
    //_components.push_back(TimeSyncEmitter(this, 1));
}