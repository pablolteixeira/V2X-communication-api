#include "../header/component.h"
#include "../header/vehicle.h"

Component::Component(Vehicle* vehicle, const unsigned short& id)
    : _id(id), _running(false), _semaphore(0), _vehicle(vehicle) {
    }

Component::~Component() {}

void Component::start() {
    if (_running) return;
    set_interests();
    format_interests();

    _running = true;
    _running_thread = new PeriodicThread(
            std::bind(&Component::run, this), 
            static_cast<__u64>(std::chrono::microseconds(100 * 1000).count()),
            static_cast<__u64>(std::chrono::microseconds(80 * 1000).count())
        );
    _running_thread->start();
}

void Component::stop() {
    _running = false;
    _running_thread->stop();}

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

void Component::format_interests() {
    for (ComponentInterest component_interest : _interests) {
        InterestData data;
        data.data_type = component_interest.data_type;
        data.interest_broadcast_type = component_interest.interest_broadcast_type;
        data.period = component_interest.period;
        data.next_receive = component_interest.period;
        
        _formatted_interests.push_back(data);
    }
}

std::vector<InterestData> Component::get_interests() {
    return _formatted_interests;
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
