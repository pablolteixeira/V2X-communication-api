#include "../header/component/lidar_component.h"
#include <random>


void LidarComponent::run() 
{
    ConsoleLogger::log("About to generate data");
    generate_data();
    ConsoleLogger::log("Data generation completed");
}

LidarComponent::LidarComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id) {
    // SI unit: meter => m+1 => 0b101 (5) => m+4 = 9
    _data_type = 0b1 << 31 | 9 << 18;  // Only m+4 is set
}

void LidarComponent::generate_data() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100); // Lidar range in meters

    _value = dist(gen);
    ConsoleLogger::log("Data generated: " + std::to_string(_value));
}

void LidarComponent::set_interests() {
    ComponentDataType _control_data_type = 0b0 << 31;
    _interests.push_back({_control_data_type, std::chrono::milliseconds(10) });
    _interests.push_back({_data_type, std::chrono::milliseconds(10)});
}

void LidarComponent::process_data(Message::ResponseMessage* data) {

}

/*
unsigned int length =
1 << 31 | 0 << 27 |
(4 + 0) << 24 | (4 + 0) << 21 |
(4 + 1) << 18 | (4 + 0) << 15 |
(4 + 0) << 12 | (4 + 0) << 9 |
(4 + 0) << 6 | (4 + 0) << 3 | (4 + 0);


Bit 31 (T) = 0     (Digital type, not SI units)
Bits 30-29 (N) = 0  (Using Int32 representation)
Bits 28-27 (M) = 0   (Direct scaling, no manipulation)
Bits 26-24: Sensor type (e.g., 001 = temperature)
Bits 23-20: Temperature units (e.g., 0001 = Celsius)
Bits 19-16: Precision bits (e.g., 0010 = 2 decimal places)
Bits 15-0: Reserved for other metadata
*/

