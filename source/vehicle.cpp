#include "../header/vehicle.h"
#include "../header/nic.h"
#include "../header/components/sensor_component.h"
#include "../header/components/reciever_component.h"

#include <iostream>
#include <pthread.h>



struct TestMessage {
    EthernetProtocol::Address from;
    EthernetProtocol::Address to;
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

    Sensor *sensor = new Sensor(_id+":1", SensorMessage::LIDAR, 1);
    MessageReceiver *reciever = new MessageReceiver(_id+":2", _communicator);
    _components.push_back(sensor);
    _components.push_back(reciever);
    
}

Vehicle::~Vehicle() {
    delete _communicator;
    
    for(Component* component: _components) {
        delete component;
    }
    _components.clear();
}

void Vehicle::start() {
    ConsoleLogger::log("Starting Vehicle -> " + std::to_string(_id));
    
    if (_running) {
        ConsoleLogger::log("Running: " + std::to_string(_running));
    }
    for(Component* component : _components) {
        component->start();
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
    for(Component* component: _components) {
        component->stop();
    }

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

                std::string origin = mac_to_string(data->from.paddr());

                ConsoleLogger::log("Test Message - Vehicle -> " + vehicle_mac + " origin: " + origin);
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

        unsigned char BROADCAST[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

        EthernetProtocol::Address from(_nic->address(), 1);
        EthernetProtocol::Address to(BROADCAST, 2);

        data->from = from;
        data->to = to;
        msg->size(sizeof(TestMessage));

        _communicator->send(msg, data->from, data->to);
    }    

    delete msg;

    return;
}