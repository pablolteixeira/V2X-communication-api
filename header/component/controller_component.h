#ifndef CONTROLLER_COMPONENT_H
#define CONTROLLER_COMPONENT_H

#include "../component.h"

class ControllerComponent : public Component {
public:
    ControllerComponent(Vehicle* vehicle, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data) override;

private:
    int _lidar_value;
    int _gps_value;
    int _external_gps_value;
};

#endif // CONTROLLER_COMPONENT_H