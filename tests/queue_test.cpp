#include <iostream>
#include "../header/queue.hpp"  // Supondo que a classe Queue esteja definida neste arquivo

bool test_queue_add() {
    Queue<int, 3> q;

    int a = 10, b = 20, c = 30;
    return q.add(&a) && q.add(&b) && q.add(&c) && !q.add(&a); // o último deve falhar (fila cheia)
}

bool test_queue_remove() {
    Queue<int, 3> q;

    int a = 100, b = 200;
    q.add(&a);
    q.add(&b);

    int* val1 = q.remove();
    int* val2 = q.remove();
    int* val3 = q.remove(); // deve retornar nullptr

    return val1 && *val1 == 100 &&
           val2 && *val2 == 200 &&
           val3 == nullptr;
}

bool test_queue_order() {
    Queue<int, 5> q;

    int values[5] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i) {
        q.add(&values[i]);
    }

    for (int i = 0; i < 5; ++i) {
        int* v = q.remove();
        if (!v || *v != values[i]) {
            return false;
        }
    }

    return true;
}

int main() {
    std::cout << "Iniciando testes para Queue..." << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int failures = 0;

    std::cout << "Teste 1: Inserção até limite" << std::endl;
    if (test_queue_add()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 2: Remoção e underflow" << std::endl;
    if (test_queue_remove()) {
        std::cout << "Teste 2: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 2: FALHOU" << std::endl;
        failures++;
    }
    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 3: Ordem FIFO" << std::endl;
    if (test_queue_order()) {
        std::cout << "Teste 3: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 3: FALHOU" << std::endl;
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