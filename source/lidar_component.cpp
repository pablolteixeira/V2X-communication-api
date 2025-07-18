#include "../header/component/lidar_component.h"

void LidarComponent::run() 
{
    ConsoleLogger::log("About to generate Lidar data");
    generate_data();
    ConsoleLogger::log("Lidar data generation completed");
}

LidarComponent::LidarComponent(AutonomousAgent* autonomous_agent, const unsigned short& id)
    : Component(autonomous_agent, id) {
    // SI unit: meter => m+1 => 0b101 (5) => m+4 = 9
    _data_type = ComponentDataTypes::METER_DATATYPE;  // Only m+4 is set
}

void LidarComponent::generate_data() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, 100); // Lidar range in meters

    _value = dist(gen);
    ConsoleLogger::log("Lidar data generated: " + std::to_string(_value));
}

void LidarComponent::set_interests() {
}

void LidarComponent::process_data(Message::ResponseMessage* data, const unsigned int id) {
    Ethernet::MessageInfo message_info = get_message_info(id);
    ConsoleLogger::log("Lidar Component: Message info received: Origin MAC address -> " + mac_to_string(message_info.origin_mac) +
        "; Origin ID -> " + std::to_string(message_info.origin_id) +
        "; Timestamp -> " + std::to_string(message_info.timestamp) + 
        "; Quadrant -> " + std::to_string(message_info.quadrant) +
        "; MAC -> " + std::to_string(message_info.mac));
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

