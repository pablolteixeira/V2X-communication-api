#include "../header/mac_handler.h"
#include "../header/console_logger.h"

MACHandler::MACHandler(size_t mac_size_bytes) : 
    _key_is_set(false), _mac_byte_size(mac_size_bytes) {
}

void MACHandler::set_mac_key(Ethernet::MAC_KEY *key) {
    memcpy(&_mac_key, key, Ethernet::MAC_BYTE_SIZE);
    _key_is_set = true;
}

Ethernet::MAC_KEY* MACHandler::get_mac_key() {
    return &_mac_key;
}

void MACHandler::create_mac_key() {
    Ethernet::MAC_KEY key;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    for(size_t i = 0; i < Ethernet::MAC_BYTE_SIZE; i++) {
        key[i] = static_cast<unsigned char>(distrib(gen));
    }
    set_mac_key(&key);
}

void MACHandler::print_mac_key() {
    std::cout << "Chave gerada (" << Ethernet::MAC_BYTE_SIZE << " bytes): ";
    std::cout << std::hex;
    for (unsigned char byte : _mac_key) {
        std::cout << (static_cast<int>(byte) & 0xFF) << " ";
    }
    std::cout << std::dec << std::endl;
}

uint32_t MACHandler::generate_mac(const unsigned char* data, size_t data_length) const {
    if (!_key_is_set) {
        return 0;
    }

    std::vector<unsigned char> mac_buffer(this->_mac_byte_size, 0);

    for (size_t i = 0; i < data_length; ++i) {
        mac_buffer[i % this->_mac_byte_size] ^= data[i];
    }

    for (size_t i = 0; i < Ethernet::MAC_BYTE_SIZE; ++i) {
        mac_buffer[i % this->_mac_byte_size] ^= _mac_key[i];
    }

    uint32_t result_mac = 0;
    if (!mac_buffer.empty()) {
        result_mac = mac_buffer[0]; // Inicializa com o primeiro byte (MSB)
        for (size_t i = 1; i < this->_mac_byte_size; ++i) {
            result_mac = (result_mac << 8) | mac_buffer[i];
        }
    }
    
    return result_mac;
}

bool MACHandler::verify_mac(const unsigned char* data, size_t data_length, uint32_t received_mac) const {
    if (!_key_is_set) {
        return false; 
    }

    ConsoleLogger::log("Checking mac integrity");
    uint32_t calculated_mac = generate_mac(data, data_length);
    ConsoleLogger::log("MAC comparison " + std::to_string(calculated_mac)+ " : " + std::to_string(received_mac));
    calculated_mac == received_mac ? ConsoleLogger::log("MAC correct") : ConsoleLogger::log("MAC incorrect");
    return calculated_mac == received_mac;
}