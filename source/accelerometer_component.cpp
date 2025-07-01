#include "../header/component/accelerometer_component.h"

void AccelerometerComponent::run() 
{
    ConsoleLogger::log("About to generate Accelerometer data");
    generate_data();
    ConsoleLogger::log("Accelerometer data generation completed");
}

AccelerometerComponent::AccelerometerComponent(AutonomousAgent* autonomous_agent, const unsigned short& id, int dataset_id)
    : Component(autonomous_agent, id) {
    // SI unit: meter => m+1 => 0b101 (5) => m+4 = 9
    _data_type = ComponentDataTypes::ACCELERATION_DATA_TYPE;
    _reader = new CSVReader("dataset/dynamics-vehicle_"+ std::to_string(dataset_id) + ".csv");
}

void AccelerometerComponent::generate_data() {
    _reader->read_row(_row); 

    _value = _row["acceleration"].get<double>();
    ConsoleLogger::log("Accelerometer data generated: " + std::to_string(_value));
}

void AccelerometerComponent::set_interests() {
}

void AccelerometerComponent::process_data(Message::ResponseMessage* data, const unsigned int id) {
}
