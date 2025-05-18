#ifndef BREAK_COMPONENT_H
#define BREAK_COMPONENT_H

#include "../component.h"

class BreakComponent : public Component {
public:
    BreakComponent(Vehicle* vehicle, const unsigned short& id);

    void run() override;

    void generate_data() override;

    void set_interests() override;

    void process_data(Message::ResponseMessage* data) override;
};

#endif // BREAK_COMPONENT_H