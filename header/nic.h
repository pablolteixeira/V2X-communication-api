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
    NIC(const std::string& id, const unsigned short quadrant) : _buffer_pool(Ethernet::MTU), _running(true), _quadrant(quadrant), 
                                                                _send_mac_key(false), _packet_origin(Ethernet::Metadata::PacketOrigin::OTHERS) {
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
        _mac_key_cache = new LRU_Cache<unsigned short, Ethernet::MAC_KEY>(5);

        _worker_thread = std::thread(&NIC::data_processing_thread, this);
    }
    ~NIC() {
        _instance->stop();
    }

    void stop() {
        ConsoleLogger::log("NIC: Stopping");
        cleanup_nic();
    }
    
    void create_mac_key_data(std::vector<Ethernet::MAC_KEY> mac_keys) {
        if (mac_keys.empty()) {
            std::cerr << "Error: mac_keys vector is empty. Cannot create MAC key data." << std::endl;
            return;
        }
        if (_quadrant < 1 || _quadrant > mac_keys.size()) {
            std::cerr << "Error: Invalid quadrant value " << _quadrant 
                    << ". It must be between 1 and " << mac_keys.size() << "." << std::endl;
            return;
        }

        std::stringstream geek;
        geek << std::hex;
        const auto& current_key = mac_keys[_quadrant - 1];
        for (size_t i = 0; i < current_key.size(); i++) {
            geek << static_cast<unsigned int>(current_key[i]) << (i < current_key.size() - 1 ? " " : "");
        }

        ConsoleLogger::log("MAC KEY: " + geek.str());

        _mac_handler->set_mac_key(&mac_keys[_quadrant-1]);
        //_mac_handler->print_mac_key();

        std::cout << "Creating MAC key data for quadrant " << _quadrant << std::endl;

        unsigned short prev_quad = (_quadrant == 1) ? mac_keys.size() : _quadrant - 1;
        unsigned short next_quad = (_quadrant % mac_keys.size()) + 1;
        
        unsigned char* ptr = _mac_key_data;
        const size_t key_size = Ethernet::MAC_BYTE_SIZE;
        const size_t id_size = sizeof(unsigned short);

        // quadrante anterior
        memcpy(ptr, &prev_quad, id_size);
        ptr += id_size;
        memcpy(ptr, mac_keys[prev_quad - 1].data(), key_size);
        ptr += key_size;

        // quadrante posterior
        memcpy(ptr, &next_quad, id_size);
        ptr += id_size;
        memcpy(ptr, mac_keys[next_quad - 1].data(), key_size);
        ptr += key_size;

        // atual
        memcpy(ptr, &_quadrant, id_size);
        ptr += id_size;
        memcpy(ptr, mac_keys[_quadrant - 1].data(), key_size);
        ptr += key_size;

        if (_quadrant == 4) {
            std::cout << std::hex;
            int total_length = 3 * (id_size + key_size);
            for (int i = 0; i < total_length; i++) {
                std::cout << static_cast<unsigned int>(_mac_key_data[i]) << " ";
            }
            std::cout << std::dec << std::endl;
        }
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

            frame->metadata()->set_packet_origin(_packet_origin);
            frame->metadata()->set_has_mac_keys(false);

            size_t payload_size = buf->size() - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata);

            if (_packet_origin == Ethernet::Metadata::PacketOrigin::OTHERS) {
                auto mac = _mac_handler->generate_mac(frame->data(), payload_size);
                //ConsoleLogger::log("GENERATING MESSAGE MAC: " + std::to_string(mac) + " - PAYLOAD SIZE: " + std::to_string(payload_size) +  " - HASH: " + calcularHashDJB2(frame->data(), payload_size));
                frame->metadata()->set_mac(mac);
            }

            frame->metadata()->set_quadrant(_quadrant);
            
            auto now = _time_keeper->get_system_timestamp();
            frame->metadata()->set_timestamp(now);

            if(_send_mac_key) {
                int length  = sizeof(unsigned short) + Ethernet::MAC_BYTE_SIZE;
                memcpy(frame->data(), &_unicast_addr, ETH_ALEN);
                memcpy(frame->data()+ETH_ALEN, &_mac_key_data, 3*length);
                frame->metadata()->set_has_mac_keys(true);
                ConsoleLogger::log("SENDING MAC KEY");
                
                /*std::stringstream geek;
                geek << std::hex;
                for (int i = 0; i < 3*length; i++) {
                    geek << static_cast<unsigned int>(frame->data()[i]) << " ";
                }*/

                //ConsoleLogger::log("HEADER + METADATA SIZE: " +  std::to_string(sizeof(Ethernet::Header) + sizeof(Ethernet::Metadata)));
                buf->size(sizeof(Ethernet::Header) + sizeof(Ethernet::Metadata) + 3*length);
                //ConsoleLogger::log("MAC KEYS SENT: " + geek.str());
            }
            
            int result = Engine::raw_send(
                frame->header()->h_dest, 
                prot,
                frame->metadata(),
                frame->data(),
                buf->size() - sizeof(Ethernet::Header) - sizeof(Ethernet::Metadata)
            );

            //ConsoleLogger::log("Result: " + std::to_string(result + sizeof(Ethernet::Header) + sizeof(Ethernet::Metadata)));

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
        _packet_origin = packet_origin;
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
                if(_packet_origin == Ethernet::Metadata::PacketOrigin::RSU) {
                    if(_quadrant == sender_quadrant) {
                        ConsoleLogger::log("RSU: Message with quadrant " + std::to_string(sender_quadrant) + " is in my quadrant " + std::to_string(_quadrant));
                        Address sender_address;
                        memcpy(&sender_address, frame->data(), 6);
                        if(!_vehicle_table.check_vehicle(&sender_address)) {
                            ConsoleLogger::log("RSU: New vehicle found with address: " + mac_to_string(sender_address));
                            std::array<unsigned char, ETH_ALEN> sender_address_array;
                            memcpy(sender_address_array.data(), &sender_address, ETH_ALEN);
                            _vehicle_table.set_vehicle(sender_address_array);
                            _send_mac_key = true;
                            memcpy(&_unicast_addr, &sender_address, ETH_ALEN);

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
                            Address dest;
                            memcpy(&dest, frame->data(), ETH_ALEN);
                            if (mac_to_string(dest) == mac_to_string(_address)){                                 
                                ConsoleLogger::log("Received RSU message has MAC keys");
                                // FRAME -> FRAME HEADER + METADATA + (DATA) -> [(id1+CHAVE1) + (id2+CHAVE2) + (id3+CHAVE3)]
                                int length = sizeof(unsigned short) + Ethernet::MAC_BYTE_SIZE;      
                                
                                for(int i = 0; i < 3; i++) {
                                    unsigned short quadrant;
                                Ethernet::MAC_KEY key;
                                memcpy(&quadrant, frame->data()+(i*length)+ETH_ALEN, sizeof(unsigned short));
                                memcpy(key.data(), frame->data()+(i*length)+sizeof(unsigned short)+ETH_ALEN, Ethernet::MAC_BYTE_SIZE);
                                
                                _mac_key_cache->put(quadrant, key);
                                }
                                _mac_handler->set_mac_key(_mac_key_cache->get(_quadrant));
                                for(int i = 0; i < 4; i++) {
                                    auto key_2 = _mac_key_cache->get(i+1);
                                    if(key_2) {
                                        std::stringstream geek;
                                        geek << std::hex;
                                        for (size_t i = 0; i < Ethernet::MAC_BYTE_SIZE; i++) {
                                            geek << static_cast<unsigned int>(key_2->data()[i]) << " ";
                                        }
                                        
                                        ConsoleLogger::log("MAC KEY ACCESSED [" + std::to_string(i+1) + "]: " + geek.str());
                                    }
                                }
                            }
                        }
                        free(buf);
                    } else if (metadata.get_packet_origin() == Ethernet::Metadata::PacketOrigin::OTHERS) {
                        Ethernet::MAC_KEY* mac_key = _mac_key_cache->get(sender_quadrant);
                        ConsoleLogger::log("Vehicle received message from other vehicle");
                        if(mac_key) {
                            ConsoleLogger::log("Received Vehicle message with known MAC -> from = " + std::to_string(sender_quadrant) + "; to = " + std::to_string(_quadrant));
                            size_t payload_size = size;
                            ConsoleLogger::log("RECEIVING MESSAGE MAC:" + std::to_string(metadata.get_mac()) + " | Payload size: " + std::to_string(payload_size) + " | HASH: " + calcularHashDJB2(frame->data(), size));
                            if(_mac_handler->verify_mac(frame->data(), payload_size, metadata.get_mac())) {
                                ConsoleLogger::log("MAC verification successful");
                                if (!notify(prot, buf)) {
                                    free(buf);
                                }
                            } else {

                                free(buf);
                            }
                        } else {
                            ConsoleLogger::log("Received Vehicle message but with unknown MAC -> from = " + std::to_string(sender_quadrant) + "; to = " + std::to_string(_quadrant));
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
        //_data_semaphore.v();  
        
        if (_worker_thread.joinable()) {
            _worker_thread.join();
        }
    }

    std::string mac_to_string(Address& addr) {
        std::stringstream ss;
        ss << std::hex << std::setfill('0');
        
        for (size_t i = 0; i < sizeof(Address); ++i) {
            if (i > 0) ss << ":";
            ss << std::setw(2) << static_cast<int>(addr[i]);
        }
        
        return ss.str();
    }

    std::string calcularHashDJB2(const unsigned char* dados, size_t tamanho) {
        unsigned long hash = 5381; // Valor inicial arbitrário para o DJB2

        for (size_t i = 0; i < tamanho; ++i) {
            hash = ((hash << 5) + hash) + dados[i]; // hash * 33 + dados[i]
        }

        // Converter o valor de hash numérico para uma string hexadecimal
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
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
    Ethernet::Metadata::PacketOrigin _packet_origin;
    
    VehicleTable _vehicle_table;
    unsigned int _quadrant;
    bool _send_mac_key;
    Address _unicast_addr;
    LRU_Cache<unsigned short, Ethernet::MAC_KEY>* _mac_key_cache;
    unsigned char _mac_key_data[3 * (Ethernet::MAC_BYTE_SIZE + sizeof(unsigned short))];
};


template <typename Engine>
NIC<Engine>* NIC<Engine>::_instance = nullptr;


#endif // NIC_H