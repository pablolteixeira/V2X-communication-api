#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "observer.h"
#include "buffer.h"
#include "traits.h"
#include "ethernet.h"

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
        Physical_Address _from_paddr;
        Port _from_port;
        Physical_Address _to_paddr;
        Port _to_port;
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
    Protocol() {}

public:
    ~Protocol() {
        unregister_nic(_nic);
    }
    
    static Protocol* get_instance() {  
        if (!_instance) {
            _instance = new Protocol();
        }

        return _instance;
    }

    static void register_nic(NIC* nic) {
        if (_instance) {
            ConsoleLogger::print("Protocol: Registering NIC");
            //std::string mac_string = mac_to_string(nic->address());
            //ConsoleLogger::log("MAC string " + mac_string);
            //_mac_table[mac_string] = nic;
            nic->attach(_instance, PROTO);
            _nic = nic;
        }
    }

    static void unregister_nic(NIC* nic) {
        if (_instance) {
            ConsoleLogger::print("Protocol: Unregistering NIC");
            nic->detach(_instance, PROTO);
            _nic = nullptr;
        }
    }

    int send(Address from, Address to, const void * data, unsigned int size) {
        if (size > MTU) {
            return -1;
        }
        //ConsoleLogger::print("Protocol: Sending message.");
        
        if (_nic) {
            NICBuffer* buf = _nic->alloc(Address::BROADCAST_MAC, PROTO, sizeof(Header) + size);
            if (!buf) {
                return -1;
            }

            //ConsoleLogger::print("Protocol: Buffer allocated.");

            Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
            packet->Header::operator=(Header(from, to, size));

            memcpy(packet->template data<void>(), data, size);
            
            int result = _nic->send(buf);

            return result;
        }

        return -1;
    }

    int receive(NICBuffer * buf, Address from, void * data, unsigned int size) {
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        if (packet->length() > size) {
            return -1;
        }

        if (_nic) {
            Physical_Address paddr_source;
            _nic->receive(buf, &paddr_source);
            from = Address(paddr_source, packet->from_port());
            memcpy(data, packet->template data<void>(), packet->length());
            _nic->free(buf);
            return packet->length();
        }

        return -1;
    }

    static void attach(Observer * obs, Port port) {
        _observed.attach(obs, port);
    }
    static void detach(Observer * obs, Port port) {
        _observed.detach(obs, port);
    }

private:
    void update(NICBuffer * buf) override {
        //ConsoleLogger::print("Protocol: Update observers.");
        
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());

        if(!_observed.notify(packet->to_port(), buf)) {
            //ConsoleLogger::print("Protocol: Calling free buffer.");
            _nic->free(buf);
        }
    }

private:
    static Protocol* _instance;
    static NIC* _nic;

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
NIC* Protocol<NIC>::_nic = nullptr;

template <typename NIC>
Protocol<NIC>* Protocol<NIC>::_instance = nullptr;

#endif // PROTOCOL_H