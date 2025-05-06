#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>
#include <pthread.h>

std::string mac_to_string(Ethernet::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }
    
    return ss.str();
}

Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) : _id(getpid()), _nic(nic), _protocol(protocol), _semaphore(0) {
    _protocol->register_nic(_nic);

    for(int i = 0; i < 5; i++){
        EthernetProtocol::Address component_addr(_nic->address(), i+1);

        _communicator = new EthernetCommunicator(_protocol, component_addr);
        _components[i] = new Component(this, i+1, _communicator);
    }
}

Vehicle::~Vehicle() {
    delete _communicator;
    
    for(Component* component: _components) {
        delete component;
    }
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
    // _receive_thread = std::thread(&Vehicle::receive, this);
    // _send_thread = std::thread(&Vehicle::send, this);
    ConsoleLogger::log("Threads running.");
}

void Vehicle::stop() {
    ConsoleLogger::log("Stopping Vehicle -> " + std::to_string(_id));

    _nic->stop();

    _running = false;
    for(Component* component: _components) {
        component->stop();
    }

    // if (_send_thread.joinable()) {
    //     _send_thread.join();
    // }

    // _semaphore.v();

    // if (_receive_thread.joinable()) {
    //     _receive_thread.join();
    // }
}

void Vehicle::receive() {
    ConsoleLogger::log("Vehicle receive started.");
    std::string vehicle_mac = Ethernet::address_to_string(_nic->address());

    Message* msg = new Message();
    
    while (_running) {
        ConsoleLogger::log("RUNNING RECEIVE THREAD");
        /*
        if (_running && _communicator->receive(msg)) {
            ConsoleLogger::log("MESSAGE RECEIVE FROM COMMUNICATOR");    
            ComponentMessage* component_message = msg->get_data<ComponentMessage>();
            // Verify the destination address

            ConsoleLogger::log("ARRIVED MESSAGE!");
            BROADCAST LOCAL (COMPONENT TO LOCAL COMPONENTS) - MESMO MAC E PORTA DE DESTINO 0
            UNICAST LOCAL (COMPONENT TO COMPONENT) - MESMO MAC E PORTA DE DESTINO > 0
            BROADCAST EXTERNO (COMPONENT TO ALL EXTERNAL COMPONENTS) - BROADCAST MAC E PORTA DE DESTINO 0
            UNICAST EXTERNO (COMPONENT TO EXTERNAL COMPONENT) - MAC DIFERENTE DE LOCAL E BROACAST 
            if (memcmp(component_message->to_addr, _nic->address(), sizeof(Ethernet::Address)) || memcmp(component_message->to_addr, Ethernet::BROADCAST_MAC, sizeof(Ethernet::Address))) {
                Message* data;
                // Broadcast
                if(component_message->to_port == 0) {
                    ConsoleLogger::log("BROADCAST: " + std::to_string(component_message->to_port));
                    data = msg;
                    for(Component* component: _components){
                        component->notify(data);
                    }
                } else {
                    // Unicast
                    ConsoleLogger::log("UNICAST TO COMPONENT: " + std::to_string(component_message->to_port));
                    if (data) {
                        data = msg;
                        for(Component* component: _components){
                            if (component_message->to_port == component->id()) {
                                component->notify(data);
                                break;
                            }
                        }
                    } else {
                        ConsoleLogger::log("REFFERENCE BUFFER IS FULL!");
                    }
                }
            }
        }*/
    }

    delete msg;

    ConsoleLogger::log("Vehicle receive started.");
}

void Vehicle::send() {
    ConsoleLogger::log("Vehicle send started.");

    while (_running) {
        ConsoleLogger::log("RUNNING SEND THREAD");
        // std::cout << "[PID:" << getpid() << "] RUNNING SEND THREAD" << std::endl;
        /*_semaphore.p();
        ConsoleLogger::log("VEHICLE SEND P");
        
        if (!_running) {
            break;
        }
        Message* msg = _queue.remove();      

        if (msg) {
            ComponentMessage* data = msg->get_data<ComponentMessage>();
        
            EthernetProtocol::Address from(data->from_addr, data->from_port);
            EthernetProtocol::Address to(data->to_addr, data->to_port);

            _communicator->send(msg, from, to);

            delete msg;
        }*/
    }

    ConsoleLogger::log("Vehicle send finished.");
}

/*void Vehicle::free(Message* msg) {
    ConsoleLogger::log("FREE REFERENCE BUFFER");

    _reference_buffer.free(msg);
}*/

/* void Vehicle::notify(Message* msg) {
    if (!_running) {
        return;
    }

    ConsoleLogger::log("NOTIFY VEHICLE");
    Message* data;
    data = msg;
    
    bool added = _queue.add(data);
    if (added) {
        _semaphore.v();
    }
}
 */
