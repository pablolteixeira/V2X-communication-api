#include "../header/vehicle.h"
#include "../header/nic.h"

#include <iostream>

Vehicle::Vehicle(int id, EthernetNIC* nic) : _id(id), _nic(nic) {
    //_protocol = new EthernetProtocol(_nic);
}

Vehicle::~Vehicle() {

}

void Vehicle::start() {
    std::cout << "Starting Vehicle -> " << _id << std::endl; 
}