#ifndef NIC_H
#define NIC_H

#include "observer.h"
#include "ethernet.h"
#include "console_logger.h"
#include "mac_address_generator.h"
#include "protocol.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include <vector>
#include <algorithm>
#include <string>

template <typename Engine>
class NIC: public Ethernet, public Conditionally_Data_Observed<Ethernet::Address,Buffer<Ethernet::Frame>,
            Ethernet::Protocol>, private Engine
{
public:
    static const unsigned int BUFFER_SIZE = Traits<NIC>::SEND_BUFFERS + Traits<NIC>::RECEIVE_BUFFERS;
    
    typedef Ethernet::Address Address;
    typedef Ethernet::Protocol Protocol_Number;
    typedef Buffer<Ethernet::Frame> NICBuffer;
    
    typedef Conditional_Data_Observer<Address,Buffer<Ethernet::Frame>, Ethernet::Protocol> Observer;
    typedef Conditionally_Data_Observed<Address,Buffer<Ethernet::Frame>, Ethernet::Protocol> Observed;

    // Global signal handler needs access to all NICs
    static std::vector<NIC*> active_nics;
public:
    NIC(const std::string& id) : _buffer_count(0) {
        ConsoleLogger::print("NIC " + id + ": Starting...");
        MacAddressGenerator::generate_mac_from_seed(id, _address);
        ConsoleLogger::print("NIC " + id + ": Logical MAC created");

        for (unsigned int i = 0; i < BUFFER_SIZE; i++) {
            _buffer[i] = new Buffer<Ethernet::Frame>(Ethernet::MTU);
        }

        // Register this NIC instance
        active_nics.push_back(this);
        
        // Set up SIGIO handling if this is the first NIC
        if (active_nics.size() == 1) {
            // Register the signal handler for SIGIO
            struct sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_flags = SA_RESTART;
            sa.sa_handler = &NIC::sigio_handler;
            if (sigaction(SIGIO, &sa, NULL) < 0) {
                ConsoleLogger::error("sigaction");
                exit(EXIT_FAILURE);
            }
            ConsoleLogger::print("NIC " + id + ": SIGIO handler set");
        }
        
        int flags = fcntl(Engine::_socket, F_GETFL, 0);
        fcntl(Engine::_socket, F_SETFL, flags | O_ASYNC | O_NONBLOCK);
        fcntl(Engine::_socket, F_SETOWN, getpid());
        
        ConsoleLogger::print("NIC " + id + ": Process ID = " + std::to_string(getpid()));
        // std::cout << "PROCESS ID: " << getpid() << std::endl;
    }
    ~NIC() {
        // Remove this NIC from the active NICs list
        auto it = std::find(active_nics.begin(), active_nics.end(), this);
        if (it != active_nics.end()) {
            active_nics.erase(it);
        }
        
        /*
        for(unsigned int i = 0; i < BUFFER_SIZE; i++) {
            delete _buffer[i];
        }*/
    }

    NICBuffer * alloc(const Address dst, Protocol_Number prot, unsigned int size) {
        //ConsoleLogger::print("NIC: Allocating buffer.");
        
        if (_buffer_count >= BUFFER_SIZE) {
            //ConsoleLogger::error("NIC: _buffer_count >= BUFFER_SIZE");
            return nullptr;
        }

        NICBuffer* buf = _buffer[_buffer_count++];
        buf->size(size + sizeof(Ethernet::Header));
        Ethernet::Frame* frame = buf->frame();
        memcpy(frame->header()->h_dest, dst, ETH_ALEN);
        memcpy(frame->header()->h_source, Engine::_addr, ETH_ALEN);
        frame->header()->h_proto = htons(prot);

        return buf;
    }

    int send(NICBuffer * buf) {
        //ConsoleLogger::print("NIC: Sending frame.");
        Ethernet::Frame* frame = buf->frame();
        int result = Engine::raw_send(
            frame->header()->h_dest, 
            ntohs(frame->header()->h_proto), 
            frame->data(),
            buf->size() - sizeof(Ethernet::Header)
        );

        //ConsoleLogger::print("NIC: Frame sent.");
        return result;
    }

    void free(NICBuffer * buf) {
        //ConsoleLogger::print("NIC: Free buffer.");
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

    void receive(NICBuffer * buf, Address * src) {
        Ethernet::Frame* frame = buf->frame();
        memcpy(src, frame->header()->h_source, ETH_ALEN);
    }

    Address& address() {
        return _address;
    }

    //void address(Address address);

    //const Statistics & statistics();

    //void attach(Observer * obs, Protocol_Number prot); // possibly inherited
    //void detach(Observer * obs, Protocol_Number prot); // possibly inherited

    using Observed::attach;
    using Observed::detach;

private:
    // SIGIO handler (static function shared by all NICs)
    static void sigio_handler(int signum) {
        std::cout << "SIGIO RECEIVED: " << signum << std::endl;
        // Check all active NICs for data
        for (auto nic : active_nics) {
            nic->process_incoming_data();
        }
    }

    // Process incoming data when SIGIO is received
    void process_incoming_data() {
        // Keep reading while there's data available (non-blocking)
        while (true) {
            Address src;
            Protocol_Number prot;

            // Get a free buffer
            NICBuffer* buf = alloc(address(), 0, Ethernet::MTU - sizeof(Ethernet::Header));
            if (!buf) {
                // No buffers available, we'll have to try again later when buffer is freed
                break;
            }
            
            Ethernet::Frame* frame = buf->frame();
            int size = Engine::raw_receive(&src, &prot, frame->data(), Ethernet::MTU - sizeof(Ethernet::Header));
            
            if (size > 0) {
                // Successful read
                buf->size(size + sizeof(Ethernet::Header));
                std::cout << "FRAME PROTO RECEIVED: " << prot << std::endl;
                if (!notify(_address, prot, buf)) {
                    free(buf);
                }
            } else if (size == 0 || (size < 0 && errno == EAGAIN)) {
                // No more data available
                free(buf);
                break;
            } else {
                // Error
                free(buf);
                perror("Error reading from socket");
                break;
            }
        }
    }

private:
    //Statistics _statistics;
    NICBuffer* _buffer[BUFFER_SIZE];
    unsigned int _buffer_count;
    Address _address;
};

template <typename Engine>
std::vector<NIC<Engine>*> NIC<Engine>::active_nics;

#endif // NIC_H