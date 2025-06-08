#include "../header/vehicle_table.h"

VehicleTable::VehicleTable() {
    
    _vehicles.clear();
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<> distrib(0, 255);
    // _vehicles.reserve(Traits<RSU>::NUM_RSU);

    // for (int i = 0; i < Traits<RSU>::NUM_RSU; ++i) {
    //     Ethernet::MAC_KEY key;

    //     for (size_t j = 0; j < Ethernet::MAC_BYTE_SIZE; ++j) {
    //         key[j] = static_cast<unsigned char>(distrib(gen));
    //     }

    //     _vehicles.push_back(key);
    // }

    ConsoleLogger::log("Created vehicle table");
}

bool VehicleTable::check_vehicle(Ethernet::Address* address) {
    for(auto vehicle : _vehicles) {
        if(memcmp(vehicle, address, 6) == 0) {
            return true;
        };
    }
    return false;
}

void VehicleTable::set_vehicle(Ethernet::Address* address) {
    _vehicles.push_back(address);
}