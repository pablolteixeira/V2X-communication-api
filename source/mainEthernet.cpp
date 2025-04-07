#include <iostream>
#include <iomanip>
#include "Ethernet.h" // Certifique-se de que este arquivo está no caminho correto

int main() {
    std::cout << "=== Teste da classe Ethernet ===" << std::endl;
    
    // Definir alguns endereços MAC para teste
    Ethernet::Address mac1 = {0x00, 0x1A, 0x2B, 0x3C, 0x4D, 0x5E};
    Ethernet::Address mac2 = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};
    Ethernet::Address broadcast = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    // Testar o método address_to_string
    char mac_str[18]; // Tamanho para "xx:xx:xx:xx:xx:xx" + '\0'
    Ethernet::address_to_string(mac1, mac_str);
    std::cout << "Endereço MAC 1: " << mac_str << std::endl;
    
    Ethernet::address_to_string(mac2, mac_str);
    std::cout << "Endereço MAC 2: " << mac_str << std::endl;
    
    Ethernet::address_to_string(broadcast, mac_str);
    std::cout << "Endereço Broadcast: " << mac_str << std::endl;
    
    // Testar verificação de broadcast
    std::cout << "MAC 1 é broadcast? " << (Ethernet::is_broadcast(mac1) ? "Sim" : "Não") << std::endl;
    std::cout << "MAC 2 é broadcast? " << (Ethernet::is_broadcast(mac2) ? "Sim" : "Não") << std::endl;
    std::cout << "Broadcast é broadcast? " << (Ethernet::is_broadcast(broadcast) ? "Sim" : "Não") << std::endl;
    
    // Testar criação de frame
    Ethernet::Protocol protocol = 0x0800; // IPv4
    Ethernet::Frame frame(mac2, mac1, protocol);
    
    // Verificar conteúdo do cabeçalho
    Ethernet::Header* header = frame.header();
    std::cout << "\nConteúdo do cabeçalho do frame:" << std::endl;
    
    std::cout << "Endereço de destino: ";
    for (int i = 0; i < ETH_ALEN; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(header->h_dest[i]);
        if (i < ETH_ALEN - 1) std::cout << ":";
    }
    std::cout << std::endl;
    
    std::cout << "Endereço de origem: ";
    for (int i = 0; i < ETH_ALEN; i++) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') 
                  << static_cast<int>(header->h_source[i]);
        if (i < ETH_ALEN - 1) std::cout << ":";
    }
    std::cout << std::endl;
    
    std::cout << "Protocolo: 0x" << std::hex << std::setw(4) << std::setfill('0') 
              << ntohs(header->h_proto) << std::endl;
    
    // Verificar o tamanho do frame e espaço disponível para dados
    std::cout << "\nTamanho total do frame: " << std::dec << Ethernet::MTU << " bytes" << std::endl;
    std::cout << "Tamanho do cabeçalho: " << sizeof(Ethernet::Header) << " bytes" << std::endl;
    std::cout << "Espaço disponível para dados: " 
              << (Ethernet::MTU - sizeof(Ethernet::Header)) << " bytes" << std::endl;
    
    return 0;
}