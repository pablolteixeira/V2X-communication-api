#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>
#include <atomic>
#include "../header/raw_socket_engine.h"
#include "../header/ethernet.h"

class TestableRawSocketEngine : public RawSocketEngine {
public:
    TestableRawSocketEngine() : RawSocketEngine() {}
    
    using RawSocketEngine::raw_send;
    using RawSocketEngine::raw_receive;
    using RawSocketEngine::get_interface;
    
    int get_socket() const { return _socket; }
    int get_ifindex() const { return _ifindex; }
    const Ethernet::Address& get_addr() const { return _addr; }
};

// Função auxiliar para impressão de endereço MAC
std::string mac_to_string(const Ethernet::Address& addr) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < ETH_ALEN; i++) {
        ss << std::setw(2) << (static_cast<unsigned int>(addr[i]) & 0xFF);
        if (i < ETH_ALEN - 1) {
            ss << ":";
        }
    }
    return ss.str();
}

bool test_raw_socket_engine_init() {
    try {
        TestableRawSocketEngine engine;
        
        // Verifica se o socket foi criado com sucesso
        assert(engine.get_socket() >= 0);
        
        // Verifica se o índice da interface foi obtido
        assert(engine.get_ifindex() > 0);
        
        // Verifica se o endereço MAC foi obtido (não deve ser todo zeros)
        bool all_zeros = true;
        for (int i = 0; i < ETH_ALEN; i++) {
            if (engine.get_addr()[i] != 0) {
                all_zeros = false;
                break;
            }
        }
        assert(!all_zeros);
        
        std::cout << "Interface: " << engine.get_interface() << std::endl;
        std::cout << "MAC Address: " << mac_to_string(engine.get_addr()) << std::endl;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de inicialização: " << e.what() << std::endl;
        return false;
    }
}

bool test_raw_socket_send_receive() {
    try {
        TestableRawSocketEngine sender;
        TestableRawSocketEngine receiver;
        
        std::atomic<bool> packet_received(false);
        
        std::thread receive_thread([&]() {
            Ethernet::Address src;
            Ethernet::Protocol prot;
            char buffer[1024];
            
            auto start_time = std::chrono::steady_clock::now();
            while (!packet_received && 
                   std::chrono::steady_clock::now() - start_time < std::chrono::seconds(5)) {
                
                int bytes_received = receiver.raw_receive(&src, &prot, buffer, sizeof(buffer));
                
                if (bytes_received > 0 && prot == 0x8888) {  // Protocolo personalizado para teste
                    std::string received_data(buffer, bytes_received);
                    if (received_data == "TESTE_RAW_SOCKET") {
                        std::cout << "Pacote recebido com sucesso!" << std::endl;
                        packet_received = true;
                        break;
                    }
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Envia pacote de teste para o endereço de broadcast
        Ethernet::Address broadcast = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        const char* test_data = "TESTE_RAW_SOCKET";
        
        std::cout << "Enviando pacote de teste..." << std::endl;
        int bytes_sent = sender.raw_send(broadcast, 0x8888, test_data, strlen(test_data));
        
        std::cout << "Enviados " << bytes_sent << " bytes." << std::endl;
        
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        
        return packet_received;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de envio/recebimento: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Iniciando testes para RawSocketEngine..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    int failures = 0;
    
    std::cout << "Teste 1: Inicialização do RawSocketEngine" << std::endl;
    if (test_raw_socket_engine_init()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 2: Envio e recebimento de dados" << std::endl;
    if (test_raw_socket_send_receive()) {
        std::cout << "Teste 2: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 2: FALHOU (Pode ser devido à filtragem de pacotes ou falta de privilégios)" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    if (failures == 0) {
        std::cout << "TODOS OS TESTES PASSARAM!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " TESTE(S) FALHARAM!" << std::endl;
        return 1;
    }
}