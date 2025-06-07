#include <../header/mac_key_table.h>

MacKeyTable::MacKeyTable() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);

    _macs_key.clear();
    _macs_key.reserve(Traits<RSU>::NUM_RSU);

    for (int i = 0; i < Traits<RSU>::NUM_RSU; ++i) {
        Ethernet::MAC_KEY key;

        for (size_t j = 0; j < Ethernet::DEFAULT_MAC_BYTE_SIZE; ++j) {
            key[j] = static_cast<unsigned char>(distrib(gen));
        }

        _macs_key.push_back(key);
    }

    ConsoleLogger::log("Created mac key table");
}

void MacKeyTable::print_mac_key_vector() {
    std::ios_base::fmtflags original_flags = std::cout.flags();

    std::cout << "\n Chaves MAC" << std::endl;
    if (_macs_key.empty()) {
        std::cout << "O vetor de MACs está vazio." << std::endl;
        return;
    }

    for (const auto& mac : _macs_key) {
        for (size_t i = 0; i < Ethernet::DEFAULT_MAC_BYTE_SIZE; ++i) {
            std::cout << std::hex << std::uppercase
                        << std::setw(2) << std::setfill('0')
                        << static_cast<int>(mac[i]);

            if (i < Ethernet::DEFAULT_MAC_BYTE_SIZE - 1) {
                std::cout << ":";
            }
        }
        std::cout << std::endl;
    }
    std::cout.flags(original_flags);
};