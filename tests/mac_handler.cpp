#include "../header/mac_handler.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

bool test_mac_handler() {
    const unsigned char* key_char = "supersecretkey";
    std::vector<unsigned char> key;
    key.assign(key_char, key_char + 14);
    const unsigned char* message =
        "Cappu-cappu-cappuccino Assassino! Assassini cappuccini! "
        "Questo killer furtivo si infiltra tra i nemici approffitando de la notte. "
        "Attento, oddiatore di caffè! Se non bevi una tazza di cappuccino al mattino, "
        "è meglio non incrociare questo tizio.";

    // Generate MAC
    MACHandler mac_handler = MACHandler();
    mac_handler.set_mac_key(key);
    Ethernet::MAC mac = mac_handler.generate_mac(message.data(), message.size());

    // Verify MAC
    bool result = mac_handler.verify_mac(message.data(), message.size(), mac);

    return result;
}

int main() {
    bool result = test_mac_handler();

    // Check that verification passed
    assert(result && "MAC verification failed");

    std::cout << "MAC test passed!" << std::endl;
    return 0;
}