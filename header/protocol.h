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
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>


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
        Header() : _from_port(0), _to_port(0), _from_paddr(), _to_paddr(), _length(0) {}
        Header(Address from_address, Address to_address, unsigned short length)
            : _from_port(from_address.port()), _to_port(to_address.port()), _length(length) {
                memcpy(_from_paddr, from_address.paddr(), sizeof(Physical_Address));
                memcpy(_to_paddr, to_address.paddr(), sizeof(Physical_Address));
            }


        Port from_port() const { 
            return _from_port; 
        }
        Port to_port() const { 
            return _to_port;
        }

        Physical_Address& from_paddr() { 
            return _from_paddr; 
        }
        Physical_Address& to_paddr() { 
            return _to_paddr; 
        }

        unsigned short length() const { 
            return _length; 
        }
        
    private:
        Port _from_port;
        Port _to_port;
        Physical_Address _from_paddr;
        Physical_Address _to_paddr;
        unsigned short _length;
    } __attribute__((packed));
    
    static const unsigned int MTU;
    static constexpr unsigned int get_mtu() {
        return NIC::MTU - sizeof(Header);
    }
    typedef unsigned char Data[get_mtu()];

    class Packet: public Header
    {
    public:
        Packet();
                
        template<typename T>
        T * data() { 
            return reinterpret_cast<T*>(&_data); 
        }

    private:
        Data _data;
    } __attribute__((packed));

protected:
    Protocol() {
        if (!_instance_mutex) {
            _instance_mutex = sem_open("/eth_protocol_instance_mutex", O_CREAT, 0666, 1);
        }    
    }

public:
    ~Protocol() {
        for (auto nic : _nics) {
            nic->detach(this, PROTO);
        }

        sem_close(_instance_mutex);
        sem_unlink("/eth_protocol_instance_mutex");
    }
    
    static Protocol* get_instance() {  
        if (!_instance) {
            void* _shared_mem = mmap(NULL, sizeof(Protocol), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

            if (_shared_mem != MAP_FAILED) {
                _instance = new (_shared_mem) Protocol();
            }
        }

        return _instance;
    }

    static void register_nic(NIC* nic) {
        if (_instance) {
            ConsoleLogger::print("Protocol: Registering NIC");
            _nics.push_back(nic);
            std::string mac_string = mac_to_string(nic->address());
            std::cout << "mac string " << mac_string << std::endl;

            _mac_table[mac_string] = nic;
            std::cout << "nic - mac string " << nic << std::endl;
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
        //ConsoleLogger::print("Protocol: Sending message.");
        
        NIC* nic = get_nic(from.paddr());
        NICBuffer* buf = nic->alloc(to.paddr(), PROTO, sizeof(Header) + size);
        if (!buf) {
            return -1;
        }

        //ConsoleLogger::print("Protocol: Buffer allocated.");

        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        packet->Header::operator=(Header(from, to, size));

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
        
        Physical_Address paddr_source;
        nic->receive(buf, &paddr_source);
        from = Address(paddr_source, packet->from_port());
        memcpy(data, packet->template data<void>(), packet->length());

        return packet->length();
    }

    static void attach(Observer * obs, Address address) {
        _observed.attach(obs, PROTO);
    }
    static void detach(Observer * obs, Address address) {
        _observed.detach(obs, PROTO);
    }

private:
    void update(Physical_Address& paddr, typename NIC::Protocol_Number prot, NICBuffer * buf) override {
        // std::cout << "Buffer pointer before: " << buf << std::endl;
        ConsoleLogger::print("Protocol: Update observers.");
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        
        if(!_observed.notify(prot, buf)) {
            NIC* nic = get_nic(paddr);
            ConsoleLogger::print("Protocol: Calling free buffer.");
            nic->free(buf);
        }
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
    static sem_t* _instance_mutex;
    static void* _shared_mem;

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

template <typename NIC>
sem_t* Protocol<NIC>::_instance_mutex = nullptr;

template <typename NIC>
void* Protocol<NIC>::_shared_mem = nullptr;

#endif // PROTOCOL_H