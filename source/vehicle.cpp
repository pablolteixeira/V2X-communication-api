#include "../header/nic.h"
#include "../header/smart_data.h"
#include "../header/agent/vehicle.h"
#include "../header/component/lidar_component.h"
#include "../header/component/gps_component.h"
#include "../header/component/steering_component.h"
#include "../header/component/controller_component.h"


Vehicle::Vehicle(EthernetNIC* nic, EthernetProtocol* protocol) 
    : AutonomousAgent(nic, protocol) {
    _components.push_back(new LidarComponent(this, 1));
    _components.push_back(new GPSComponent(this, 2));
    _components.push_back(new ControllerComponent(this, 3));
    _components.push_back(new SteeringComponent(this, 4));
}
