#ifndef PROTOCOL_H
#define PROTOCOL_H

template <typename NIC>
class Protocol: private typename NIC::Observer
{
public:
    static const typename NIC::Protocol_Number PROTO = Traits<Protocol>::ETHERNET_PROTOCOL_NUMBER;
    
    typedef typename NIC::Buffer Buffer;
    typedef typename NIC::Address Physical_Address;
    typedef unsigned short Port;
    typedef Conditional_Data_Observer<Buffer<Ethernet::Frame>, Port> Observer;
    typedef Conditionally_Data_Observed<Buffer<Ethernet::Frame>, Port> Observed;

    class Address
    {
    public:
        enum Null;

    public:
        Address() : _port(0) {
            memset(_paddr, 0, sizeof(_paddr));
        };  
        Address(const Null & null) : _port(0) {
            memset(_paddr, 0, sizeof(_paddr));
        };
        Address(Physical_Address paddr, Port port) : _paddr(paddr), _port(port);
        
        operator bool() const { return (_paddr || _port); }
        
        bool operator==(Address a) { return (_paddr == a._paddr) && (_port == a._port); }
        
        Port port() const { return _port; }
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
    
    static const unsigned int MTU = NIC::MTU - sizeof(Header);
    typedef unsigned char Data[MTU];

    class Packet: public Header
    {
    public:
        Packet();
        
        Header * header();
        
        template<typename T>
        T * data() { 
            return reinterpret_cast<T *>(&_data); 
        }

    private:
        Data _data;
    } __attribute__((packed));

protected:
    Protocol(NIC * nic): _nic(nic) { _nic->attach(this, PROTO); }

public:
    ~Protocol() { _nic->detach(this, PROTO); }
    
    static int send(Address from, Address to, const void * data, unsigned int size) {
        if (size > MTU) {
            return -1;
        }

        Buffer* buf = _nic->alloc(to._paddr(), PROTO, sizeof(Header) + size);
        if (!buf) {
            return -1;
        }

        Packet* packet = reinterpret_cast::<Packet*>(buf->frame()->data());
        packet->Header::operator=(Header(from.port(), to.port(), size));
        memcpy(packet->data<void>(), data, size);

        int result = _nic->send(buf);
        _nic->free(buf);

        return result;
    }

    static int receive(Buffer * buf, Address from, void * data, unsigned int size) {
        Packet* packet = reinterpret_cast::<Packet*>(buf->frame()->data());

        if (packet->length() > size) {
            return -1;
        }

        Physical_Address paddr;
        _nic->receive(buf, &paddr, nullptr, nullptr, 0);

        *from = Address(paddr, packet->from_port());
        memcpy(data, packet->data<void>(), packet->length());

        return packet->length();
    }
    
    static void free(Buffer* buf) {
        _nic->free(buf);
    }

    static void attach(Observer * obs, Address address) {
        _observed.attach(obs, address.port());
    }
    static void detach(Observer * obs, Address address) {
        _observed.detach(obs, address.port());
    }

private:
    void update(typename NIC::Observed * obs, NIC::Protocol_Number prot, Buffer * buf) {
        Packet* packet = reinterpret_cast<Packet*>(buf->frame()->data());
        if(!_observed.notify(packet->to_port(), buf)) // to call receive(...);
            _nic->free(buf);
    }

private:
    NIC * _nic;
    // Channel protocols are usually singletons
    static Observed _observed;
};

#endif PROTOCOL_H