#ifndef NIC_H
#define NIC_H

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <list>
#include <vector>
#include <algorithm>
#include <string>
#include <thread>

#include "observer.h"
#include "ethernet.h"
#include "console_logger.h"
#include "mac_address_generator.h"
#include "protocol.h"
#include "buffer_pool.h"
#include "semaphore.h"
#include "time_keeper.h"

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
        ConsoleLogger::print("NIC " + id + ": Starting...");
        // MAC ADDRESS + PID + COMPONENT ID
        MacAddressGenerator::generate_mac_from_seed(id, _address);
        ConsoleLogger::print("NIC " + id + ": Logical MAC created");
        ConsoleLogger::print("NIC " + id + ": MAC ADDRESS -> "+  mac_to_string(_address));

        _instance = this;

        // Register the signal handler for SIGIO
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_flags = SA_RESTART;
        sa.sa_handler = &NIC::sigio_handler;
        if (sigaction(SIGIO, &sa, NULL) < 0) {
            ConsoleLogger::error("sigaction");
            exit(EXIT_FAILURE);
        }

        // Configure socket for async I/O
        int flags = fcntl(Engine::_socket, F_GETFL, 0);
        fcntl(Engine::_socket, F_SETFL, flags | O_ASYNC | O_NONBLOCK);
        fcntl(Engine::_socket, F_SETOWN, getpid());
        
        _time_keeper = new TimeKeeper();

        _worker_thread = std::thread(&NIC::data_processing_thread, this);
    }
    ~NIC() {}

    void stop() {
        ConsoleLogger::log("NIC: Stopping");
        cleanup_nic();
    }

    NICBuffer* alloc(const Address dst, Protocol_Number prot, unsigned int size) {
        //ConsoleLogger::print("NIC: Allocating buffer. ");
        
        NICBuffer* buf = _buffer_pool.alloc();

        buf->size(size + sizeof(Ethernet::Header));
        Ethernet::Frame* frame = buf->frame();
        memcpy(frame->header()->h_dest, dst, ETH_ALEN);
        memcpy(frame->header()->h_source, Engine::_addr, ETH_ALEN);
        frame->header()->h_proto = htons(prot);

        return buf;
    }

    int send(NICBuffer* buf) {
        //ConsoleLogger::print("NIC: Sending frame.");

        Ethernet::Frame* frame = buf->frame();
        Protocol_Number prot;
        prot = ntohs(frame->header()->h_proto);

        bool is_local_broadcast = memcmp(frame->data(), frame->data() + 8, 6) == 0;

        if (is_local_broadcast) {
            notify(prot, buf);
            //ConsoleLogger::print("NIC: Frame sent BROADCAST LOCAL.");
            return 0;
        } else {
            auto now = _time_keeper->get_system_timestamp();
            frame->footer()->set_timestamp(now);

            int result = Engine::raw_send(
                frame->header()->h_dest, 
                prot, 
                frame->data(),
                buf->size() - sizeof(Ethernet::Header)
            );
            
            free(buf);

            //ConsoleLogger::print("NIC: Frame sent BROADCAST EXTERNAL.");
            return result;
        }
    }

    void free(NICBuffer* buf) {
        //ConsoleLogger::print("NIC: Free buffer");
        _buffer_pool.free(buf);
    }

    void receive(NICBuffer* buf, Address* src) {
        //ConsoleLogger::print("NIC: Receiving frame.");
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
        _instance->_data_semaphore.v();
    }

    void data_processing_thread() {
        while (_running) {
            _data_semaphore.p();

            if (!_running) {
                break;
            }
            
            process_incoming_data();
        }
    }

    void process_incoming_data() {
        while (true) {
            //ConsoleLogger::log("PROCESS INCOMING DATA");
            Address src;
            Protocol_Number prot;
            Footer footer;
            
            // Get a free buffer
            NICBuffer* buf = alloc(address(), 0, Ethernet::MTU - sizeof(Ethernet::Header) - sizeof(Ethernet::Footer));
            if (!buf) {
                //ConsoleLogger::error("No buffers available for incoming data");
                return;
            }
            
            Ethernet::Frame* frame = buf->frame();
            int size = Engine::raw_receive(&src, &prot, &footer, frame->data(), 
                                        Ethernet::MTU - sizeof(Ethernet::Header) - sizeof(Ethernet::Footer));

            if (size > 0) {
                // Successful read
                buf->size(size + sizeof(Ethernet::Header));
                auto t = _time_keeper->get_local_timestamp();
                if (!notify(prot, buf)) {
                    free(buf);
                } else {
                    _time_keeper->update_time_keeper(footer.get_timestamp() , t);
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

    void cleanup_nic() {
        _running = false;
        _data_semaphore.v();  // Wake up the processing thread
        
        if (_worker_thread.joinable()) {
            _worker_thread.join();
        }
    }

    std::string mac_to_string(Ethernet::Address& addr) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < sizeof(Ethernet::Address); ++i) {
            if (i > 0) ss << ":";
            ss << std::setw(2) << static_cast<int>(addr[i]);
        }
        
        return ss.str();
    }

private:
    //Statistics _statistics;
    BufferPool<Ethernet::Frame, BUFFER_SIZE> _buffer_pool;
    Semaphore _data_semaphore;
    bool _running;
    Address _address;
    std::thread _worker_thread;
    TimeKeeper* _time_keeper;
};

template <typename Engine>
NIC<Engine>* NIC<Engine>::_instance = nullptr;


#endif // NIC_H