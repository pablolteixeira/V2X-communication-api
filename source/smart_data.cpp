#include "../header/smart_data.h"
#include "../header/console_logger.h"
#include "../header/message.h"
#include "../header/period_thread.h"

SmartData::SmartData(Component* component, EthernetCommunicator* communicator) 
    : _running(false), _component(component), _communicator(communicator), _semaphore(0), _period_time(0), _response_thread(nullptr) {
    _interest_thread = new PeriodicThread(
        std::bind(&SmartData::send_interests, this),
        static_cast<__u64>(std::chrono::milliseconds(500).count() * 1000),
        static_cast<__u64>(std::chrono::milliseconds(100).count() * 1000)
    );
}

SmartData::~SmartData() {}

void SmartData::start() {
    if (_running) return;
    _running = true;

    _receive_thread = std::thread(&SmartData::receive, this);
    _interest_thread->start();
}

void SmartData::stop() {
    _interest_thread->stop();
}

void SmartData::send_interests() {
    ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: Sending Interests");
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

        Message::MessageHeader* header = msg->get_header();

        switch(header->type) {
            case Message::Type::INTEREST: {
                auto* interest_payload = msg->get_payload<Message::InterestMessage>();
                ConsoleLogger::log("SmartData: Interest arrived");

                if (interest_payload->type == _component->get_data_type()) {
                    if (_response_thread == nullptr) {
                        _period_time = interest_payload->period;
                        ConsoleLogger::log("SmartData: Interest arrived and response thread not initialized");

                        _response_thread = new PeriodicThread(
                            std::bind(&SmartData::send_response, this), 
                            static_cast<__u64>(_period_time.count()),
                            static_cast<__u64>((unsigned long long)(_period_time.count() * 0.7))
                        );
                        
                        _response_thread->start();
                    } else {
                        ConsoleLogger::log("SmartData: Interest arrived and response thread initialized");
                        ConsoleLogger::log(std::to_string(_period_time.count()) + " - " + std::to_string(interest_payload->period.count()));
                        if (_period_time != interest_payload->period) {
                            _period_time = std::chrono::microseconds(std::__gcd(_period_time.count(), interest_payload->period.count()));
                            ConsoleLogger::log("SmartData: Updating response period -> " + std::to_string(_period_time.count()) + " microseconds.");
                            _response_thread->update(_period_time.count());
                        }
                    }
                }
                break;
            }
            case Message::Type::RESPONSE: {
                Message::ResponseMessage* response_payload = msg->get_payload<Message::ResponseMessage>();
                ConsoleLogger::log("SmartData: Response arrived");

                for (InterestData data : _component->get_interests()) {
                    if (response_payload->type == data.data_type) {
                        
                        auto now = std::chrono::system_clock::now();
                        auto now_micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

                        if (now_micro >= data.next_receive) {
                            data.next_receive += data.period;
                            ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: received response message - value = " + std::to_string(response_payload->value) + " and using it.");
                        } else {
                            ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: received response message - value = " + std::to_string(response_payload->value) +  " but descarting it.");
                        }

                        //_component->process_data(response_message);

                        break;
                    }
                }

                ConsoleLogger::log("SmartData: Not interest found in this type");
                break;
            }
            default:
                ConsoleLogger::log("SmartData: Message with unknown type");

                break;
        }
    }

    delete msg;
}

void SmartData::send_response() {
    ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: Sending Response");
    EthernetProtocol::Address from(_component->get_address(), _component->id());
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

    int value = _component->get_value();

    Message* msg = new Message();

    Message::ResponseMessage response_payload;

    Origin origin = Origin {
        *_component->get_address(),
        _component->id()
    };

    response_payload.origin = origin;
    response_payload.type = _component->get_data_type();
    response_payload.value = value;

    msg->set_type(Message::Type::RESPONSE);
    msg->set_payload(response_payload);

    _communicator->send(msg, from, to);

    delete msg;
}

void SmartData::send_interest(InterestData& interest) {
    ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: Sending Interest");
    EthernetProtocol::Address from(_component->get_address(), _component->id());
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

    Message* msg = new Message();

    Message::InterestMessage interest_payload;

    Origin origin = Origin {
        *_component->get_address(),
        _component->id()
    };

    interest_payload.origin = origin;
    interest_payload.type = interest.data_type;
    interest_payload.period = interest.period;

    msg->set_type(Message::Type::INTEREST);
    msg->set_payload(interest_payload);

    _communicator->send(msg, from, to);

    delete msg;
}
