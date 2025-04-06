#ifndef ETHERNET_H
#define ETHERNET_H

#include <net/ethernet.h>
#include <cstring>
#include <arpa/inet.h>
#include <cstdio>

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
    } __attribute__((packed));

    static void address_to_string(Address addr, char* str) {
        sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
            addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
    }

    static bool is_broadcast(Address addr) {
        for(int i = 0; i < ETH_ALEN; i++)
            if(addr[i] != 0xFF)
                return false;
        return true;
    }
};

#endif ETHERNET_H