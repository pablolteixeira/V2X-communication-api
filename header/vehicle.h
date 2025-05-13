#ifndef VEHICLE_H
#define VEHICLE_H

#include "ethernet.h"
#include "traits.h"
#include "type_definitions.h"
#include "message.h"
#include "reference_buffer.h"
#include "queue.h"
#include "component.h"
#include "traits.h"
#include "smart_data.h"

#include <thread>
#include <atomic>
#include <random>

class Vehicle
{
public:
    Vehicle(EthernetNIC* nic, EthernetProtocol* protocol);
    ~Vehicle();

    void start();
    void stop();
    void free(Message* msg);

    EthernetNIC* nic() const;

private:
    int _id;
    EthernetNIC* _nic;
    EthernetProtocol* _protocol;
    
    std::atomic<bool> _running;
    
    std::thread _receive_thread;
    std::thread _send_thread;

    EthernetCommunicator* _communicator[Traits<Component>::NUM_COMPONENTS];
    Component* _components[Traits<Component>::NUM_COMPONENTS];
    SmartData* _smart_datas[Traits<Component>::NUM_COMPONENTS];
    
    Semaphore _semaphore;
    Queue<Message, 16> _queue;
};

#endif // VEHICLE_H