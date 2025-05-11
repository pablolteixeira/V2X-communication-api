#ifndef SMART_DATA_H
#define SMART_DATA_H

#include "type_definitions.h"
#include "vehicle.h"
#include "component.h"
#include <numeric>
#include <thread>

struct InterestData {
    ComponentDataType data_type;
    std::chrono::microseconds period;
    std::chrono::microseconds next_receive;
};

class SmartData {
public:
    SmartData(Component* component, EthernetCommunicator* communicator) : 
        _running(false), _component(component), _communicator(communicator), _semaphore(0), _sleep_time(0) {
            
        send_interests();
    }
    ~SmartData() {}

    void start() {
        
    }

    void stop() {

    }

    void send_interests() {
        std::vector<InterestData> component_interests = _component->get_interests();

        for (InterestData interest : component_interests) {
            send_interest(interest);
        }
    }

private:
    void receive() {
        Message* msg = new Message();

        while (_running) {
            _communicator->receive(msg);
            if (!_running) {
                break;
            }

            switch(msg->get_type()) {
                case Message::Type::INTEREST:
                    Message::InterestMessage* interest_message = msg->get_data<Message::InterestMessage>();

                    if (interest_message->type == _component->get_data_type()) {
                        if (!_response_thread.joinable()) {
                            _sleep_time = interest_message->period;
                            _response_thread = std::thread(&SmartData::send_response, this);
                        } else {
                            if (_sleep_time != interest_message->period) 
                                _sleep_time = std::__gcd(_sleep_time, interest_message->period);
                        }
                    }

                    break;
                case Message::Type::RESPONSE:
                    Message::ResponseMessage* response_message = msg->get_data<Message::ResponseMessage>();

                    InterestData interest;

                    for (InterestData data : _interests) {
                        if (response_message->type == data.data_type) {
                            interest = data;
                        }
                    }

                    auto now = std::chrono::system_clock::now();
                    auto now_micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

                    if (now_micro >= interest.next_receive) {
                        interest.next_receive += interest.period;

                        ConsoleLogger::log("SmartData ["+std::to_string(_component->id())+"]: received interest message - value = " + std::to_string(response_message->value));
                    }

                    break;
                default:
                    break;
            }
        }

        delete msg;
    }
private:
    void send_response() {
        Message* msg = new Message();

        while (_running) {
            int value = _component->get_value();

            EthernetProtocol::Address from(_component->get_address(), _component->id());
            EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

            msg->set_type(Message::Type::RESPONSE);
            Message::ResponseMessage* response_message = msg->get_data<Message::ResponseMessage>();

            Origin origin = Origin {
                *_component->get_address(),
                _component->id()
            };

            response_message->origin = origin;
            response_message->type = _component->get_data_type();
            response_message->value = value;

            _communicator->send(msg, from, to);

            std::this_thread::sleep_for(_sleep_time);
        }
        
        delete msg;
    }

    void send_interest(InterestData& interest) {
        EthernetProtocol::Address from(_component->get_address(), _component->id());
        EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

        Message* msg = new Message();

        msg->set_type(Message::Type::INTEREST);
        Message::InterestMessage* interest_message = msg->get_data<Message::InterestMessage>();

        Origin origin = Origin {
            *_component->get_address(),
            _component->id()
        };

        interest_message->origin = origin;
        interest_message->type = interest.data_type;
        interest_message->period = interest.period;
        
        _communicator->send(msg, from, to);

        delete msg;
    }


private:
    bool _running;
    EthernetCommunicator* _communicator;
    Component* _component;
    std::thread _receive_thread;
    std::thread _send_thread;

    std::thread _response_thread;
    std::chrono::microseconds _sleep_time;

    std::vector<InterestData> _interests;

    Semaphore _semaphore;
    Queue<int, 32> _queue;
};

#endif // SMART_DATA_H