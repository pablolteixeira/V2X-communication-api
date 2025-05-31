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

#include "u64_type.h"

// Network
class Ethernet 
{
public:
    static const unsigned int MTU = ETH_FRAME_LEN;
    static const unsigned char BROADCAST_MAC[ETH_ALEN];
    
    typedef struct ethhdr Header;
    typedef unsigned char Address[ETH_ALEN];
    typedef unsigned short Protocol;

    class Metadata 
    {
    public:
        enum SyncState {
            NOT_SYNCHRONIZED,
            SYNCHRONIZED
        };

        enum PacketOrigin {
            RSU,
            OTHERS
        };
        
        Metadata() : _timestamp(0), _sync_state(SyncState::NOT_SYNCHRONIZED), _packet_origin(PacketOrigin::OTHERS) {}
        Metadata(U64 timestamp, SyncState sync_state, PacketOrigin packet_origin) : _timestamp(timestamp), _sync_state(sync_state), _packet_origin(packet_origin) {}

        U64 get_timestamp() { return _timestamp; }
        SyncState get_sync_state() { return _sync_state; }
        PacketOrigin get_packet_origin() {return _packet_origin; }

        void set_timestamp(U64 timestamp) {_timestamp = timestamp; }
        void set_sync_state(SyncState sync_state) { _sync_state = sync_state; }
        void set_packet_origin(PacketOrigin packet_origin) { _packet_origin = packet_origin; }

    private:
        U64 _timestamp;
        SyncState _sync_state;
        PacketOrigin _packet_origin;
    } __attribute__((packed));

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
        Metadata* metadata() { return &_metadata; }
        unsigned char* data() { return _data; }
        
    private:
        Header _header;
        Metadata _metadata;
        unsigned char _data[ETH_FRAME_LEN - sizeof(Header) - sizeof(Metadata)];
    } __attribute__((packed));
    
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