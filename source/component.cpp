#include "../header/component.h"
#include "../header/vehicle.h"

Component::Component(Vehicle* vehicle, const unsigned short& id)
    : _id(id), _running(false), _semaphore(0), _vehicle(vehicle) {
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
