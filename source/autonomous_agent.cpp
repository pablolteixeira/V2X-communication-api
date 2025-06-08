#include "../header/smart_data.h"
#include "../header/autonomous_agent.h"

std::string mac_to_string(Ethernet::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }
    
    return ss.str();
}

AutonomousAgent::AutonomousAgent(EthernetNIC* nic, EthernetProtocol* protocol) : _id(getpid()), _nic(nic), _protocol(protocol), _running(false) {}

AutonomousAgent::~AutonomousAgent() {}

EthernetNIC* AutonomousAgent::nic() const { 
    return _nic; 
}