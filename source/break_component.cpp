#include "../header/component/break_component.h"
#include <random>


void BreakComponent::run() 
{
    
}

BreakComponent::BreakComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id) {
    // SI unit: meter => m+1 => 0b101 (5) => m+4 = 9
    _data_type = 0b1 << 31 | 9 << 18;  // Only m+4 is set
}

void BreakComponent::generate_data() {
    
}

void BreakComponent::set_interests() {
    ComponentDataType _control_data_type = 0b0 << 31;
    //_interests.push_back({_control_data_type, std::chrono::milliseconds(10) });
    auto time = getpid() % 2 == 0 ? std::chrono::milliseconds(250) : std::chrono::milliseconds(100);

    _interests.push_back({_data_type, time});
}

void BreakComponent::process_data(Message::ResponseMessage* data) {
    std::string log_msg = "Data processed: ";
    log_msg += "Port: " + std::to_string(data->origin.port) + ", ";
    log_msg += "Type: " + std::to_string(data->type) + ", ";
    log_msg += "Value: " + std::to_string(data->value);

    ConsoleLogger::log(log_msg);
    
}

// struct ResponseMessage {
//     Origin origin;
//     unsigned int type;
//     int value;
// };

// struct Origin {
//     Ethernet::Address mac;
//     unsigned short port;
// };
