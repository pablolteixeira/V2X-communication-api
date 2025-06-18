#ifndef CONTROLLER_COMPONENT_H
#define CONTROLLER_COMPONENT_H

#include "../component.h"

class ControllerComponent : public Component {
public:
    ControllerComponent(AutonomousAgent* autonomous_agent, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data, const unsigned int id) override;

private:
    int _lidar_value;
    int _gps_value;
    int _external_gps_value;
};

#endif // CONTROLLER_COMPONENT_H