#ifndef RAW_SOCKET_ENGINE_H
#define RAW_SOCKET_ENGINE_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include "ethernet.h"
#include "console_logger.h"

#include <iostream>
#include <sstream>
#include <iomanip>

class RawSocketEngine 
{
protected:
    RawSocketEngine() {
        _socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(_socket < 0) {
            ConsoleLogger::error("Socket creation failed");
            throw std::runtime_error("Falha ao criar socket raw");
        }
        
        // Get interface index
        ConsoleLogger::print("Raw Socker Engine:: Setting interface index.");
        
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "wlp4s0"); // Default interface, can be configurable
                
        if(ioctl(_socket, SIOCGIFINDEX, &ifr) < 0) {
            ConsoleLogger::error("SIOCGIFINDEX");
            close(_socket);
            throw std::runtime_error("Falha ao obter índice da interface");
        }
        _ifindex = ifr.ifr_ifindex;

        // Get MAC address
        ConsoleLogger::print("Raw Socker Engine:: Getting MAC Address.");

        if(ioctl(_socket, SIOCGIFHWADDR, &ifr) < 0) {
            ConsoleLogger::error("SIOCGIFHWADDR");
            close(_socket);
            throw std::runtime_error("Falha ao obter endereço MAC da interface");
        }
        
        std::stringstream mac;
        mac << std::hex << std::setfill('0');
        for (int i = 0; i < ETH_ALEN; i++) {
            mac << std::setw(2) << (static_cast<unsigned int>(ifr.ifr_hwaddr.sa_data[i]) & 0xFF);
            if (i < ETH_ALEN - 1) {
                mac << ":";
            }
        }

        std::cout << "MAC Address: " << mac.str() << std::endl;
        memcpy(_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    }
    
    ~RawSocketEngine() {
        if(_socket >= 0)
            close(_socket);
    }
    
    int raw_send(Ethernet::Address dst, Ethernet::Protocol prot, const void* data, unsigned int size) {
        ConsoleLogger::print("Raw Socker Engine: Sending frame.");
        Ethernet::Frame frame(dst, _addr, prot);
        std::cout << "PROTO -> " << prot << std::endl;
        memcpy(frame.data(), data, size);
        
        struct sockaddr_ll socket_address;
        socket_address.sll_family = AF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ALL);
        socket_address.sll_ifindex = _ifindex;
        socket_address.sll_halen = ETH_ALEN;
        memcpy(socket_address.sll_addr, dst, ETH_ALEN);
        
        int bytes_sent = sendto(_socket, &frame, sizeof(Ethernet::Header) + size, 0,
                               (struct sockaddr*)&socket_address, sizeof(socket_address));
        
        ConsoleLogger::print("Raw Socker Engine: Frame sent.");                  
        return bytes_sent - sizeof(Ethernet::Header);
    }
    
    int raw_receive(Ethernet::Address* src, Ethernet::Protocol* prot, void* data, unsigned int size) {
        ConsoleLogger::print("Raw Socker Engine: Receive started.");  
        Ethernet::Frame frame;
        int bytes_received = recvfrom(_socket, &frame, sizeof(frame), 0, NULL, NULL);
        
        if(bytes_received < 0)
            return -1;
        
        memcpy(src, frame.header()->h_source, ETH_ALEN);
        *prot = ntohs(frame.header()->h_proto);
        
        int data_size = bytes_received - sizeof(Ethernet::Header);
        if(data_size > 0) {
            int copy_size = (data_size > (int)size) ? size : data_size;
            memcpy(data, frame.data(), copy_size);
            return copy_size;
        }
        
        return 0;
    }
    
protected:
    int _socket;
    int _ifindex;
    Ethernet::Address _addr;
};

#endif // RAW_SOCKET_ENGINE_H