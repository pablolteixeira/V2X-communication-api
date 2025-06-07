#ifndef VEHICLE_TABLE
#define VEHICLE_TABLE

#include <vector>
#include <random>
#include <ethernet.h>

#include <traits.h>
#include <console_logger.h>
#include "agent/vehicle.h"

class VehicleTable {
public:
    VehicleTable();
    bool VehicleTable::check_vehicle(Ethernet::Address* address);
    void VehicleTable::set_vehicle(Ethernet::Address* address);

private:
    std::vector<Ethernet::Address* > _vehicles;
};

#endif // VEHICLE_TABLE