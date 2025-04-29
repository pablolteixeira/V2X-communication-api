#include <iostream>
#include <cassert>
#include <cstring>
#include "../header/buffer.h"

struct TestData {
    int id;
    double value;
    char name[64];
    
    TestData() : id(0), value(0.0) {
        std::memset(name, 0, sizeof(name));
    }
    
    TestData(int i, double v, const char* n) : id(i), value(v) {
        std::strncpy(name, n, sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0';
    }
    
    bool operator==(const TestData& other) const {
        return id == other.id && 
               value == other.value && 
               std::strcmp(name, other.name) == 0;
    }
};

bool test_buffer_creation() {
    try {
        Buffer<TestData> buffer(sizeof(TestData));
        
        if (buffer.size() != 0) {
            std::cerr << "Tamanho inicial incorreto. Esperado: 0, Atual: " << buffer.size() << std::endl;
            return false;
        }
        
        if (buffer.frame() == nullptr) {
            std::cerr << "O método frame() retornou nullptr" << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de criação: " << e.what() << std::endl;
        return false;
    }
}

bool test_buffer_data_manipulation() {
    try {
        Buffer<TestData> buffer(sizeof(TestData));
        
        TestData testData(42, 3.14, "Test Buffer");
        
        std::memcpy(buffer.frame(), &testData, sizeof(TestData));
        buffer.size(sizeof(TestData));
        
        if (buffer.size() != sizeof(TestData)) {
            std::cerr << "Tamanho incorreto após definir dados. Esperado: " << sizeof(TestData) 
                      << ", Atual: " << buffer.size() << std::endl;
            return false;
        }
        
        TestData* readData = buffer.frame();
        
        if (!(readData->id == testData.id && 
              readData->value == testData.value && 
              std::strcmp(readData->name, testData.name) == 0)) {
            std::cerr << "Dados lidos não correspondem aos dados escritos." << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de manipulação de dados: " << e.what() << std::endl;
        return false;
    }
}

bool test_buffer_size_limits() {
    try {
        const size_t maxSize = 100;
        
        Buffer<char> buffer(maxSize);
        
        buffer.size(maxSize * 2);
        
        if (buffer.size() != maxSize) {
            std::cerr << "Limite de tamanho não respeitado. Esperado: " << maxSize 
                      << ", Atual: " << buffer.size() << std::endl;
            return false;
        }
        
        const size_t smallerSize = maxSize / 2;
        buffer.size(smallerSize);
        
        if (buffer.size() != smallerSize) {
            std::cerr << "Tamanho não foi atualizado corretamente. Esperado: " << smallerSize 
                      << ", Atual: " << buffer.size() << std::endl;
            return false;
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de limites de tamanho: " << e.what() << std::endl;
        return false;
    }
}

bool test_buffer_different_types() {
    try {
        {
            Buffer<int> buffer(sizeof(int));
            int testValue = 12345;
            
            std::memcpy(buffer.frame(), &testValue, sizeof(int));
            buffer.size(sizeof(int));
            
            if (*buffer.frame() != testValue) {
                std::cerr << "Dados int não correspondentes. Esperado: " << testValue 
                          << ", Atual: " << *buffer.frame() << std::endl;
                return false;
            }
        }
        
        {
            Buffer<double> buffer(sizeof(double));
            double testValue = 3.14159;
            
            std::memcpy(buffer.frame(), &testValue, sizeof(double));
            buffer.size(sizeof(double));
            
            if (*buffer.frame() != testValue) {
                std::cerr << "Dados double não correspondentes. Esperado: " << testValue 
                          << ", Atual: " << *buffer.frame() << std::endl;
                return false;
            }
        }
        {
            const char testString[] = "Teste de Buffer com char array";
            Buffer<char> buffer(sizeof(testString));
            
            std::memcpy(buffer.frame(), testString, sizeof(testString));
            buffer.size(sizeof(testString));
            
            if (std::strcmp(buffer.frame(), testString) != 0) {
                std::cerr << "Dados string não correspondentes. Esperado: " << testString 
                          << ", Atual: " << buffer.frame() << std::endl;
                return false;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de diferentes tipos: " << e.what() << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Iniciando testes para Buffer..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    int failures = 0;
    
    std::cout << "Teste 1: Criação e destruição do Buffer" << std::endl;
    if (test_buffer_creation()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 2: Manipulação de dados no Buffer" << std::endl;
    if (test_buffer_data_manipulation()) {
        std::cout << "Teste 2: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 2: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 3: Limites de tamanho do Buffer" << std::endl;
    if (test_buffer_size_limits()) {
        std::cout << "Teste 3: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 3: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 4: Diferentes tipos de dados no Buffer" << std::endl;
    if (test_buffer_different_types()) {
        std::cout << "Teste 4: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 4: FALHOU" << std::endl;
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