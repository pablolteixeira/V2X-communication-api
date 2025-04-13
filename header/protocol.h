#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "observer.h"
#include "buffer.h"
#include "traits.h"
#include "ethernet.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

template <typename NIC>
class Protocol: private NIC::Observer
{
public:
    static const typename NIC::Protocol_Number PROTO = Traits<Protocol>::ETHERNET_PROTOCOL_NUMBER;
    
    typedef typename NIC::Address Physical_Address;
    typedef unsigned short Port;
    typedef Concurrent_Observer<Buffer<Ethernet::Frame>, Port> Observer;
    typedef Concurrent_Observed<Buffer<Ethernet::Frame>, Port> Observed;
    typedef typename NIC::NICBuffer NICBuffer;

    class Address
    {
    public:
        enum Null {};

        static Physical_Address BROADCAST_MAC;
        static const Address BROADCAST;
        
    public:
        Address() : _port(0) {
            memset(_paddr, 0, sizeof(_paddr));
        };  
        Address(const Null & null) : _port(0) {
            memset(_paddr, 0, sizeof(_paddr));
        };
        Address(Physical_Address paddr, Port port) : _port(port) {
            memcpy(_paddr, paddr, sizeof(Physical_Address));
        }
        
        operator bool() const { return (_paddr || _port); }
        
        bool operator==(Address a) { return (_paddr == a._paddr) && (_port == a._port); }
        
        Port port() const { return _port; }
    
        Physical_Address& paddr() { return _paddr; }
    private:
        Physical_Address _paddr;
        Port _port;
    };

    class Header
    {
    public:
        Header() : _from_port(0), _to_port(0), _length(0) {}
        Header(Port from_port, Port to_port, unsigned short length)
            : _from_port(from_port), _to_port(to_port), _length(length) {}
        
        Port from_port() const { 
            return _from_port; 
        }
        Port to_port() const { 
            return _to_port; 
        }
        unsigned short length() const { 
            return _length; 
        }
        
    private:
        Port _from_port;
        Port _to_port;
        unsigned short _length;
    } __attribute__((packed));
    
    static const unsigned int MTU;
    typedef unsigned char Data[MTU];

    class Packet: public Header
    {
    public:
        Packet();
        
        Header * header();
        
        template<typename T>
        T * data() { 
            return reinterpret_cast<T*>(&_data); 
        }

    private:
        Data _data;
    } __attribute__((packed));

protected:
    Protocol() {}

public:
    ~Protocol() {
        for (auto nic : _nics) {
            nic->detach(this, PROTO);
        }
    }
    
    static Protocol* get_instance() {
        if (_instance == nullptr) {
            _instance = new Protocol();
        }

        return _instance;
    }

    static void register_nic(NIC* nic) {
        if (_instance) {
            ConsoleLogger::print("Protocol: Registering NIC");
            _nics.push_back(nic);
            std::string mac_string = mac_to_string(nic->address());
            
            std::cout << "Protocol: NIC Mac Address registered - " << mac_string << std::endl;
            
            _mac_table[mac_string] = nic;
            nic->attach(_instance, PROTO);
        }
    }

    static void unregister_nic(NIC* nic) {
        if (_instance) {
            ConsoleLogger::print("Protocol: Unregistering NIC");
            for (auto it = _nics.begin(); it != _nics.end(); ++it) {
                if (*it == nic) {
                    nic->detach(_instance, PROTO);
                    _nics.erase(it);
                    return;
                }
            }
        }
    }

    int send(Address from, Address to, const void * data, unsigned int size) {
        if (size > MTU) {
            return -1;
        }
        ConsoleLogger::print("Protocol: Sending message.");
        std::cout << "SIZES -> " << sizeof(Header) + size << " " << sizeof(Header) << " " << size << std::endl;

        NIC* nic = get_nic(from.paddr());
        NICBuffer* buf = nic->alloc(to.paddr(), PROTO, sizeof(Header) + size);
        if (!buf) {
            return -1;
        }

        ConsoleLogger::print("Protocol: Buffer allocated.");
        
        Ethernet::Frame* frame = buf->frame();

        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        packet->Header::operator=(Header(from.port(), to.port(), size));
        memcpy(packet->template data<void>(), data, size);
        
        int result = nic->send(buf);
        nic->free(buf);

        return result;
    }

    int receive(NICBuffer * buf, Address from, void * data, unsigned int size) {
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());

        if (packet->length() > size) {
            return -1;
        }

        NIC* nic = get_nic(from.paddr());

        Physical_Address paddr;
        nic->receive(buf, &paddr, nullptr, nullptr, 0);

        from = Address(paddr, packet->from_port());
        memcpy(data, packet->template data<void>(), packet->length());

        return packet->length();
    }

    static void attach(Observer * obs, Address address) {
        _observed.attach(obs, address.port());
    }
    static void detach(Observer * obs, Address address) {
        _observed.detach(obs, address.port());
    }

private:
    void update(typename NIC::Protocol_Number prot, NICBuffer * buf) override {
        ConsoleLogger::print("Protocol: Update observers.");
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        if(!_observed.notify(packet->to_port(), buf)) {
            //_nic->free(buf);
        } // to call receive(...);
    }

    static std::string mac_to_string(Physical_Address& addr) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < sizeof(Physical_Address); ++i) {
            if (i > 0) ss << ":";
            ss << std::setw(2) << static_cast<int>(addr[i]);
        }
        
        return ss.str();
    }

    static NIC* get_nic(Physical_Address& paddr) {
        std::string key = mac_to_string(paddr);
        return _mac_table[key];
    }

private:
    static Protocol* _instance;
    static std::vector<NIC*> _nics;
    static std::unordered_map<std::string, NIC*> _mac_table;
    // Channel protocols are usually singletons
    static Observed _observed;
};

template <typename NIC>
typename Protocol<NIC>::Physical_Address Protocol<NIC>::Address::BROADCAST_MAC = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

template <typename NIC>
const typename Protocol<NIC>::Address Protocol<NIC>::Address::BROADCAST(Protocol<NIC>::Address::BROADCAST_MAC, 0);

template <typename NIC>
const unsigned int Protocol<NIC>::MTU = NIC::MTU - sizeof(Protocol<NIC>::Header);

template <typename NIC>
typename Protocol<NIC>::Observed Protocol<NIC>::_observed;

template <typename NIC>
std::vector<NIC*> Protocol<NIC>::_nics;

template <typename NIC>
std::unordered_map<std::string, NIC*> Protocol<NIC>::_mac_table;

template <typename NIC>
Protocol<NIC>* Protocol<NIC>::_instance = nullptr;

#endif // PROTOCOL_H