#include "../header/component.h"
#include "../header/vehicle.h"

Component::Component(Vehicle* vehicle, const unsigned short& id, EthernetCommunicator* communicator)
    : _id(id), _running(false), _semaphore(0), _vehicle(vehicle), _communicator(communicator) {
    
}

Component::~Component() {}

void Component::start() {
    if (_running) return;
    _running = true;
    _running_thread = std::thread(&Component::run, this);
}

void Component::stop() {
    _running = false;
    if (_running_thread.joinable()) {
        _running_thread.join();
    }
}

void Component::run() {
    while(_running) {
        generate_data();
    }
}

// void Component::stop_send() {
//     if (!_running) return;
//     _running = false;
//     if (_send_thread.joinable()) {
//         _send_thread.join();
//     }
// }

// void Component::stop_receive() {
//     if (_running) return;
//     _communicator->stop();
//     if (_receive_thread.joinable()) {
//         _receive_thread.join();
//     }
// }

// void Component::receive() {
//     ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive started.");

//     Message* msg = new Message();

//     while (_running) {
//         _communicator->receive(msg);
//         if (!_running) break;

//         ComponentMessage* component_msg = msg->get_data<ComponentMessage>();
//         std::string origin = mac_to_string(component_msg->origin_addr);

//         ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] RECEIVED MESSAGE FROM: MAC = " + origin + " - ORIGIN COMPONENT ID = " + std::to_string(component_msg->origin_port));
//     }

//     delete msg;

//     ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component receive stopped.");
// }

// void Component::send() {
//     ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send started.");

//     while (_running) {
//         Message* msg = new Message();
//         ComponentMessage* data = msg->get_data<ComponentMessage>();
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));

//         if (!_running) break;

//         memcpy(data->origin_addr, _vehicle->nic()->address(), sizeof(Ethernet::Address));
//         data->origin_port = _id;

//         EthernetProtocol::Address from(_vehicle->nic()->address(), _id);
//         EthernetProtocol::Address to(EthernetProtocol::Address::BROADCAST_MAC, 0);

//         msg->size(sizeof(ComponentMessage));
//         _communicator->send(msg, from, to);
//     }

//     ConsoleLogger::log("[COMPONENT ID: [" + std::to_string(_id) + "] Component send stopped.");
// }

Ethernet::Address& Component::get_address() {
    return _vehicle->nic()->address();
}

const unsigned short& Component::id() const {
    return _id;
}

ComponentDataType Component::get_data_type() {
    return _data_type;
}

int Component::get_value() {
    return _value;
}

std::vector<InterestData> Component::get_interests() {
    std::vector<InterestData> interests;

    for (ComponentInterest component_interest : _interests) {
        InterestData data;
        data.data_type = component_interest.first;
        data.period = component_interest.second;
        data.next_receive = component_interest.second;
        
        interests.push_back(data);
    }

    return interests;
} 

std::string Component::mac_to_string(Ethernet::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');

    for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
        if (i > 0) ss << ":";
        ss << std::setw(2) << static_cast<int>(addr[i]);
    }

    return ss.str();
}
