#ifndef STEERING_COMPONENT_H
#define STEERING_COMPONENT_H

#include "../component.h"

class SteeringComponent : public Component {
public:
    SteeringComponent(AutonomousAgent* autonomous_agent, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data, const unsigned int id) override;

private:
    int _command_value;
};

#endif // STEERING_COMPONENT_H