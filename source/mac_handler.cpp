#include "../header/mac_handler.h"

MACHandler::MACHandler(size_t mac_size_bytes) : 
    _key_is_set(false), _mac_byte_size(mac_size_bytes) {
}

void MACHandler::set_mac_key(Ethernet::MAC_KEY *key) {
    // for(size_t i = 0; i < Ethernet::DEFAULT_MAC_BYTE_SIZE; i++) {
    //     if (key[i]) {
    //         _key_is_set = false;
    //         throw std::invalid_argument("A chave MAC não pode ser vazia.");
    //     }
    // }
    memcpy(&_mac_key, key, Ethernet::DEFAULT_MAC_BYTE_SIZE);
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
    for(size_t i = 0; i < Ethernet::DEFAULT_MAC_BYTE_SIZE; i++) {
        key[i] = static_cast<unsigned char>(distrib(gen));
    }
    set_mac_key(&key);
}

void MACHandler::print_mac_key() {
    std::cout << "Chave gerada (" << Ethernet::DEFAULT_MAC_BYTE_SIZE << " bytes): ";
    std::cout << std::hex;
    for (unsigned char byte : _mac_key) {
        std::cout << (static_cast<int>(byte) & 0xFF) << " ";
    }
    std::cout << std::dec << std::endl;
}

uint32_t MACHandler::generate_mac(const unsigned char* data, size_t data_length) const {
    if (!_key_is_set) {
        throw std::runtime_error("A chave MAC não está definida. Não é possível gerar MAC.");
    }

    std::vector<unsigned char> mac_buffer(this->_mac_byte_size, 0);

    // 1. Faz XOR dos bytes dos dados no buffer do MAC
    // Cada byte dos dados afeta um byte do mac_buffer de forma cíclica.
    for (size_t i = 0; i < data_length; ++i) {
        mac_buffer[i % this->_mac_byte_size] ^= data[i];
    }

    // 2. Faz XOR dos bytes da chave no buffer do MAC
    // A chave inteira é "dobrada" (folded) no mac_buffer.
    for (size_t i = 0; i < Ethernet::DEFAULT_MAC_BYTE_SIZE; ++i) {
        mac_buffer[i % this->_mac_byte_size] ^= _mac_key[i];
    }

    // 3. Converte o mac_buffer (std::vector<unsigned char>) para uint32_t.
    // A conversão assume uma ordem de bytes Big Endian para consistência:
    // mac_buffer[0] é o byte mais significativo (MSB).
    uint32_t result_mac = 0;
    if (!mac_buffer.empty()) {
        result_mac = mac_buffer[0]; // Inicializa com o primeiro byte (MSB)
        for (size_t i = 1; i < this->_mac_byte_size; ++i) {
            result_mac = (result_mac << 8) | mac_buffer[i];
        }
    }
    
    // Se mac_byte_size < sizeof(uint32_t), os bits mais significativos de result_mac serão 0.
    // Ex: se mac_byte_size = 2, result_mac será (mac_buffer[0] << 8) | mac_buffer[1].
    
    return result_mac;
}

bool MACHandler::verify_mac(const unsigned char* data, size_t data_length, uint32_t received_mac) const {
    if (!_key_is_set) {
        return false; 
    }
    
    uint32_t calculated_mac = generate_mac(data, data_length);
    return calculated_mac == received_mac;
}