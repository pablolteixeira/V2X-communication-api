#include "../header/component/gps_component.h"


void GPSComponent::run() 
{
    ConsoleLogger::log("About to generate GPS data");
    generate_data();
    ConsoleLogger::log("GPS data generation completed");
}

GPSComponent::GPSComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id) {
    // SI unit: position (lat/long) => ComponentDataTypes::POSITION_DATA_TYPE; // Representing position data
    _data_type = ComponentDataTypes::POSITION_DATA_TYPE;  // Position data type
}

void GPSComponent::generate_data() {    
    _value = static_cast<int>(1000000);
    
    ConsoleLogger::log("GPS data generated: Lat: " + std::to_string(_value) + 
                      ", Long: " + std::to_string(_value));
}

void GPSComponent::set_interests() {
}

void GPSComponent::process_data(Message::ResponseMessage* data) {
}