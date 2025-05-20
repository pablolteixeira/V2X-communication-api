#include "../header/component/steering_component.h"

void SteeringComponent::run() 
{
    ConsoleLogger::log("Steering actuator running");
    generate_data();
    ConsoleLogger::log("Steering actuation completed");
}

SteeringComponent::SteeringComponent(Vehicle* vehicle, const unsigned short& id)
    : Component(vehicle, id), _command_value(0) {
    // SI unit: angle (degrees) => ComponentDataTypes::ANGLE_DATA_TYPE;
    _data_type = ComponentDataTypes::ANGLE_DATA_TYPE;  // Angle data type
}

void SteeringComponent::generate_data() {
    // Simply reflect the command we received, representing the actual steering angle achieved
    _value = _command_value;
    
    // Add some "actuation noise" to simulate real-world conditions
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(-2, 2);
    _value += dist(gen);
    
    ConsoleLogger::log("Steering actuated to: " + std::to_string(_value) + " degrees");
}

void SteeringComponent::set_interests() {
    // Interested in Controller command data
    ComponentDataType controller_data_type = ComponentDataTypes::DIGITAL_COMAND_TYPE;
    _interests.push_back({controller_data_type, std::chrono::microseconds(100 * 1000), InterestBroadcastType::INTERNAL});
}

void SteeringComponent::process_data(Message::ResponseMessage* data) {
    _command_value = data->value;
    ConsoleLogger::log("Received steering command: " + std::to_string(_command_value));
}
