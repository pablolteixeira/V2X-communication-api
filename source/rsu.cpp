#include "../header/agent/rsu.h"


RSU::RSU(EthernetNIC* nic, EthernetProtocol* protocol)
    : AutonomousAgent(nic, protocol) {
    _protocol->register_nic(_nic);
    EthernetProtocol::Address address(nic->address(), 1);
    _communicator = new EthernetCommunicator(protocol, address);
}

RSU::~RSU() {}

void RSU::send_sync_messages() {
    Message* msg = new Message();
    msg->set_type(Message::Type::PTP);
    EthernetProtocol::Address from(_nic->address(), _id);
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);
    ConsoleLogger::log("RSU: " + Ethernet::address_to_string(_nic->address()) + " Sending Synchronization Message");
    std::string t1 = _communicator->send(msg, from, to) ? "Success" : "Failure";
    std::string t2 = _communicator->send(msg, from, to) ? "Success" : "Failure";
    ConsoleLogger::log("t1 status: " + t1 + " | t2 status: " + t2);
}

void RSU::start() {
    if (!_running) return;
    _nic->set_time_keeper_packet_origin(Ethernet::Metadata::PacketOrigin::RSU);
    
    _running_thread = new PeriodicThread(
        std::bind(&RSU::send_sync_messages, this),
        static_cast<__u64>(std::chrono::microseconds(1000).count()),
        static_cast<__u64>(std::chrono::microseconds(400).count())
    );
    _running_thread->start();}

void RSU::stop() {
    ConsoleLogger::log("Stopping RSU -> " + std::to_string(_id));

    _nic->stop();

    _running = false;

    _running_thread->stop();
}