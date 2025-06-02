#include "../header/mac_handler.h"
#include <cassert>
#include <iostream>
#include <string>
#include <vector>

bool test_mac_handler() {
    const unsigned char key_char[] = "supersecretkey";
    std::vector<unsigned char> key_vector;
    key_vector.assign(key_char, key_char + 14);
    
    const char* message_string =
        "Cappu-cappu-cappuccino Assassino! Assassini cappuccini! "
        "Questo killer furtivo si infiltra tra i nemici approffitando de la notte. "
        "Attento, oddiatore di caffè! Se non bevi una tazza di cappuccino al mattino, "
        "è meglio non incrociare questo tizio.";

    const unsigned char* message_char = reinterpret_cast<const unsigned char*>(message_string);
    size_t message_length = strlen(message_string);

    MACHandler mac_handler = MACHandler();
    mac_handler.set_mac_key(key_vector);

    uint32_t mac_generated = mac_handler.generate_mac(message_char, message_length);

    return mac_handler.verify_mac(message_char, message_length, mac_generated);
}

int main() {
    int failures = 0;

    std::cout << "Iniciando testes para MACHandler..." << std::endl;
    
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 1: Comparação de MACs" << std::endl;
    if (test_mac_handler()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures ++;
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