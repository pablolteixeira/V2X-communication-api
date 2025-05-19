#include "../header/component/gps_component.h"
#include <random>

void GPSComponent::run() 
{
    ConsoleLogger::log("About to generate GPS data");
    generate_data();
    ConsoleLogger::log("GPS data generation completed");
}

GPSComponent::GPSComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id) {
    // SI unit: position (lat/long) => 0b1 << 31 | 10 << 18; // Representing position data
    _data_type = 0b1 << 31 | 10 << 18;  // Position data type
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