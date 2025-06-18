#ifndef LIDAR_COMPONENT_H
#define LIDAR_COMPONENT_H

#include <random>

#include "../component.h"

class LidarComponent : public Component {
public:
    LidarComponent(AutonomousAgent* autonomous_agent, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data, const unsigned int id) override;
};

#endif // LIDAR_COMPONENT_H