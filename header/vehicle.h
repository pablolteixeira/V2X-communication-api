#ifndef VEHICLE_H
#define VEHICLE_H

#include "ethernet.h"
#include "traits.h"
#include "nic.h"
#include "raw_socket_engine.h"
#include "protocol.h"
#include "communicator.h"
#include "message.h"
#include "reference_buffer.h"
#include "queue.h"

#include <thread>
#include <atomic>
#include <random>

typedef NIC<RawSocketEngine> EthernetNIC;
typedef Protocol<EthernetNIC> EthernetProtocol;
typedef Communicator<EthernetProtocol> EthernetCommunicator;

class Component;

class Vehicle
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Vehicle();

    void start();
    void stop();
    void free(Message* msg);
    void notify(Message* msg);

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
    Ethernet::Address from_addr;
    unsigned short from_port;
    Ethernet::Address to_addr;
    unsigned short to_port;
    int id;
};

// Component base class
class Component {
    public:
        Component(Vehicle* vehicle, const unsigned short& id) : _id(id), _running(false), _semaphore(0), _vehicle(vehicle) {}
        ~Component() {
            stop();
        }
    
        void start() {
            if (_running) return;
            _running = true;
            _receive_thread = std::thread(&Component::receive, this);
            _send_thread = std::thread(&Component::send, this);
        }
    
        void stop() {
            if (!_running) return;
            _running = false;
            
            if (_send_thread.joinable()) {
                _send_thread.join();
            }

            _semaphore.v();
            if (_receive_thread.joinable()) {
                _receive_thread.join();
            }
        }

        void notify(Message* data) {
            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Received notify.");
            // _buffer add data
            if (!_running) {
                return;
            }
            Message* new_data = data;
            _receive_queue.add(new_data);
            _semaphore.v();
        };

        void receive() {
            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive started.");
            
            while (_running) {
                _semaphore.p();
                ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component P.");
                
                if (!_running) {
                    break;
                }
                // Process message based on type
                Message* msg = _receive_queue.remove();
                if (msg) {
                    ComponentMessage* component_msg = msg->get_data<ComponentMessage>();

                    std::string origin = mac_to_string(component_msg->from_addr);

                    ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] RECEIVED MESSAGE FROM: MAC = " + origin + " - ORIGIN COMPONENT ID = " + std::to_string(component_msg->from_port));
                    
                    //delete msg;
                }
            }

            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive stopped.");
        }

        void send() {
            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send started.");
            
            while (_running) {
                Message* msg = new Message();
                ComponentMessage* data = msg->get_data<ComponentMessage>();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                /*ComponentMessage* component_msg = msg->get_data<ComponentMessage>();

                std::string origin = mac_to_string(component_msg->from.paddr());

                ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] RECEIVED MESSAGE FROM: MAC = " + origin + " - COMPONENT ID = " + std::to_string(component_msg->from.port()));*/
                
                memcpy(data->from_addr, _vehicle->nic()->address(), sizeof(Ethernet::Address));
                data->from_port = _id;
                memcpy(data->to_addr, _vehicle->nic()->address(), sizeof(Ethernet::Address));
                data->to_port = 0;
                
                
                msg->size(sizeof(ComponentMessage));

                ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Notify vehicle.");
                _vehicle->notify(msg);

                //delete msg;
            }
            

            ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send stopped.");
        }
    
        const unsigned short& id() const { return _id; }
        
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
        Semaphore _semaphore;        Queue<Message, 16> _receive_queue;
        std::atomic<int> _count;

        Vehicle* _vehicle;
};

#endif // VEHICLE_H