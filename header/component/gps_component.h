#ifndef GPS_COMPONENT_H
#define GPS_COMPONENT_H

#include "../component.h"

class GPSComponent : public Component {
public:
    GPSComponent(Vehicle* vehicle, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data) override;
};

#endif // GPS_COMPONENT_H