#ifndef MAC_HANDLER_H
#define MAC_HANDLER_H

#include <vector>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <random>
#include <algorithm>
#include <iostream>


#include "ethernet.h"


// Define o tamanho do MAC em bytes. Um MAC de 4 bytes pode ser representado como uint32_t.


class MACHandler 
{

public:
    MACHandler(size_t mac_size_bytes = Ethernet::DEFAULT_MAC_BYTE_SIZE);

    void create_mac_key();
    void set_mac_key(Ethernet::MAC_KEY *key);
    Ethernet::MAC_KEY* get_mac_key();

    void print_mac_key();

    uint32_t generate_mac(const unsigned char* data, size_t data_length) const;
    bool verify_mac(const unsigned char* data, size_t data_length, uint32_t received_mac) const;

private:
    Ethernet::MAC_KEY _mac_key;
    bool _key_is_set;
    size_t _mac_byte_size;
};

#endif // MAC_HANDLER_H