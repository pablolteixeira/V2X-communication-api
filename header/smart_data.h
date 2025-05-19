#ifndef SMART_DATA_H
#define SMART_DATA_H

#include "type_definitions.h"
#include "vehicle.h"
#include "component.h"
#include "observer.h"
#include <numeric>
#include <pthread.h>
#include <thread>
#include "period_thread.h"
#include "observer.h"
#include <functional>

class SmartData {
public:
    SmartData(Component* component, EthernetCommunicator* communicator);
    ~SmartData();

    void start();
    void stop();

    void send_interests();

private:
    void receive();
    void send_response_external();
    void send_response_internal();
    void send_interest(InterestData& interest);

    bool _running;
    EthernetCommunicator* _communicator;
    Component* _component;
    std::thread _receive_thread;
    std::thread _send_thread;
    PeriodicThread* _external_response_thread;
    PeriodicThread* _internal_response_thread;
    PeriodicThread* _interest_thread;

    std::chrono::microseconds _period_time_external_response_thread;
    std::chrono::microseconds _period_time_internal_response_thread;
    std::vector<InterestData> _interests;

    Semaphore _semaphore;
    Queue<int, 32> _queue;
};

#endif // SMART_DATA_H
