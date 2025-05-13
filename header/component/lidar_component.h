#ifndef LIDAR_COMPONENT_H
#define LIDAR_COMPONENT_H

#include "../component.h"

class LidarComponent : public Component {
public:
    LidarComponent(Vehicle* vehicle, const unsigned short& id);

    void generate_data() override;
    void set_interests() override;
};

#endif // LIDAR_COMPONENT_H
