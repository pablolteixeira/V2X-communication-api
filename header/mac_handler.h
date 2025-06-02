#ifndef MAC_HANDLER_H
#define MAC_HANDLER_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>


#include "ethernet.h"


// Define o tamanho do MAC em bytes. Um MAC de 4 bytes pode ser representado como uint32_t.
const size_t DEFAULT_MAC_BYTE_SIZE = 4;

class MACHandler 
{

public:
    MACHandler(size_t mac_size_bytes = DEFAULT_MAC_BYTE_SIZE);

    void set_mac_key(const std::vector<unsigned char>& key);

    uint32_t generate_mac(const unsigned char* data, size_t data_length) const;
    bool verify_mac(const unsigned char* data, size_t data_length, uint32_t received_mac) const;

private:
    std::vector<unsigned char> _mac_key;
    bool _key_is_set;
    size_t _mac_byte_size;
};

#endif // MAC_HANDLER_H