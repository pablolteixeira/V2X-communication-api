#ifndef SMART_DATA_H
#define SMART_DATA_H

#include "type_definitions.h"
#include "vehicle.h"
#include <thread>

class SmartData {
public:
    SmartData(Component* component, EthernetCommunicator* communicator) : _component(component), _communicator(communicator), _semaphore(0) {}
    ~SmartData() {}

    void start() {
        
    }

    void stop() {

    }

private:
    void receive() {
        Message* msg = new Message();

        while (_running) {
            _communicator->receive(msg);
            if (!_running) {
                break;
            }
            
            // Process message based on type
            ComponentMessage* component_msg = msg->get_data<ComponentMessage>();

        }

        delete msg;
    }

    void send() {
        //ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send started.");

        Message* msg = new Message();
        
        while (_running) {
            _semaphore.p();
            int value = *_queue.remove();
            
            

            ComponentMessage* data = msg->get_data<ComponentMessage>();
            //std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            if (!_running) {
                break;
            }

            memcpy(data->origin_addr, _component->get_address(), sizeof(Ethernet::Address));
            data->origin_port = _component->id();

            EthernetProtocol::Address from(_component->get_address(), _component->id());
            EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

            msg->size(sizeof(ComponentMessage));

            _communicator->send(msg, from, to);
        }
        
        //ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send stopped.");
    }

private:
    bool _running;
    EthernetCommunicator* _communicator;
    Component* _component;
    std::thread _receive_thread;
    std::thread _send_thread;

    Semaphore _semaphore;
    Queue<int, 32> _queue;
};

#endif // SMART_DATA_H