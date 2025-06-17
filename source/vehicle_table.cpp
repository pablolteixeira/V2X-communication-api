#include "../header/vehicle_table.h"

VehicleTable::VehicleTable() {
    _vehicles.clear();

    ConsoleLogger::log("Created vehicle table");
}

bool VehicleTable::check_vehicle(Ethernet::Address* address) {
    for(auto vehicle_add : _vehicles) {
        if(memcmp(vehicle_add.data(), address, ETH_ALEN) == 0) {
            return true;
        }
    }
    return false;
}

void VehicleTable::set_vehicle(std::array<unsigned char, ETH_ALEN> address) {
    _vehicles.push_back(address);
}