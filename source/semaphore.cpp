#include "../header/semaphore.h"

Semaphore::Semaphore(int initial = 0) : _count(initial) {
    pthread_mutex_init(&_mutex, nullptr);
    pthread_cond_init(&_condition, nullptr);
}

Semaphore::~Semaphore() {
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_condition);
}

void Semaphore::p() { // Wait operation
    pthread_mutex_lock(&_mutex);
    while (_count == 0) {
        pthread_cond_wait(&_condition, &_mutex);
    }
    _count--;
    pthread_mutex_unlock(&_mutex);
}

void Semaphore::v() { // Signal operation
    pthread_mutex_lock(&_mutex);
    _count++;
    pthread_cond_signal(&_condition);
    pthread_mutex_unlock(&_mutex);
}