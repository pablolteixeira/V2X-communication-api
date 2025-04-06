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
        Frame(Address dst, Address src, Protocol prot);
    
        Header* header() { return &_header; }
        unsigned char* data() { return _data; }
    
    private:
        Header _header;
        unsigned char _data[ETH_FRAME_LEN - sizeof(Header)];
    } __attribute__((packed));

    static void address_to_string(Address addr, char* str);

    static bool is_broadcast(Address addr);
};

#endif ETHERNET_H