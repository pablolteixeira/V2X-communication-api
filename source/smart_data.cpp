#include "smart_data.h"
#include "console_logger.h"
#include "message"
#include "period_thread.h"

SmartData::SmartData(Component* component, EthernetCommunicator* communicator) 
    : _running(false), _component(component), _communicator(communicator), _semaphore(0), _period_time(0) {
    send_interests();
}

SmartData::~SmartData() {}

void SmartData::start() {
    // Implementation for start
}

void SmartData::stop() {
    // Implementation for stop
}

void SmartData::send_interests() {
    std::vector<InterestData> component_interests = _component->get_interests();
    for (InterestData interest : component_interests) {
        send_interest(interest);
    }
}

void SmartData::receive() {
    Message* msg = new Message();

    while (_running) {
        _communicator->receive(msg);
        if (!_running) {
            break;
        }

        switch(msg->get_type()) {
            case Message::Type::INTEREST: {
                auto* interest_message = msg->get_data<Message::InterestMessage>();
                if (interest_message->type == _component->get_data_type()) {
                    if (!_response_thread.joinable()) {
                        _period_time = interest_message->period;

                        _response_thread = new PeriodicThread(send_response, _period_time * 1000);
                        _response_thread->start();
                    } else {
                        if (_period_time != interest_message->period) {
                            _period_time = std::chrono::microseconds(std::__gcd(_period_time.count(), interest_message->period.count()));                            
                            _response_thread->update(_period_time);
                        }
                    }
                }interest_message->type == _component->get_data_type()
                break;
            }
            case Message::Type::RESPONSE: {
                Message::ResponseMessage* response_message = msg->get_data<Message::ResponseMessage>();
                InterestData interest;

                for (InterestData data : _interests) {
                    if (response_message->type == data.data_type) {
                        interest = data;
                        
                        auto now = std::chrono::system_clock::now();
                        auto now_micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

                        if (now_micro >= interest.next_receive) {
                            interest.next_receive += interest.period;
                            ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: received interest message - value = " + std::to_string(response_message->value));
                        }

                        _component->process_data(response_message);

                        break;
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    delete msg;
}

void SmartData::send_response() {
    Message* msg = new Message();

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

    delete msg;
}

void SmartData::send_interest(InterestData& interest) {
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
