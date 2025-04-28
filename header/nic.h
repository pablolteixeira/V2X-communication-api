#ifndef NIC_H
#define NIC_H

#include "observer.h"
#include "ethernet.h"
#include "console_logger.h"
#include "mac_address_generator.h"
#include "protocol.h"
#include "buffer_pool.h"
#include "semaphore.h"
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include <thread>

template <typename Engine>
class NIC: public Ethernet, public Conditionally_Data_Observed<Buffer<Ethernet::Frame>,
            Ethernet::Protocol>, private Engine
{
public:
    static const unsigned int BUFFER_SIZE = Traits<NIC>::SEND_BUFFERS + Traits<NIC>::RECEIVE_BUFFERS;
    
    typedef Ethernet::Address Address;
    typedef Ethernet::Protocol Protocol_Number;
    typedef Buffer<Ethernet::Frame> NICBuffer;
    
    typedef Conditional_Data_Observer<Buffer<Ethernet::Frame>, Ethernet::Protocol> Observer;
    typedef Conditionally_Data_Observed<Buffer<Ethernet::Frame>, Ethernet::Protocol> Observed;

    static NIC<Engine>* _instance;
public:
    NIC(const std::string& id) : _buffer_pool(Ethernet::MTU), _data_semaphore(0), _running(true) {
        //ConsoleLogger::print("NIC " + id + ": Starting...");
        // MAC ADDRESS + PID + COMPONENT ID
        MacAddressGenerator::generate_mac_from_seed(id, _address);
        //ConsoleLogger::print("NIC " + id + ": Logical MAC created");

        // Register the signal handler for SIGIO
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = &NIC::sigio_handler;
        //sa.sa_handler = &NIC::signal_handler_void;
        if (sigaction(SIGIO, &sa, NULL) < 0) {
            ConsoleLogger::error("sigaction");
            exit(EXIT_FAILURE);
        }

        int flags = fcntl(Engine::_socket, F_GETFL, 0);
        fcntl(Engine::_socket, F_SETFL, flags | O_ASYNC | O_NONBLOCK);
        fcntl(Engine::_socket, F_SETOWN, getpid());
        
        _instance = this;

        _worker_thread = std::thread(&NIC::data_processing_thread, this);
    }
    ~NIC() {}

    void stop() {
        cleanup_nic();
    }

    NICBuffer* alloc(const Address dst, Protocol_Number prot, unsigned int size) {
        ConsoleLogger::print("NIC: Allocating buffer. ");
        
        NICBuffer* buf = _buffer_pool.alloc();

        buf->size(size + sizeof(Ethernet::Header));
        Ethernet::Frame* frame = buf->frame();
        memcpy(frame->header()->h_dest, dst, ETH_ALEN);
        memcpy(frame->header()->h_source, Engine::_addr, ETH_ALEN);
        frame->header()->h_proto = htons(prot);

        return buf;
    }

    int send(NICBuffer* buf) {
        ConsoleLogger::print("NIC: Sending frame.");
        std::cout << buf << std::endl;

        Protocol_Number prot;
        Ethernet::Frame* frame = buf->frame();
        prot = ntohs(frame->header()->h_proto);
        int result = Engine::raw_send(
            frame->header()->h_dest, 
            prot, 
            frame->data(),
            buf->size() - sizeof(Ethernet::Header)
        );
        
        notify(prot, buf);

        ConsoleLogger::print("NIC: Frame sent.");
        return result;
    }

    void free(NICBuffer* buf) {
        ConsoleLogger::print("NIC: Free buffer");
        std::cout << buf << std::endl;
        _buffer_pool.free(buf);
    }

    void receive(NICBuffer* buf, Address* src) {
        ConsoleLogger::print("NIC: Receiving frame.");
        Ethernet::Frame* frame = buf->frame();
        memcpy(src, frame->header()->h_source, ETH_ALEN);
    }

    Address& address() {
        return _address;
    }

    //void address(Address address);

    //const Statistics & statistics();

    using Observed::attach;
    using Observed::detach;

private:
    static void sigio_handler(int signum) {
        if (_instance) {
            _instance->_data_semaphore.v();
        }
    }

    static void signal_handler_void(int sig) {
        ConsoleLogger::log("Received signal: " + std::to_string(sig));
        if (sig == SIGIO) {
            ConsoleLogger::error("Received SIGIO (29) - I/O notification issue");
            // Handle appropriately
        }
    }

    void data_processing_thread() {
        while (_running) {
            if (_data_semaphore.try_p()) {
                process_incoming_data();
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            if (!_running) {
                break;
            }
        }
    }

    void process_incoming_data() {
        ConsoleLogger::log("PROCESSING INCOMING DATA");
        ConsoleLogger::log(std::to_string(pthread_self()));
        // Keep reading while there's data available (non-blocking)
        Address src;
        Protocol_Number prot;
        
        // Get a free buffer
        NICBuffer* buf = alloc(address(), 0, Ethernet::MTU - sizeof(Ethernet::Header));
        if (!buf) {
            // No buffers available, we'll have to try again later
            return;
        }
        
        Ethernet::Frame* frame = buf->frame();
        int size = Engine::raw_receive(&src, &prot, frame->data(), 
                                    Ethernet::MTU - sizeof(Ethernet::Header));

        ConsoleLogger::log("SIZE RAW RECEIVE: " + std::to_string(size));

        if (size > 0) {
            // Successful read
            buf->size(size + sizeof(Ethernet::Header));
            if (!notify(prot, buf)) {
                free(buf);
            }
        } else if (size == 0 || (size < 0 && errno == EAGAIN)) {
            // No more data available
            free(buf);
        } else {
            // Error
            free(buf);
            perror("Error reading from socket");
        }

        return;
    }

    void cleanup_nic() {
        _running = false;
        
        _data_semaphore.v();
        
        if (_worker_thread.joinable()) {
            _worker_thread.join();
        }
    }

private:
    //Statistics _statistics;
    BufferPool<Ethernet::Frame, BUFFER_SIZE> _buffer_pool;
    Address _address;
    bool _running;
    std::thread _worker_thread;
    Semaphore _data_semaphore;
};

template <typename Engine>
NIC<Engine>* NIC<Engine>::_instance = nullptr;


#endif // NIC_H