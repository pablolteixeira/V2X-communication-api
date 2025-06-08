#ifndef VEHICLE_TABLE
#define VEHICLE_TABLE

#include <vector>
#include <random>

#include "traits.h"
#include "ethernet.h"
#include "console_logger.h"

class VehicleTable {
public:
    VehicleTable();
    bool check_vehicle(Ethernet::Address* address);
    void set_vehicle(Ethernet::Address* address);

private:
    std::vector<Ethernet::Address* > _vehicles;
};

#endif // VEHICLE_TABLE