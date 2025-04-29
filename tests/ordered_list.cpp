#include <iostream>
#include <cassert>
#include <string>
#include "../header/ordered_list.h"

class TestItem {
private:
    int _value;
    
public:
    TestItem(int value) : _value(value) {}
    
    int getValue() const { return _value; }
    
    bool operator<(const TestItem& other) const {
        return _value < other._value;
    }
    
    bool operator==(const TestItem& other) const {
        return _value == other._value;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const TestItem& item) {
        os << item._value;
        return os;
    }
};

bool test_ordered_list_insert() {
    try {
        Ordered_List<TestItem> list;
        
        TestItem* item1 = new TestItem(10);
        TestItem* item2 = new TestItem(5);
        TestItem* item3 = new TestItem(15);
        
        list.insert(item1);
        list.insert(item2);
        list.insert(item3);
        
        if (list.size() != 3) {
            std::cerr << "Tamanho incorreto após inserções. Esperado: 3, Atual: " << list.size() << std::endl;
            delete item1;
            delete item2;
            delete item3;
            return false;
        }
        
        list.insert(nullptr);
        if (list.size() != 3) {
            std::cerr << "Tamanho incorreto após inserção de nullptr. Esperado: 3, Atual: " << list.size() << std::endl;
            delete item1;
            delete item2;
            delete item3;
            return false;
        }
        
        delete item1;
        delete item2;
        delete item3;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de inserção: " << e.what() << std::endl;
        return false;
    }
}

bool test_ordered_list_remove() {
    try {
        Ordered_List<TestItem> list;
        
        TestItem* item1 = new TestItem(10);
        TestItem* item2 = new TestItem(5);
        TestItem* item3 = new TestItem(15);
        
        list.insert(item1);
        list.insert(item2);
        list.insert(item3);
        
        list.remove(item2);
        
        if (list.size() != 2) {
            std::cerr << "Tamanho incorreto após remoção. Esperado: 2, Atual: " << list.size() << std::endl;
            delete item1;
            delete item2;
            delete item3;
            return false;
        }
        
        list.remove(nullptr);
        if (list.size() != 2) {
            std::cerr << "Tamanho incorreto após remoção de nullptr. Esperado: 2, Atual: " << list.size() << std::endl;
            delete item1;
            delete item2;
            delete item3;
            return false;
        }
        
        delete item1;
        delete item2;
        delete item3;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de remoção: " << e.what() << std::endl;
        return false;
    }
}

bool test_ordered_list_iterator() {
    try {
        Ordered_List<TestItem> list;
        
        int count = 0;
        for (auto item : list) {
            (void)item; 
            count++;
        }
        
        if (count != 0) {
            std::cerr << "Contagem incorreta para lista vazia. Esperado: 0, Atual: " << count << std::endl;
            return false;
        }
        
        TestItem* item1 = new TestItem(10);
        TestItem* item2 = new TestItem(5);
        TestItem* item3 = new TestItem(15);
        
        list.insert(item1);
        list.insert(item2);
        list.insert(item3);
        
        count = 0;
        for (auto item : list) {
            if (!item) {
                std::cerr << "Item nulo encontrado durante iteração" << std::endl;
                delete item1;
                delete item2;
                delete item3;
                return false;
            }
            count++;
        }
        
        if (count != 3) {
            std::cerr << "Contagem incorreta após iteração. Esperado: 3, Atual: " << count << std::endl;
            delete item1;
            delete item2;
            delete item3;
            return false;
        }
        
        delete item1;
        delete item2;
        delete item3;
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Exceção durante teste de iterador: " << e.what() << std::endl;
        return false;
    }
}

// bool test_ordered_list_order() {
//     try {
//         Ordered_List<TestItem> list;
        
//         TestItem* item1 = new TestItem(30);
//         TestItem* item2 = new TestItem(10);
//         TestItem* item3 = new TestItem(20);
        
//         list.insert(item1);
//         list.insert(item2);
//         list.insert(item3);
        
//         if (list.size() != 3) {
//             std::cerr << "Tamanho incorreto. Esperado: 3, Atual: " << list.size() << std::endl;
//             delete item1;
//             delete item2;
//             delete item3;
//             return false;
//         }
        
//         delete item1;
//         delete item2;
//         delete item3;
        
//         return true;
//     } catch (const std::exception& e) {
//         std::cerr << "Exceção durante teste de ordem: " << e.what() << std::endl;
//         return false;
//     }
// }

int main() {
    std::cout << "Iniciando testes para Ordered_List..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    
    int failures = 0;
    
    std::cout << "Teste 1: Inserção de itens" << std::endl;
    if (test_ordered_list_insert()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 2: Remoção de itens" << std::endl;
    if (test_ordered_list_remove()) {
        std::cout << "Teste 2: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 2: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    std::cout << "Teste 3: Iteração sobre itens" << std::endl;
    if (test_ordered_list_iterator()) {
        std::cout << "Teste 3: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 3: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;
    
    // std::cout << "Teste 4: Ordem de inserção" << std::endl;
    // if (test_ordered_list_order()) {
    //     std::cout << "Teste 4: PASSOU" << std::endl;
    // } else {
    //     std::cout << "Teste 4: FALHOU" << std::endl;
    //     failures++;
    // }
    // std::cout << "----------------------------------------" << std::endl;
    
    if (failures == 0) {
        std::cout << "TODOS OS TESTES PASSARAM!" << std::endl;
        return 0;
    } else {
        std::cout << failures << " TESTE(S) FALHARAM!" << std::endl;
        return 1;
    }
}