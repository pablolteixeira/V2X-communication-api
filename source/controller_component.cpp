#include "../header/component/controller_component.h"

void ControllerComponent::run() 
{
    ConsoleLogger::log("Controller processing and generating commands");
    generate_data();
    ConsoleLogger::log("Command generation completed");
}

ControllerComponent::ControllerComponent(AutonomousAgent* autonomous_agent, const unsigned short& id)
    : Component(autonomous_agent, id), _lidar_value(0), _gps_value(0), _external_gps_value(0) {
    // Command data type (not an SI unit)
    _data_type = ComponentDataTypes::DIGITAL_COMAND_TYPE;  // Digital command type
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
    std::chrono::microseconds period = std::chrono::microseconds(100 * 1000);

    // Interested in Lidar data (from own vehicle)
    ComponentDataType lidar_data_type = ComponentDataTypes::METER_DATATYPE;
    InterestData comp_lidar = {
        lidar_data_type,
        InterestBroadcastType::INTERNAL,
        period,
        period
    };
    _interests.push_back(comp_lidar);
    
    period = std::chrono::microseconds(200 * 1000);
    // Interested in GPS data (from own vehicle)
    ComponentDataType gps_data_type = ComponentDataTypes::POSITION_DATA_TYPE;
    InterestData comp_gps_int = {
        gps_data_type, 
        InterestBroadcastType::INTERNAL,
        period,
        period
    };
    _interests.push_back(comp_gps_int);
    
    // Interested in external GPS data
    ComponentDataType ext_gps_data_type = ComponentDataTypes::POSITION_DATA_TYPE;

    //std::chrono::microseconds period = getpid() % 2 == 0 ? std::chrono::microseconds(300 * 1000) : std::chrono::microseconds(200 * 1000);  

    InterestData comp_gps_ext = {
        ext_gps_data_type,  
        InterestBroadcastType::EXTERNAL,
        period,
        period
    };
    _interests.push_back(comp_gps_ext);
}

void ControllerComponent::process_data(Message::ResponseMessage* data, const unsigned int id) {
    ComponentDataType data_type = data->type;
    Ethernet::MessageInfo message_info = get_message_info(id);
    
    bool is_internal = memcmp(message_info.origin_mac, get_address(), ETH_ALEN) == 0;
    
    if (data_type == (ComponentDataTypes::METER_DATATYPE)) {
        _lidar_value = data->value;
        ConsoleLogger::log("Received Lidar data: " + std::to_string(_lidar_value));
    } else if (data_type == (ComponentDataTypes::POSITION_DATA_TYPE)) {
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

    ConsoleLogger::log("Controller Component: Message info received: Origin MAC address -> " + mac_to_string(message_info.origin_mac) +
        "; Origin ID -> " + std::to_string(message_info.origin_id) +
        "; Timestamp -> " + std::to_string(message_info.timestamp) + 
        "; Quadrant -> " + std::to_string(message_info.quadrant) +
        "; MAC -> " + std::to_string(message_info.mac));
}
