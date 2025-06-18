#include "../header/component/gps_component.h"


void GPSComponent::run() 
{
    ConsoleLogger::log("About to generate GPS data");
    generate_data();
    ConsoleLogger::log("GPS data generation completed");
}

GPSComponent::GPSComponent(AutonomousAgent* autonamous_agent, const unsigned short& id)
    : Component(autonamous_agent, id) {
    // SI unit: position (lat/long) => ComponentDataTypes::POSITION_DATA_TYPE; // Representing position data
    _data_type = ComponentDataTypes::POSITION_DATA_TYPE;  // Position data type
}

void GPSComponent::generate_data() {    
    _value = static_cast<int>(getpid());
    
    ConsoleLogger::log("GPS data generated: Lat: " + std::to_string(_value) + 
                      ", Long: " + std::to_string(_value));
}

void GPSComponent::set_interests() {
}

void GPSComponent::process_data(Message::ResponseMessage* data, const unsigned int id) {
    Ethernet::MessageInfo message_info = get_message_info(id);
    ConsoleLogger::log("GPS Component: Message info received: Origin MAC address -> " + mac_to_string(message_info.origin_mac) +
        "; Origin ID -> " + std::to_string(message_info.origin_id) +
        "; Timestamp -> " + std::to_string(message_info.timestamp) + 
        "; Quadrant -> " + std::to_string(message_info.quadrant) +
        "; MAC -> " + std::to_string(message_info.mac));
}