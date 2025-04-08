class RawSocketEngine 
{
protected:
    RawSocketEngine() {
        _socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
        if(_socket < 0) {
            perror("Socket creation failed");
            throw std::runtime_error("Falha ao criar socket raw");
        }
        
        // Get interface index
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "eth0"); // Default interface, can be configurable
        if(ioctl(_socket, SIOCGIFINDEX, &ifr) < 0) {
            perror("SIOCGIFINDEX");
            close(_socket);
            throw std::runtime_error("Falha ao obter índice da interface");
        }
        _ifindex = ifr.ifr_ifindex;
        
        // Get MAC address
        if(ioctl(_socket, SIOCGIFHWADDR, &ifr) < 0) {
            perror("SIOCGIFHWADDR");
            close(_socket);
            throw std::runtime_error("Falha ao obter endereço MAC da interface");
        }
        memcpy(_addr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    }
    
    ~RawSocketEngine() {
        if(_socket >= 0)
            close(_socket);
    }
    
    int raw_send(Ethernet::Address dst, Ethernet::Protocol prot, const void* data, unsigned int size) {
        Ethernet::Frame frame(dst, _addr, prot);
        memcpy(frame.data(), data, size);
        
        struct sockaddr_ll socket_address;
        socket_address.sll_family = AF_PACKET;
        socket_address.sll_protocol = htons(ETH_P_ALL);
        socket_address.sll_ifindex = _ifindex;
        socket_address.sll_halen = ETH_ALEN;
        memcpy(socket_address.sll_addr, dst, ETH_ALEN);
        
        int bytes_sent = sendto(_socket, &frame, sizeof(Ethernet::Header) + size, 0,
                               (struct sockaddr*)&socket_address, sizeof(socket_address));
        
        return bytes_sent - sizeof(Ethernet::Header);
    }
    
    int raw_receive(Ethernet::Address* src, Ethernet::Protocol* prot, void* data, unsigned int size) {
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