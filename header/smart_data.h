#ifndef SMART_DATA_H
#define SMART_DATA_H

#include <numeric>
#include <pthread.h>
#include <thread>
#include <functional>

#include "types.h"
#include "type_definitions.h"
#include "observer.h"
#include "period_thread.h"
#include "observer.h"
#include "interest_table.h"
#include "queue.h"
#include "component_types.h"


class SmartData {
public:
    SmartData(Ethernet::Address& address, const unsigned short id);
    ~SmartData();

    typedef std::function<std::vector<InterestData>()>  GetInterestsCallback;
    typedef std::function<int()>                        GetDataCallback;
    typedef std::function<Ethernet::Address&()>         GetAddressCallback;
    typedef std::function<void(Message::ResponseMessage *)> ProcessDataCallback;
    typedef std::pair<Message*, EthernetProtocol::Address*> MessageAddressPair;

    void start();
    void stop();

    void register_component(GetInterestsCallback get_cb, GetDataCallback get_data, GetAddressCallback get_add, ProcessDataCallback p_data, ComponentDataType data_type);
    void send_external_interests();
    void send_internal_interests();

private:
    void receive();
    void send_response_external();
    void send_response_internal();
    void send_interest(MessageAddressPair);

    bool _running;
    const unsigned short _id;
    ComponentDataType _data_type;
    EthernetCommunicator* _communicator;
    Semaphore _semaphore;
    std::chrono::microseconds _period_time_internal_response_thread;
    std::chrono::microseconds _period_time_external_response_thread;

    PeriodicThread* _internal_response_thread;
    PeriodicThread* _external_response_thread;
    PeriodicThread* _interest_thread;
    
    std::thread _receive_thread;
    std::thread _send_thread;

    InterestTable _interest_table;
    std::vector<MessageAddressPair> _external_interest_messages;
    std::vector<MessageAddressPair> _internal_interest_messages;

    Queue<int, 32> _queue;

    GetInterestsCallback _get_interests;
    GetDataCallback _get_data;
    GetAddressCallback _get_address;
    ProcessDataCallback _process_data;
};

#endif // SMART_DATA_H
