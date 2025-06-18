#include "../header/agent/rsu.h"


RSU::RSU(EthernetNIC* nic, EthernetProtocol* protocol, std::vector<Ethernet::MAC_KEY> mac_key_vector)
    : AutonomousAgent(nic, protocol), _mac_key_vector(mac_key_vector), _running_thread(nullptr) {
    ConsoleLogger::log("Initializing NIC MAC KEY data");
    nic->create_mac_key_data(_mac_key_vector);
    ConsoleLogger::log("NIC MAC KEY data initialized");
    ConsoleLogger::log("Quadrant: " + std::to_string(_nic->get_quadrant()));
    _protocol->register_nic(_nic);
    EthernetProtocol::Address address(nic->address(), 1);
    _communicator = new EthernetCommunicator(protocol, address);
}

RSU::~RSU() {
}

void RSU::send_sync_messages() {
    Message* msg = new Message();
    EthernetProtocol::Address from(_nic->address(), _id);
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);
    ConsoleLogger::log("RSU: " + Ethernet::address_to_string(_nic->address()) + " Sending Synchronization Message");
    std::string t1 = _communicator->send(msg, from, to) ? "Success" : "Failure";
    std::string t2 = _communicator->send(msg, from, to) ? "Success" : "Failure";
    ConsoleLogger::log("t1 status: " + t1 + " | t2 status: " + t2);
    delete msg;
}

void RSU::start() {
    if (_running) return;
    ConsoleLogger::log("Starting RSU -> " + std::to_string(_id));
    
    _nic->set_packet_origin(Ethernet::Attributes::PacketOrigin::RSU);

    _running = true;

    _running_thread = new PeriodicThread(
        std::bind(&RSU::send_sync_messages, this),
        static_cast<__u64>(std::chrono::microseconds(1000).count()),
        static_cast<__u64>(std::chrono::microseconds(500).count())
    );
    _running_thread->start();
}

void RSU::stop() {
    if(!_running) return;
    ConsoleLogger::log("Stopping RSU -> " + std::to_string(_id));

    _running = false;
    
    if (_running_thread != nullptr) {
        delete _running_thread;
    }

    delete _communicator;
}