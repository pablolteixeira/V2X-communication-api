#ifndef RAW_SOCKET_ENGINE_H
#define RAW_SOCKET_ENGINE_H

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ifaddrs.h>
#include <set>
#include <string>

#include "ethernet.h"
#include "console_logger.h"

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
        //ConsoleLogger::print("Raw Socket Engine: Setting interface index.");
        
        std::string interface_name = get_interface();

        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, interface_name.c_str()); // Default interface, can be configurable
                
        if(ioctl(_socket, SIOCGIFINDEX, &ifr) < 0) {
            ConsoleLogger::error("SIOCGIFINDEX");
            close(_socket);
            throw std::runtime_error("Falha ao obter índice da interface");
        }
        _ifindex = ifr.ifr_ifindex;

        // Get MAC address
        //ConsoleLogger::print("Raw Socket Engine: Getting MAC Address.");

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

        //ConsoleLogger::print("Raw Socket Engine: MAC Address = " + mac.str());
        memcpy(_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    }
    
    ~RawSocketEngine() {
        if(_socket >= 0)
            close(_socket);
    }
    
    int raw_send(Ethernet::Address dst, Ethernet::Protocol prot, Ethernet::Footer* footer, const void* data, unsigned int size) {
        //ConsoleLogger::print("Raw Socket Engine: Sending frame.");
        Ethernet::Frame frame(dst, _addr, prot);
        //ConsoleLogger::print("Raw Socket Engine:PROTO -> " + std::to_string(prot));
        ConsoleLogger::log("Before Send Timestamp: " + std::to_string(footer->get_timestamp()));
        ConsoleLogger::log("Before Send Sync State: " + std::to_string(footer->get_sync_state()));
        ConsoleLogger::log("Before Send Packet Origin: " + std::to_string(footer->get_packet_origin()));

        memcpy(frame.footer(), footer, sizeof(Ethernet::Footer));
        memcpy(frame.data(), data, size);
        
        ConsoleLogger::log("Send Timestamp: " + std::to_string(frame.footer()->get_timestamp()));
        ConsoleLogger::log("Send Sync State: " + std::to_string(frame.footer()->get_sync_state()));
        ConsoleLogger::log("Send Packet Origin: " + std::to_string(frame.footer()->get_packet_origin()));

        struct sockaddr_ll socket_address;
        socket_address.sll_family = AF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ALL);
        socket_address.sll_ifindex = _ifindex;
        socket_address.sll_halen = ETH_ALEN;
        memcpy(socket_address.sll_addr, dst, ETH_ALEN);
        
        int bytes_sent = sendto(_socket, &frame, sizeof(Ethernet::Header) + sizeof(Ethernet::Footer) + size, 0,
                               (struct sockaddr*)&socket_address, sizeof(socket_address));

        ConsoleLogger::log("Bytes sent: " + std::to_string(bytes_sent));

        //ConsoleLogger::print("Raw Socket Engine: Frame sent.");                  
        return bytes_sent - sizeof(Ethernet::Header) - sizeof(Ethernet::Footer);
    }
    
    int raw_receive(Ethernet::Address* src, Ethernet::Protocol* prot, Ethernet::Footer* footer, void* data, unsigned int size) {
        //ConsoleLogger::print("Raw Socket Engine: Receive started.");  
        Ethernet::Frame frame;
        int bytes_received = recvfrom(_socket, &frame, sizeof(frame), 0, NULL, NULL);
        ConsoleLogger::log("Bytes received: " + std::to_string(bytes_received));
        if(bytes_received < 0)
            return -1;
        
        memcpy(src, frame.header()->h_source, ETH_ALEN);
        memcpy(footer, (char*)&frame + sizeof(Ethernet::Header), sizeof(Ethernet::Footer));
        *prot = ntohs(frame.header()->h_proto);

        ConsoleLogger::log("Timestamp: " + std::to_string(footer->get_timestamp()));
        ConsoleLogger::log("Sync State: " + std::to_string(footer->get_sync_state()));
        ConsoleLogger::log("Packet Origin: " + std::to_string(footer->get_packet_origin()));

        //ConsoleLogger::print("Raw Socket Engine: Receive PROTO -> " + std::to_string(*prot));
        int data_size = bytes_received - sizeof(Ethernet::Header) - sizeof(Ethernet::Footer);
        if(data_size > 0) {
            int copy_size = (data_size > (int)size) ? size : data_size;
            memcpy(data, frame.data(), copy_size);
            return copy_size;
        }
        
        return 0;
    }

    std::string get_interface() {
        struct ifaddrs* ifaddr;
    
        if (getifaddrs(&ifaddr) == -1) {
            ConsoleLogger::error("Interfaces not recognized");
            throw std::runtime_error("Falha ao carregar interfaces");
        }
    
        std::string name;
        std::set<std::string> interfaces;
    
        for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (ifa->ifa_name) {
                name = ifa->ifa_name;
                if (name.find("wl") != std::string::npos) {
                    break;
                }
            }
        }
        freeifaddrs(ifaddr);
        return name;
    }

protected:
    int _socket;
    int _ifindex;
    Ethernet::Address _addr;
};

#endif // RAW_SOCKET_ENGINE_H