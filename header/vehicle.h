#ifndef VEHICLE_H
#define VEHICLE_H

#include "ethernet.h"
#include "traits.h"
#include "type_definitions.h"
#include "message.h"
#include "reference_buffer.h"
#include "queue.h"

#include <thread>
#include <atomic>
#include <random>

class Component;

class Vehicle
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Vehicle();

    void start();
    void stop();
    void free(Message* msg);

    EthernetNIC* nic() const { return _nic; }
private:
    void receive();
    void send();

private:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    EthernetCommunicator* _communicator;

    std::atomic<bool> _running;
    
    std::thread _receive_thread;
    std::thread _send_thread;
    Component* _components[5];

    Semaphore _semaphore;
    Queue<Message, 16> _queue;
};

struct ComponentMessage {
    Ethernet::Address origin_addr;
    unsigned short origin_port;
    int id;
};

// Component base class
class Component {
    public:
        Component(Vehicle* vehicle, const unsigned short& id, EthernetCommunicator* communicator) : _id(id), _running(false), _semaphore(0), _vehicle(vehicle), _communicator(communicator) {}
        ~Component() {}
    
        void start() {
            if (_running) return;
            _running = true;
            _receive_thread = std::thread(&Component::receive, this);
            _send_thread = std::thread(&Component::send, this);
        }
    
        void stop_send() {
            if (!_running) return;
            _running = false;
            if (_send_thread.joinable()) {
                _send_thread.join();
            }
        }

        void stop_receive() {
            if (_running) return;
            _communicator->stop();
            if (_receive_thread.joinable()) {
                _receive_thread.join();
            }
        }

        void receive() {
            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive started.");
            
            Message* msg = new Message();

            while (_running) {
                _communicator->receive(msg);
                if (!_running) {
                    break;
                }
                
                // Process message based on type
                ComponentMessage* component_msg = msg->get_data<ComponentMessage>();

                std::string origin = mac_to_string(component_msg->origin_addr);

                ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] RECEIVED MESSAGE FROM: MAC = " + origin + " - ORIGIN COMPONENT ID = " + std::to_string(component_msg->origin_port));
            }

            delete msg;

            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive stopped.");
        }

        void send() {
            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send started.");
            
            while (_running) {
                Message* msg = new Message();
                ComponentMessage* data = msg->get_data<ComponentMessage>();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                if (!_running) {
                    break;
                }

                memcpy(data->origin_addr, _vehicle->nic()->address(), sizeof(Ethernet::Address));
                data->origin_port = _id;

                EthernetProtocol::Address from(_vehicle->nic()->address(), _id);
                EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

                msg->size(sizeof(ComponentMessage));

                _communicator->send(msg, from, to);
            }
            

            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send stopped.");
        }
        
        Ethernet::Address& get_address() {
            return _vehicle->nic()->address();
        }

        const unsigned short& id() const { return _id; }

        // virtual void process_data();
        // virtual void create_message();
        
    private:
        std::string mac_to_string(Ethernet::Address& addr) {
            std::stringstream ss;
            ss << std::hex << std::setfill('0');
            
            for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
                if (i > 0) ss << ":";
                ss << std::setw(2) << static_cast<int>(addr[i]);
            }
            
            return ss.str();
        }
    
    protected:    
        unsigned short _id;
        std::atomic<bool> _running;
        std::thread _send_thread;
        std::thread _receive_thread;
        Semaphore _semaphore;
        Queue<Message, 16> _receive_queue;
        std::atomic<int> _count;
        EthernetCommunicator* _communicator;

        Vehicle* _vehicle;
};

#endif // VEHICLE_H