#ifndef ETHERNET_H
#define ETHERNET_H

// Incluir os cabeçalhos padrão do Linux para ethernet
#include <netinet/ether.h>
#include <netinet/in.h>
#include <cstring>
#include <cstdio>
#include <cstdint>

#include <string>
#include <sstream>
#include <iomanip>

// Network
class Ethernet 
{
public:
    static const unsigned int MTU = ETH_FRAME_LEN;
    
    typedef struct ethhdr Header;
    typedef unsigned char Address[ETH_ALEN];
    typedef unsigned short Protocol;
    
    class Frame 
    {
    public:
        Frame() {}
        Frame(Address dst, Address src, Protocol prot) {
            memcpy(_header.h_dest, dst, ETH_ALEN);
            memcpy(_header.h_source, src, ETH_ALEN);
            _header.h_proto = htons(prot);
        }
        
        Header* header() { return &_header; }
        unsigned char* data() { return _data; }
        
    private:
        Header _header;
        unsigned char _data[ETH_FRAME_LEN - sizeof(Header)];
    };
    
    static std::string address_to_string(Address addr) {
        std::stringstream ss;
        
        // Format each byte with leading zeros and colons
        for (int i = 0; i < 6; ++i) {
            // Add colon after first byte
            if (i > 0) {
                ss << ":";
            }
            
            // Format as hex with leading zeros
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(addr[i]);
        }
        
        return ss.str();
    }
    
    static bool is_broadcast(Address addr) {
        for(int i = 0; i < ETH_ALEN; i++)
            if(addr[i] != 0xFF)
                return false;
        return true;
    }
};

#endif // ETHERNET_H