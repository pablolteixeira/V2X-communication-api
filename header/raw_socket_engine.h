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
        
    }
    
    int raw_receive(Ethernet::Address* src, Ethernet::Protocol* prot, void* data, unsigned int size) {
        
    }
    
protected:
    int _socket;
    int _ifindex;
    Ethernet::Address _addr;
};

#endif // RAW_SOCKET_ENGINE_H