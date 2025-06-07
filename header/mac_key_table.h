#ifndef MAC_KEY_TABLE
#define MAC_KEY_TABLE

#include <vector>
#include <random>
#include <ethernet.h>

#include <traits.h>
#include <console_logger.h>
#include "agent/rsu.h"

class MacKeyTable {
public:
    MacKeyTable();
    void print_mac_key_vector();

private:
    std::vector<Ethernet::MAC_KEY> _macs_key;
};

#endif // MAC_KEY_TABLE