#include "../header/nic.h"
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

AutonomousAgent::AutonomousAgent(EthernetNIC* nic, EthernetProtocol* protocol) : _id(getpid()), _nic(nic), _protocol(protocol){}

AutonomousAgent::~AutonomousAgent() {
    for(Component* component: _components) {
        delete component;
    }
    // for(SmartData* smart_data: _smart_datas) {
    //     delete smart_data;
    // }
    // for(EthernetCommunicator* communicator: _communicator) {
    //     delete communicator;
    // }
}

void AutonomousAgent::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    ConsoleLogger::log("Starting Components");
    for(Component* component : _components) {
        component->start();
    }
    ConsoleLogger::log("Components started");

    // ConsoleLogger::log("Starting SmartDatas");
    // for(unsigned int i = 0; i < Traits<Vehicle>::NUM_COMPONENTS; i++) {
    //     _smart_datas[i]->start();
    // }
    // ConsoleLogger::log("SmartDatas started");

    _running = true;
    ConsoleLogger::log("Threads running.");
}

void AutonomousAgent::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));

    _nic->stop();

    _running = false;

    for(Component* component: _components) {
        component->stop();
    }

    // for(SmartData* smartdata : _smart_datas) {
    //     smartdata->stop();
    // }
}

EthernetNIC* AutonomousAgent::nic() const { 
    return _nic; 
}