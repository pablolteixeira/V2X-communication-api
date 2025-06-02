// tests/semaphore_test.cpp
#include <iostream>
#include <cassert>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include "../header/semaphore.h"

// Função auxiliar para imprimir resultados de teste
void test_result(const std::string& test_name, bool result) {
    std::cout << test_name << ": " << (result ? "PASSOU" : "FALHOU") << std::endl;
}

// Teste básico para verificar a inicialização do semáforo
bool test_semaphore_init() {
    Semaphore sem(3);
    assert(sem.count() == 3);
    
    Semaphore sem_zero(0);
    assert(sem_zero.count() == 0);
    
    return true;
}

// Teste da operação P (decremento/wait)
bool test_semaphore_p() {
    Semaphore sem(2);
    assert(sem.count() == 2);
    
    sem.p();
    assert(sem.count() == 1);
    
    sem.p();
    assert(sem.count() == 0);
    
    return true;
}

// Teste da operação V (incremento/signal)
bool test_semaphore_v() {
    Semaphore sem(0);
    assert(sem.count() == 0);
    
    sem.v();
    assert(sem.count() == 1);
    
    sem.v();
    assert(sem.count() == 2);
    
    return true;
}

// Teste da operação try_p
bool test_semaphore_try_p() {
    Semaphore sem(1);
    
    bool result = sem.try_p();
    assert(result == true);
    assert(sem.count() == 0);
    
    result = sem.try_p();
    assert(result == false);
    assert(sem.count() == 0);
    
    return true;
}

// Teste para verificar o bloqueio e desbloqueio de threads
bool test_semaphore_thread_sync() {
    Semaphore sem(0);
    std::atomic<int> counter(0);
    std::atomic<bool> thread_blocked(false);
    std::atomic<bool> thread_released(false);
    
    // Thread que será bloqueada na operação P
    std::thread worker([&]() {
        thread_blocked = true;
        sem.p();  // Será bloqueado aqui até que alguém chame V
        counter++;
        thread_released = true;
    });
    
    // Aguarda que a thread seja bloqueada na operação P
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(thread_blocked == true);
    assert(thread_released == false);
    assert(counter == 0);
    
    // Libera a thread com a operação V
    sem.v();
    
    // Aguarda que a thread seja desbloqueada
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    assert(thread_released == true);
    assert(counter == 1);
    
    if (worker.joinable()) {
        worker.join();
    }
    
    return true;
}

// Teste para verificar múltiplas threads competindo pelo semáforo
bool test_semaphore_multiple_threads() {
    const int NUM_THREADS = 5;
    const int ITERATIONS = 1000;
    Semaphore sem(1);  // Semáforo binário como um mutex
    std::atomic<int> counter(0);
    
    auto worker_function = [&]() {
        for (int i = 0; i < ITERATIONS; i++) {
            sem.p();
            counter++;  // Seção crítica
            sem.v();
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < NUM_THREADS; i++) {
        threads.push_back(std::thread(worker_function));
    }
    
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    // Verifica se o contador está correto (sem condições de corrida)
    assert(counter == NUM_THREADS * ITERATIONS);
    
    return true;
}

int main() {
    int failures = 0;
    
    std::cout << "Iniciando testes para a classe Semaphore..." << std::endl;
    
    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 1: Inicialização" << std::endl;
    if (test_semaphore_init()) {
        std::cout << "Teste 1: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 1: FALHOU" << std::endl;
        failures ++;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 2: Operação P" << std::endl;
    if (test_semaphore_p()) {
        std::cout << "Teste 2: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 2: FALHOU" << std::endl;
        failures ++;
    }
    
    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 3: Operação V" << std::endl;
    if (test_semaphore_v()) {
        std::cout << "Teste 3: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 3: FALHOU" << std::endl;
        failures ++;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 4: Operação try_p" << std::endl;
    if (test_semaphore_try_p()) {
        std::cout << "Teste 4: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 4: FALHOU" << std::endl;
        failures ++;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 5: Sincronização de Threads" << std::endl;
    if (test_semaphore_thread_sync()) {
        std::cout << "Teste 5: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 5: FALHOU" << std::endl;
        failures ++;
    }

    std::cout << "----------------------------------------" << std::endl;

    std::cout << "Teste 6: Múltiplas de Threads" << std::endl;
    if (test_semaphore_multiple_threads()) {
        std::cout << "Teste 6: PASSOU" << std::endl;
    } else {
        std::cout << "Teste 6: FALHOU" << std::endl;
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