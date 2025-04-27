#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>
#include <pthread.h>

struct TestMessage {
    std::string from;
    std::string text;
};

std::string mac_to_string(EthernetNIC::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < sizeof(EthernetNIC::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }
    
    return ss.str();
}

Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) : _id(getpid()), _nic(nic), _protocol(protocol) {
    _protocol->register_nic(_nic);

    EthernetProtocol::Address addr(_nic->address(), 0);

    _communicator = new EthernetCommunicator(_protocol, addr);
}

Vehicle::~Vehicle() {
    delete _communicator;
}

void Vehicle::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    _running = true;
    _receive_thread = std::thread(&Vehicle::receive, this);
    _send_thread = std::thread(&Vehicle::send, this);
    ConsoleLogger::log("Threads running.");
}

void Vehicle::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));

    _nic->stop();

    _running = false;

    if (_send_thread.joinable()) {
        _send_thread.join();
    }

    if (_receive_thread.joinable()) {
        _receive_thread.join();
    }
}

void Vehicle::receive() {
    std::string vehicle_mac = Ethernet::address_to_string(_nic->address());

    Message* msg = new Message();

    while (_running) {
        ConsoleLogger::log("RUNNING RECEIVE THREAD");
        
        if (_running && _communicator->receive(msg)) {
            
            TestMessage* data = msg->get_data<TestMessage>();
            if (data != nullptr) {
                ConsoleLogger::log(vehicle_mac);
                ConsoleLogger::log(data->from);
                ConsoleLogger::log(data->text);
                ConsoleLogger::log("Test Message - Vehicle -> " + vehicle_mac + " from: " + data->from + " - text = " + data->text);
            } else {
                ConsoleLogger::log("Error: Received null message data");
            }
        }
    }

    delete msg;

    return;
}

void Vehicle::send() {
    Message* msg = new Message();
    TestMessage* data = msg->get_data<TestMessage>();

    while (_running) {
        ConsoleLogger::log("RUNNING SEND THREAD");
        std::cout << "[PID:" << getpid() << "] RUNNING SEND THREAD" << std::endl;

        std::string from = Ethernet::address_to_string(_nic->address());
        data->from = from;
        data->text = "Mensagem de teste";
        msg->size(sizeof(TestMessage));
        _communicator->send(msg);
    }

    delete msg;

    return;
}