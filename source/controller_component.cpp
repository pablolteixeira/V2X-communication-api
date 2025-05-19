#include "../header/component/controller_component.h"
#include <algorithm>

void ControllerComponent::run() 
{
    ConsoleLogger::log("Controller processing and generating commands");
    generate_data();
    ConsoleLogger::log("Command generation completed");
}

ControllerComponent::ControllerComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id), _lidar_value(0), _gps_value(0), _external_gps_value(0) {
    // Command data type (not an SI unit)
    _data_type = 0b0 << 31 | 1 << 24;  // Digital command type
}

void ControllerComponent::generate_data() {
    // Generate command based on received sensor data
    int steering_angle = 0;
    
    if (_lidar_value < 20) {
        // Object detected close by, steer away
        steering_angle = 30; // Turn 30 degrees
    }
    
    _value = steering_angle;
    ConsoleLogger::log("Controller generated command: " + std::to_string(_value));
}

void ControllerComponent::set_interests() {
    ConsoleLogger::log("Calling from inside set interests");
    // Interested in Lidar data (from own vehicle)
    ComponentDataType lidar_data_type = 0b1 << 31 | 9 << 18;
    ComponentInterest comp_lidar = {
        lidar_data_type,
        std::chrono::microseconds(100 * 1000),
        InterestBroadcastType::INTERNAL
    };
    _interests.push_back(comp_lidar);
    
    // Interested in GPS data (from own vehicle)
    ComponentDataType gps_data_type = 0b1 << 31 | 10 << 18;
    ComponentInterest comp_gps_int = {
        gps_data_type, 
        std::chrono::microseconds(200 * 1000), 
        InterestBroadcastType::INTERNAL
    };
    _interests.push_back(comp_gps_int);
    
    // Interested in external GPS data
    ComponentDataType ext_gps_data_type = 0b1 << 31 | 10 << 18;

    //std::chrono::microseconds period = getpid() % 2 == 0 ? std::chrono::microseconds(300 * 1000) : std::chrono::microseconds(200 * 1000);  

    ComponentInterest comp_gps_ext = {
        ext_gps_data_type, 
        std::chrono::microseconds(200 * 1000), 
        InterestBroadcastType::EXTERNAL
    };
    _interests.push_back(comp_gps_ext);
}

void ControllerComponent::process_data(Message::ResponseMessage* data) {
    ComponentDataType data_type = data->type;
    
    bool is_internal = memcmp(data->origin.mac, get_address(), 6) == 0;
    
    if (data_type == (0b1 << 31 | 9 << 18)) {
        _lidar_value = data->value;
        ConsoleLogger::log("Received Lidar data: " + std::to_string(_lidar_value));
    } else if (data_type == (0b1 << 31 | 10 << 18)) {
        // GPS data
        if (is_internal) {
            // Own GPS data
            _gps_value = data->value;
            ConsoleLogger::log("Received GPS data: " + std::to_string(_gps_value));
        } else {
            // External GPS data
            _external_gps_value = data->value;
            ConsoleLogger::log("Received external GPS data: " + std::to_string(_external_gps_value));
        }
    }
}
