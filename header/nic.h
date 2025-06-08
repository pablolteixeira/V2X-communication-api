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
#include <random>
#include <semaphore.h>

#include "observer.h"
#include "ethernet.h"
#include "console_logger.h"
#include "mac_address_generator.h"
#include "mac_handler.h"
#include "protocol.h"
#include "buffer_pool.h"
#include "semaphore.h"
#include "time_keeper.h"
#include "vehicle_table.h"
#include "lru-cache/lru-cache.h"

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
    NIC(const std::string& id, const unsigned short quadrant) : _buffer_pool(Ethernet::MTU), _running(true), _quadrant(quadrant), _send_mac_key(false) {
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

        sem_init(&_sem, 0, 0);

        // Configure socket for async I/O
        int flags = fcntl(Engine::_socket, F_GETFL, 0);
        fcntl(Engine::_socket, F_SETFL, flags | O_ASYNC | O_NONBLOCK);
        fcntl(Engine::_socket, F_SETOWN, getpid());
        
        _time_keeper = new TimeKeeper();
        _mac_handler = new MACHandler();
        _mac_key_cache = new LRU_Cache<unsigned short, Ethernet::MAC_KEY>(3);

        _worker_thread = std::thread(&NIC::data_processing_thread, this);
    }
    ~NIC() {
        _instance->stop();
    }

    void stop() {
        ConsoleLogger::log("NIC: Stopping");
        cleanup_nic();
    }
    
    void create_mac_key_data(std::vector<Ethernet::MAC_KEY> mac_keys){
        _mac_handler->set_mac_key(&mac_keys[_quadrant-1]);
        _mac_handler->print_mac_key(); 
        
        unsigned short prev_quad = _quadrant - 1 == 0 ? mac_keys.size() : _quadrant - 1;
        unsigned short next_quad = (_quadrant % mac_keys.size()) + 1;
        
        int length  = sizeof(unsigned short) + Ethernet::MAC_BYTE_SIZE;

        memcpy(_mac_key_data, &prev_quad, sizeof(unsigned short));
        memcpy(_mac_key_data + sizeof(unsigned short), &mac_keys[prev_quad-1], Ethernet::MAC_BYTE_SIZE);

        memcpy(_mac_key_data + length, &next_quad, sizeof(unsigned short));
        memcpy(_mac_key_data + (length + sizeof(unsigned short)), &mac_keys[next_quad-1], Ethernet::MAC_BYTE_SIZE);
        
        memcpy(_mac_key_data + (2 * length), &_quadrant, sizeof(unsigned short));
        memcpy(_mac_key_data + (2 * length) + sizeof(unsigned short), &mac_keys[_quadrant-1], Ethernet::MAC_BYTE_SIZE);
    }

    NICBuffer* alloc(const Address dst, Protocol_Number prot, unsigned int size) {
        //ConsoleLogger::print("NIC: Allocating buffer. ");
        
        NICBuffer* buf = _buffer_pool.alloc();

        buf->size(size + sizeof(Ethernet::Header) + sizeof(Ethernet::Metadata));
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
            _time_keeper->update_sync_status();

            auto sync_state = _time_keeper->get_sync_state();
            frame->metadata()->set_sync_state(sync_state);

            auto packet_origin = _time_keeper->get_packet_origin();
            frame->metadata()->set_packet_origin(packet_origin);
            frame->metadata()->set_has_mac_keys(false);

            size_t payload_size = buf->size() - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata);

            if (packet_origin == Ethernet::Metadata::PacketOrigin::OTHERS) {
                auto mac = _mac_handler->generate_mac(frame->data(), payload_size);
                frame->metadata()->set_mac(mac);
            }

            frame->metadata()->set_quadrant(_quadrant);
            
            auto now = _time_keeper->get_system_timestamp();
            frame->metadata()->set_timestamp(now);

            if(_send_mac_key) {
                int length  = sizeof(unsigned short) + Ethernet::MAC_BYTE_SIZE;
                memcpy(frame->data(), _mac_key_data, 3*length);
                frame->metadata()->set_has_mac_keys(true);
                ConsoleLogger::log("SENDING MAC KEY");
            }
            
            int result = Engine::raw_send(
                frame->header()->h_dest, 
                prot,
                frame->metadata(),
                frame->data(),
                buf->size() - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata)
            );

            _send_mac_key = false;
            
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

    void set_packet_origin(Ethernet::Metadata::PacketOrigin packet_origin) {
        _time_keeper->update_packet_origin(packet_origin);
    }

    unsigned short get_quadrant() {
        return _quadrant;
    }

    void set_quadrant(unsigned int new_quadrant) {
        _quadrant = new_quadrant;
        ConsoleLogger::log("NIC quadrant set to: " + std::to_string(_quadrant));
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
        sem_post(&_instance->_sem);
    }

    void data_processing_thread() {
        while (_running) {
            sem_wait(&_sem);

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
            Metadata metadata;
            
            // Get a free buffer
            NICBuffer* buf = alloc(address(), 0, Ethernet::MTU - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata));
            if (!buf) {
                //ConsoleLogger::error("No buffers available for incoming data");
                return;
            }
            
            Ethernet::Frame* frame = buf->frame();
            int size = Engine::raw_receive(&src, &prot, &metadata, frame->data(), 
                                        Ethernet::MTU - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata));
                
            if (size > 0) {
                // Successful read
                auto t = _time_keeper->get_local_timestamp();
                buf->size(size);

                auto sender_quadrant = metadata.get_quadrant();

                // VERIFY IF THE RSU RECEIVED THE MESSAGE 
                if(_time_keeper->get_packet_origin() == Ethernet::Metadata::PacketOrigin::RSU) {
                    if(_quadrant == sender_quadrant) {
                        ConsoleLogger::log("RSU: Message with quadrant " + std::to_string(sender_quadrant) + " is in my quadrant " + std::to_string(_quadrant));
                        Ethernet::Address sender_address;
                        memcpy(&sender_address, frame->data(), 6);
                        if(!_vehicle_table.check_vehicle(&sender_address)) {
                            ConsoleLogger::log("RSU: New vehicle found with address: " + mac_to_string(sender_address));
                            std::array<unsigned char, ETH_ALEN> sender_address_array;
                            memcpy(sender_address_array.data(), &sender_address, ETH_ALEN);

                            _vehicle_table.set_vehicle(sender_address_array);
                            _send_mac_key = true;
                        }
                    }

                    free(buf);
                // VERIFY IF THE VEHICLE RECEIVED THE MESSAGE
                } else {
                    if (metadata.get_packet_origin() == Ethernet::Metadata::PacketOrigin::RSU && sender_quadrant == _quadrant) {
                        ConsoleLogger::log("Received RSU message");
                        auto system_timestamp = metadata.get_timestamp();
                        _time_keeper->update_time_keeper(system_timestamp, t);    
                        
                        if (metadata.get_has_mac_keys()) {
                            ConsoleLogger::log("Received RSU message has MAC keys");
                            // FRAME -> FRAME HEADER + METADATA + (DATA) -> [(id1+CHAVE1) + (id2+CHAVE2) + (id3+CHAVE3)]
                            int length = sizeof(unsigned short) + Ethernet::MAC_BYTE_SIZE;
                            for(int i = 0; i < 3; i++) {
                                unsigned short quadrant;
                                Ethernet::MAC_KEY key;
                                memcpy(&quadrant, frame->data()+(i*length), sizeof(unsigned short));
                                memcpy(key.data(), frame->data()+(i*length)+2, Ethernet::MAC_BYTE_SIZE);
                                _mac_key_cache->put(sender_quadrant, key);
                            }
                        }
                        free(buf);
                    } else if (metadata.get_packet_origin() == Ethernet::Metadata::PacketOrigin::OTHERS) {
                        Ethernet::MAC_KEY* mac_key = _mac_key_cache->get(sender_quadrant);
                        ConsoleLogger::log("Vehicle received message from other vehicle");
                        if(mac_key != nullptr) {
                            size_t payload_size = buf->size() - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata);
                            if(_mac_handler->verify_mac(frame->data(), payload_size, metadata.get_mac())) {
                                ConsoleLogger::log("Received Vehicle message with known MAC -> from = " + std::to_string(sender_quadrant) + "; to = " + std::to_string(_quadrant));
                                if (!notify(prot, buf)) {
                                    free(buf);
                                }
                            } else {
                                ConsoleLogger::log("Received Vehicle message but with unknown MAC -> from = " + std::to_string(sender_quadrant) + "; to = " + std::to_string(_quadrant));
                                free(buf);
                            }
                        } else {
                            free(buf);
                        }
                    } else {
                        free(buf);
                    }
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

            /*  
                SE A MENSAGEM CHEGOU DE UM CARRO NA RSU, E O CARRO AINDA NÃO ESTÁ NA TABELA DA RSU, A RSU VAI MANDAR UMA MENSAGEM
                RAW COM AS CHAVES DENTRO DO DATA DO FRAME
                
                if RSU {
                    if VEICULO TA NO MEU QUADRANTE {
                        if NOT VEICULO TA NA TABELA DE VEICULOS {
                            ADICIONAR NA TABELA DE VEICULOS
                            ENVIAR MAC KEY (SETA FLAG)
                        }
                    }
                    FREE
                }
                
                
                if VEICULO {
                    if VEM DE RSU DO MEU QUADRANTE {
                        UPDATE TIME KEEPER
                        if MENSAGEM TEM CHAVE {
                            ATUALIZA CHAVES
                    } ELSE {
                        if CHECAR SE ELE TEM A CHAVE DAQUELE QUADRANTE {
                            if VERIFICAR MAC{
                                if NOT NOTIFY {
                                    FREE
                                }
                            } ELSE {
                                FREE
                            }
                        } ELSE {
                            FREE
                        }  
                    } else {
                        FREE
                    }
                }
            */
        }
    }

    void cleanup_nic() {
        _running = false;
        sem_wait(&_sem);
        //_data_semaphore.v();  // Wake up the processing thread
        
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
    BufferPool<Ethernet::Frame, BUFFER_SIZE> _buffer_pool;
    sem_t _sem;
    bool _running;
    Address _address;
    std::thread _worker_thread;
    TimeKeeper* _time_keeper;
    MACHandler* _mac_handler;
    
    VehicleTable _vehicle_table;
    unsigned int _quadrant;
    bool _send_mac_key;
    LRU_Cache<unsigned short, Ethernet::MAC_KEY>* _mac_key_cache;
    unsigned char* _mac_key_data[3 * (Ethernet::MAC_BYTE_SIZE + sizeof(unsigned short))];
};


template <typename Engine>
NIC<Engine>* NIC<Engine>::_instance = nullptr;


#endif // NIC_H