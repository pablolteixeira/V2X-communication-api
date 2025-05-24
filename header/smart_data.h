#ifndef SMART_DATA_H
#define SMART_DATA_H

#include "type_definitions.h"
#include "vehicle.h"
#include "observer.h"
#include <numeric>
#include <pthread.h>
#include <thread>
#include "period_thread.h"
#include "observer.h"
#include "interest_table.h"
#include <functional>

class SmartData {
public:
    SmartData(EthernetCommunicator* communicator);
    ~SmartData();

    typedef std::function<std::vector<InterestData>()>  GetInterestsCallback;
    typedef std::function<int()>                        GetDataCallback;
    typedef std::function<int()>                        GetIdCallback;
    typedef std::function<Ethernet::Address*()>         GetAddressCallback;

    void start();
    void stop();

    void register_component(GetInterestsCallback get_cb, GetDataCallback get_data, GetAddressCallback get_add, GetIdCallback get_id);
    void send_interests();

private:
    void receive();
    void send_response_external();
    void send_response_internal();
    void send_interest(Message* message);

    bool _running;
    EthernetCommunicator* _communicator;
    Semaphore _semaphore;
    std::chrono::microseconds _period_time_internal_response_thread;
    std::chrono::microseconds _period_time_external_response_thread;

    PeriodicThread* _internal_response_thread;
    PeriodicThread* _external_response_thread;
    PeriodicThread* _interest_thread;
    
    std::thread _receive_thread;
    std::thread _send_thread;

    InterestTable interest_table;
    std::vector<std::pair<Message*, Ethernet::Address>> _interest_messages;

    Queue<int, 32> _queue;

    GetInterestsCallback _get_interests;
    GetDataCallback _get_data;
    GetAddressCallback _get_address;
    GetIdCallback _get_id;
};

#endif // SMART_DATA_H
