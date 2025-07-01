#ifndef ACCELEROMETER_COMPONENT_H
#define ACCELEROMETER_COMPONENT_H

#include <random>

#include "../component.h"
#include "../csv.hpp"

using namespace csv;

class AccelerometerComponent : public Component {
public:
    AccelerometerComponent(AutonomousAgent* autonomous_agent, const unsigned short& id, int dataset_id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data, const unsigned int id) override;
private:
    std::atomic<double> _value;

    CSVReader* _reader;
    CSVRow _row;
};

#endif // ACCELEROMETER_COMPONENT_H