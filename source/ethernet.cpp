// Network
#include "../header/ethernet.h"


bool Ethernet::is_broadcast(Address addr) {
    for(int i = 0; i < ETH_ALEN; i++)
        if(addr[i] != 0xFF)
            return false;
    return true;
}

void Ethernet::address_to_string(Address addr, char* str) {
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x",
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

Ethernet::Frame::Frame(Address dst, Address src, Protocol prot) {
    memcpy(_header.h_dest, dst, ETH_ALEN);
    memcpy(_header.h_source, src, ETH_ALEN);
    _header.h_proto = htons(prot);
}