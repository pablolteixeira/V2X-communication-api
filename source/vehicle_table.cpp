#include <../header/vehicle_table.h>

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
    auto q = std::find_if(_vehicles.begin(), vehicles.end(), [](Ethernet::Address* add){memcmp(add, address, 6)})
    if(q == m.end()){
        return false;
    }
    return true;
}

void VehicleTable::set_vehicle(Ethernet::Address* address) {
    _vehicles.insert(address);
}