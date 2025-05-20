#include "../header/smart_data.h"
#include "../header/console_logger.h"
#include "../header/message.h"
#include "../header/period_thread.h"

SmartData::SmartData(Component* component, EthernetCommunicator* communicator) 
    : _running(false), _component(component), _communicator(communicator), _semaphore(0), _period_time_internal_response_thread(0), _period_time_external_response_thread(0), _internal_response_thread(nullptr), _external_response_thread(nullptr) {
    _interest_thread = new PeriodicThread(
        std::bind(&SmartData::send_interests, this),
        static_cast<__u64>(std::chrono::milliseconds(500).count() * 1000),
        static_cast<__u64>(std::chrono::milliseconds(250).count() * 1000)
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
    ConsoleLogger::log("SmartData: Stopping threads");
    _running = false;
    _interest_thread->stop();

    if (_internal_response_thread != nullptr) {
        _internal_response_thread->stop();
    }
    
    if (_external_response_thread != nullptr) {
        _external_response_thread->stop();
    }

    if (_receive_thread.joinable()) {
        _communicator->stop();
        _receive_thread.join();
    }
}

void SmartData::send_interests() {
    if (!_running) return;

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

        ConsoleLogger::log("Smart data receiving -> " + std::to_string(header->type));
        switch(header->type) {
            case Message::Type::INTEREST: {
                auto* interest_payload = msg->get_payload<Message::InterestMessage>();

                bool is_internal = memcmp(interest_payload->origin.mac, _component->get_address(), 6) == 0;
                
                if(is_internal){
                    if (_internal_response_thread == nullptr) {
                        _period_time_internal_response_thread = interest_payload->period;
                        ConsoleLogger::log("SmartData: Internal Interest arrived and response thread not initialized");

                        ConsoleLogger::log("SmartData: Initial Internal response period -> " + std::to_string(_period_time_internal_response_thread.count()) + " microseconds.");
                        _internal_response_thread = new PeriodicThread(
                            std::bind(&SmartData::send_response_internal, this), 
                            static_cast<__u64>(_period_time_internal_response_thread.count()),
                            static_cast<__u64>((unsigned long long)(_period_time_internal_response_thread.count() * 0.5))
                        );
                        _internal_response_thread->start();
                    } else {
                        ConsoleLogger::log("SmartData: Internal Interest arrived and response thread initialized");
                        ConsoleLogger::log(std::to_string(_period_time_internal_response_thread.count()) + " - " + std::to_string(interest_payload->period.count()));
                        if (_period_time_internal_response_thread.count() != std::__gcd(_period_time_internal_response_thread.count(), interest_payload->period.count())) {
                            _period_time_internal_response_thread = std::chrono::microseconds(std::__gcd(_period_time_internal_response_thread.count(), interest_payload->period.count()));
                            ConsoleLogger::log("SmartData: Updating Internal response period -> " + std::to_string(_period_time_internal_response_thread.count()) + " microseconds.");
                            _internal_response_thread->update(_period_time_internal_response_thread.count());
                        }
                    }
                } else {
                    if (_external_response_thread == nullptr) {
                        _period_time_external_response_thread = interest_payload->period;
                        ConsoleLogger::log("SmartData: External interest arrived and response thread not initialized");

                        _external_response_thread = new PeriodicThread(
                            std::bind(&SmartData::send_response_external, this), 
                            static_cast<__u64>(_period_time_external_response_thread.count()),
                            static_cast<__u64>((unsigned long long)(_period_time_external_response_thread.count() * 0.5))
                        );
                        _external_response_thread->start();
                    } else {
                        ConsoleLogger::log("SmartData: External Interest arrived and response thread initialized");
                        ConsoleLogger::log(std::to_string(_period_time_external_response_thread.count()) + " - " + std::to_string(interest_payload->period.count()));
                        if (_period_time_external_response_thread.count() != std::__gcd(_period_time_external_response_thread.count(), interest_payload->period.count())) {
                            _period_time_external_response_thread = std::chrono::microseconds(std::__gcd(_period_time_external_response_thread.count(), interest_payload->period.count()));
                            ConsoleLogger::log("SmartData: Updating External response period -> " + std::to_string(_period_time_external_response_thread.count()) + " microseconds.");
                            _external_response_thread->update(_period_time_external_response_thread.count());
                        }
                    }
                }
                break;
            }
            case Message::Type::RESPONSE: {
                auto*  response_payload = msg->get_payload<Message::ResponseMessage>();
                ConsoleLogger::log("SmartData: Response arrived");
                ConsoleLogger::log(std::to_string(_component->get_interests().size()) + "Interests for this component");
                for (InterestData data : _component->get_interests()) {
                    if (response_payload->type == data.data_type) {
                        std::string type_string = (response_payload->origin.mac) == _component->get_address() ? "Internal" : "External";

                        auto now = std::chrono::system_clock::now();
                        auto now_micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

                        if (now_micro >= data.next_receive * 1.2) {
                            ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: received " + type_string + " response message - value = " + std::to_string(response_payload->value) + " and using it.");
                        } else {
                            data.next_receive += data.period;
                            ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: received " + type_string + " response message - value = " + std::to_string(response_payload->value) +  " but descarting it.");
                        }

                        _component->process_data(response_payload);

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

void SmartData::send_response_external() {
    if (!_running) return;

    ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: Sending Response External, Value: " + std::to_string(_component->get_value()));
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

void SmartData::send_response_internal() {
    if (!_running) return;

    ConsoleLogger::log("SmartData [" + std::to_string(_component->id()) + "]: Sending Response Internal, Value: " + std::to_string(_component->get_value()));
    EthernetProtocol::Address from(_component->get_address(), _component->id());
    EthernetProtocol::Address to(_component->get_address(), 0);

    int value = _component->get_value();

    Message* msg = new Message();

    Message::ResponseMessage response_payload;

    Origin origin;
    memcpy(origin.mac, _component->get_address(), 6);
    origin.port = _component->id();

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
    EthernetProtocol::Address* to;

    if (interest.interest_broadcast_type == InterestBroadcastType::EXTERNAL) {
        to = new EthernetProtocol::Address(EthernetProtocol::Address::BROADCAST_MAC, 0);
    } else {
        to = new EthernetProtocol::Address(_component->get_address(), 0);
    }

    Message* msg = new Message();

    Message::InterestMessage interest_payload;

    Origin origin;
    memcpy(origin.mac, _component->get_address(), 6);
    origin.port = _component->id();

    interest_payload.origin = origin;
    interest_payload.type = interest.data_type;
    interest_payload.period = interest.period;

    msg->set_type(Message::Type::INTEREST);
    msg->set_payload(interest_payload);

    _communicator->send(msg, from, *to);

    delete msg;
}
