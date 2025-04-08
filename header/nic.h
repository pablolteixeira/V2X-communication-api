#ifndef NIC_H
#define NIC_H

#include "ethernet.h"
#include "buffer.h"

template <typename Engine>
class NIC: public Ethernet, public Conditional_Data_Observed<Buffer<Ethernet::Frame>,
            Ethernet::Protocol>, private Engine
{
public:
    static const unsigned int BUFFER_SIZE = Traits<NIC>::SEND_BUFFERS + Traits<NIC>::RECEIVE_BUFFERS;
    
    typedef Ethernet::Address Address;
    typedef Ethernet::Protocol Protocol_Number;
    typedef Buffer<Ethernet::Frame> Buffer;
    
    typedef Conditional_Data_Observer<Buffer<Ethernet::Frame>, Ethernet::Protocol> Observer;
    typedef Conditionally_Data_Observed<Buffer<Ethernet::Frame>, Ethernet::Protocol> Observed;
protected:
    NIC() : _buffer_count(0) {
        for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
            _buffer[i] = new Buffer(Ethernet::MTU);
        }

        // Start receive thread
        pthread_create(&_recv_thread, NULL, &receive_thread_function, this);
    }
public:
    ~NIC() {
        for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
            delete _buffer[i];
        }

        pthread_cancel(_recv_thread);
        pthread_join(_recv_thread, NULL);
    }

    Buffer * alloc(Address dst, Protocol_Number prot, unsigned int size) {
        if (_buffer_count >= BUFFER_SIZE) {
            return nullptr;
        }

        Buffer* buf = _buffer[_buffer_count++];
        Ethernet::Frame* frame = buf->frame();
        memcpy(frame->header()->h_dest, dst, ETH_ALEN);
        memcpy(frame->header()->h_source, Engine::_addr, ETH_ALEN);
        frame->header()->h_proto = htons(prot);
        buf->size(size + sizeof(Ethernet::Header));

        return buf;
    }

    int send(Buffer * buf) {
        Ethernet::Frame* frame = buf->frame();
        int result = Engine::raw_send(
            frame->header()->h_dest, 
            ntohs(frame->header()->h_proto), 
            frame->data(),
            buf->size() - sizeof(Ethernet::Header)
        );

        return result;
    }

    void free(Buffer * buf) {
        for(unsigned int i = 0; i < _buffer_count; i++) {
            if(_buffer[i] == buf) {
                if(i < _buffer_count - 1) {
                    _buffer[i] = _buffer[_buffer_count - 1];
                }
                _buffer_count--;
                break;
            }
        }
    }

    int receive(Buffer * buf, Address * src, Address * dst, void * data, unsigned int size) {
        Ethernet::Frame* frame = buf->frame();
        memcpy(src, frame->header()->h_source, ETH_ALEN);
        memcpy(dst, frame->header()->h_dest, ETH_ALEN);

        unsigned int data_size = buf->size() - sizeof(Ethernet::Header);
        unsigned int copy_size = (data_size > size) ? size : data_size;

        memcpy(data, frame->data(), copy_size);

        return copy_size;
    }

    const Address & address();

    void address(Address address);

    const Statistics & statistics();

    void attach(Observer * obs, Protocol_Number prot); // possibly inherited
    void detach(Observer * obs, Protocol_Number prot); // possibly inherited

private:
    static void* receive_thread_function(void* arg) {
        NIC* nic = static_cast<NIC*>(arg);
        while(true) {
            Address src;
            Protocol_Number prot;

            Buffer* buf = nic->alloc(nic->address(), 0, Ethernet::MTU - sizeof(Ethernet::Header));
            if (!buf) {
                sleep(1);
                continue;
            }

            Ethernet::Frame* frame = buf->frame();
            int size = nic->receive(&src, &prot, frame->data(), Ethernet::MTU - sizeof(Ethernet::Header));

            if (size > 0) {
                buf->size(size + sizeof(Ethernet::Header));
                nic->notify(prot, buf);
            } else {
                nic->free(buf);
            }
        }

        return nullptr;
    }

private:
    Statistics _statistics;
    Buffer* _buffer[BUFFER_SIZE];
    unsigned int _buffer_count;
    pthread_t _recv_thread;
};

#endif NIC_H