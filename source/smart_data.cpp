#include "../header/smart_data.h"
#include "../header/console_logger.h"
#include "../header/message.h"
#include "../header/period_thread.h"

SmartData::SmartData(Ethernet::Address& nic_address, const unsigned short id)
    : _running(false), _id(id), _semaphore(0), _period_time_internal_response_thread(0), _period_time_external_response_thread(0), _internal_response_thread(nullptr), _external_response_thread(nullptr), _interest_thread(nullptr) 
{
    _component_addr = EthernetProtocol::Address(nic_address, id);
    
    _communicator = new EthernetCommunicator(EthernetProtocol::get_instance(), _component_addr);
}

SmartData::~SmartData() {
    stop();

    delete _communicator;

    for (auto& pair : _internal_interest_messages) {
        delete pair.first;
        delete pair.second;
    }
    for (auto& pair : _external_interest_messages) {
        delete pair.first;
        delete pair.second;
    }
}

void SmartData::start() {
    ConsoleLogger::log("SmartData: Starting threads");
    if (_running) return;
    _running = true;

    send_internal_interests();

    if (_external_interest_messages.size() > 0) {
        _interest_thread = new PeriodicThread(
            std::bind(&SmartData::send_external_interests, this),
            static_cast<__u64>(std::chrono::microseconds(500 * 1000).count()),
            static_cast<__u64>(std::chrono::microseconds(_get_interests().size() * 400).count())
        );
        _interest_thread->start();
    }

    _receive_thread = std::thread(&SmartData::receive, this);
}

void SmartData::stop() {
    ConsoleLogger::log("SmartData: Stopping threads 0");
    _running = false;

    if (_interest_thread != nullptr) {
        delete _interest_thread;
    }
    ConsoleLogger::log("SmartData: Stopping threads 1");

    if (_internal_response_thread != nullptr) {
        delete _internal_response_thread;
    }
    ConsoleLogger::log("SmartData: Stopping threads 2");

    if (_external_response_thread != nullptr) {
        delete _external_response_thread;
    }
    ConsoleLogger::log("SmartData: Stopping threads 3");

    if (_receive_thread.joinable()) {
        ConsoleLogger::log("RECEIVE THREAD JOINABLE");
        _communicator->stop();
        _receive_thread.join();
    }

    _communicator->stop();
}

void SmartData::register_component(GetInterestsCallback get_cb, GetDataCallback get_data, GetAddressCallback get_add, ProcessDataCallback p_data, GetMessageInfoCallback get_message_info, ComponentDataType data_type) {
    _get_interests = get_cb;
    _get_data = get_data;
    _get_address = get_add;
    _process_data = p_data;
    _get_message_info = get_message_info;
    _data_type = data_type;

    Ethernet::Address address;
    memcpy(address, _get_address(), 6);

    EthernetProtocol::Address from(address, _id);

    for (auto interest : _get_interests()) {
        EthernetProtocol::Address* to;
        if (interest.interest_broadcast_type == InterestBroadcastType::EXTERNAL) {
            to = new EthernetProtocol::Address(EthernetProtocol::Address::BROADCAST_MAC, 0);
        } else {
            to = new EthernetProtocol::Address(address, 0);
        }
        
        Message::InterestMessage interest_payload;
        
        interest_payload.type = interest.data_type;
        interest_payload.period = interest.period;
        
        Message* msg = new Message();
        msg->set_type(Message::Type::INTEREST);
        msg->set_payload(interest_payload);
        
        MessageAddressPair interest_message = std::make_pair(msg, to);

        if (interest.interest_broadcast_type == InterestBroadcastType::EXTERNAL) {
            _external_interest_messages.push_back(interest_message);
        } else {
            _internal_interest_messages.push_back(interest_message);
        }
    }
}

void SmartData::send_external_interests() {
    if (!_running) return;
    
    for (auto interest : _external_interest_messages) {
        send_interest(interest);
    }
}

void SmartData::send_internal_interests() {
    if (!_running) return;
    
    for (auto interest : _internal_interest_messages) {
        send_interest(interest);
    }
}

// Modified receive function to register interests
void SmartData::receive() {
    std::string component_address = Ethernet::address_to_string(_get_address());
    Message* msg = new Message();
    unsigned int id;

    ConsoleLogger::log("Smart data: Starting receive thread");
    while (_running) {
        if (_communicator->receive(msg, id)) {
            if (!_running) {
                break;
            }

            Message::MessageHeader* header = msg->get_header();

            switch(header->type) {
                case Message::Type::INTEREST: {
                    auto* interest_payload = msg->get_payload<Message::InterestMessage>();
                    if (interest_payload->type == _data_type) {
                        Ethernet::MessageInfo message_info = _get_message_info(id);

                        bool is_internal = memcmp(message_info.origin_mac, _get_address(), ETH_ALEN) == 0;

                        ConsoleLogger::log("Interest arrived: From -> " + Ethernet::address_to_string(message_info.origin_mac));
                        Origin origin;
                        memcpy(&origin.mac, &message_info.origin_mac, 6);
                        origin.port = 0;

                        // Register the interest
                        _interest_table.register_interest(
                            origin,
                            interest_payload->type,
                            interest_payload->period,
                            is_internal
                        );

                        // Calculate new GCD period using the registry
                        auto new_period = _interest_table.calculate_gcd_period(is_internal);
                        
                        if(is_internal){
                            if (_internal_response_thread == nullptr) {
                                _period_time_internal_response_thread = new_period;
                                ConsoleLogger::log("SmartData: Internal Interest arrived and response thread not initialized");
                                ConsoleLogger::log("SmartData: Initial Internal response period -> " + std::to_string(_period_time_internal_response_thread.count()) + " microseconds.");
                                
                                _internal_response_thread = new PeriodicThread(
                                    std::bind(&SmartData::send_response_internal, this), 
                                    static_cast<__u64>(_period_time_internal_response_thread.count())
                                );
                                _internal_response_thread->start();
                            } else {
                                ConsoleLogger::log("SmartData: Internal Interest arrived and response thread initialized");
                                
                                if (_period_time_internal_response_thread != new_period) {
                                    _period_time_internal_response_thread = new_period;
                                    ConsoleLogger::log("SmartData: Updating Internal response period -> " + std::to_string(_period_time_internal_response_thread.count()) + " microseconds.");
                                    _internal_response_thread->update(_period_time_internal_response_thread.count());
                                }
                            }
                        } else {
                            if (_external_response_thread == nullptr) {
                                _period_time_external_response_thread = new_period;
                                ConsoleLogger::log("SmartData: External interest arrived and response thread not initialized");

                                _external_response_thread = new PeriodicThread(
                                    std::bind(&SmartData::send_response_external, this), 
                                    static_cast<__u64>(_period_time_external_response_thread.count())
                                );
                                _external_response_thread->start();
                            } else {
                                ConsoleLogger::log("SmartData: External Interest arrived and response thread initialized");
                                
                                if (_period_time_external_response_thread != new_period) {
                                    _period_time_external_response_thread = new_period;
                                    ConsoleLogger::log("SmartData: Updating External response period -> " + std::to_string(_period_time_external_response_thread.count()) + " microseconds.");
                                    _external_response_thread->update(_period_time_external_response_thread.count());
                                }
                            }
                        }
                    }
                    break;
                }
                case Message::Type::RESPONSE: {
                    auto*  response_payload = msg->get_payload<Message::ResponseMessage>();
                    for (InterestData data : _get_interests()) {
                        if (response_payload->type == data.data_type) {
                            Ethernet::MessageInfo message_info = _get_message_info(id);
                            std::string type_string = Ethernet::address_to_string(message_info.origin_mac) == component_address ? "Internal" : "External";

                            auto now = std::chrono::system_clock::now();
                            auto now_micro = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());

                            if (now_micro >= data.next_receive * 1.2) {
                                ConsoleLogger::log("SmartData [" + std::to_string(_id) + "]: received " + type_string + " response message - value = " + std::to_string(response_payload->value) + " and using it.");
                            } else {
                                data.next_receive += data.period;
                                ConsoleLogger::log("SmartData [" + std::to_string(_id) + "]: received " + type_string + " response message - value = " + std::to_string(response_payload->value) +  " but descarting it.");
                            }

                            _process_data(response_payload, id);

                            break;
                        }
                    }

                    break;
                }
                default:
                    ConsoleLogger::log("SmartData: Message with unknown type");
                    break;
            }
        }
    }

    delete msg;
}


void SmartData::send_response_external() {
    if (!_running) return;

    ConsoleLogger::log("SmartData [" + std::to_string(_id) + "]: Sending Response External, Value: " + std::to_string(_get_data()));
    Ethernet::Address address;
    memcpy(address, _get_address(), 6);
    EthernetProtocol::Address from(address, _id);
    EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

    int value = _get_data();

    Message* msg = new Message();

    Message::ResponseMessage response_payload;

    response_payload.type = _data_type;
    response_payload.value = value;

    msg->set_type(Message::Type::RESPONSE);
    msg->set_payload(response_payload);

    _communicator->send(msg, from, to);

    delete msg;
}

void SmartData::send_response_internal() {
    if (!_running) return;

    ConsoleLogger::log("SmartData [" + std::to_string(_id) + "]: Sending Response Internal, Value: " + std::to_string(_get_data()));
    Ethernet::Address address;
    memcpy(address, _get_address(), 6);
    EthernetProtocol::Address from(address, _id);
    EthernetProtocol::Address to(address, 0);

    int value = _get_data();

    Message* msg = new Message();

    Message::ResponseMessage response_payload;

    response_payload.type = _data_type;
    response_payload.value = value;

    msg->set_type(Message::Type::RESPONSE);
    msg->set_payload(response_payload);

    _communicator->send(msg, from, to);

    delete msg;
}

void SmartData::send_interest(MessageAddressPair interest) {
    EthernetProtocol::Address* to = interest.second;
    _communicator->send(interest.first, _component_addr, *to);
}
