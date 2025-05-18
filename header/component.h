#ifndef COMPONENT_H
#define COMPONENT_H

#include <thread>
#include <atomic>
#include <string>
#include <sstream>
#include <iomanip>
#include "console_logger.h"
#include "queue.h"
#include "semaphore.h"
#include "type_definitions.h"
#include "period_thread.h"
#include <chrono>

struct ComponentMessage {
    Ethernet::Address origin_addr;
    unsigned short origin_port;
    int id;
};

// Forward declarations
class Vehicle;

class Component 
{
public:
    typedef std::pair<ComponentDataType, std::chrono::microseconds> ComponentInterest;
    
    Component(Vehicle* vehicle, const unsigned short& id);
    virtual ~Component();

    void start();

    void stop();
    virtual void run() = 0;
    virtual void set_interests() = 0;
    virtual void process_data(Message::ResponseMessage* data) = 0;
    virtual void generate_data() = 0;
    Ethernet::Address& get_address();
    const unsigned short& id() const;

    ComponentDataType get_data_type();
    int get_value();

    std::vector<InterestData> get_interests();

protected:
    unsigned short _id;
    std::atomic<bool> _running;
    //std::thread _running_thread;
    PeriodicThread* _running_thread;
    Semaphore _semaphore;

    Queue<Message, 16> _receive_queue;
    ComponentDataType _data_type;
    std::vector<ComponentInterest> _interests;

    std::atomic<int> _value;

    Vehicle* _vehicle;

private:
    std::string mac_to_string(Ethernet::Address& addr);
};

#endif // COMPONENT_H
