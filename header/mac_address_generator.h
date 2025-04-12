#ifndef MAC_ADDRESS_GENERATOR_H
#define MAC_ADDRESS_GENERATOR_H

#include "ethernet.h"
#include <iostream>
#include <string>
#include <cstring>
#include <cstdint>
#include <iomanip>
#include <random>
#include <netinet/ether.h>

class MacAddressGenerator 
{
public:
    static void generate_mac_from_seed(const std::string& seed, Ethernet::Address mac) {
        // Convert string to an integer seed
        unsigned int seed_value = hash_string(seed);
        
        // Setup random number generator with the seed
        std::mt19937 rng(seed_value);
        std::uniform_int_distribution<int> dist(0, 255);
        
        // Generate random bytes
        for (int i = 0; i < ETH_ALEN; i++) {
            mac[i] = static_cast<unsigned char>(dist(rng));
        }
    }
    
    static void print_mac(const Ethernet::Address mac) {
        for (int i = 0; i < ETH_ALEN; i++) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') 
                      << static_cast<int>(mac[i]);
            if (i < ETH_ALEN - 1) {
                std::cout << ":";
            }
        }
        std::cout << std::dec << std::endl;
    }
private:
    static unsigned int hash_string(const std::string& str) {
        unsigned int hash = 5381;
        
        for (char c : str) {
            hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        
        return hash;
    }
};

#endif // MAC_ADDRESS_GENERATOR_H