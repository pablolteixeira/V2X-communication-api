#include "../header/component.h"
#include "../header/agent/vehicle.h"

Component::Component(AutonomousAgent* autonomous_agent, const unsigned short& id)
    : _id(id), _running(false), _semaphore(0), _autonomous_agent(autonomous_agent) {
        _smart_data = new SmartData(_autonomous_agent->nic()->address(), id);
    }

Component::~Component() {
    
}

void Component::start() {
    if (_running) return;
    set_interests();

    _smart_data->register_component(
        [&]() { return get_interests(); },
        [&]() { return get_value(); },
        [&]() -> Ethernet::Address& { return get_address(); },
        [&](Message::ResponseMessage* msg) { process_data(msg); },
        _data_type
    );
    
    _running = true;
    _running_thread = new PeriodicThread(
            std::bind(&Component::run, this), 
            static_cast<__u64>(std::chrono::microseconds(100 * 1000).count()),
            static_cast<__u64>(std::chrono::microseconds(400).count())
        );
    _running_thread->start();

    _smart_data->start();
}

void Component::stop() {
    _running = false;

    ConsoleLogger::log("COMPONENT: STOPPING RUNNING THREAD");
    if (_running_thread != nullptr) {
        delete _running_thread;
    }
    ConsoleLogger::log("COMPONENT: STOPPING SMART DATA");
    delete _smart_data;
}

Ethernet::Address& Component::get_address() {
    return _autonomous_agent->nic()->address();
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
    // ConsoleLogger::log("Get Interests component size: " + std::to_string(_interests.size()));
    return _interests;
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
