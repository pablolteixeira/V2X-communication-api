#include "../header/semaphore.h"

// Constructor
Semaphore::Semaphore(int initial = 0) : _count(initial) {
    pthread_mutex_init(&_mutex, nullptr);
    pthread_cond_init(&_condition, nullptr);
}

// Destructor
Semaphore::~Semaphore() {
    pthread_mutex_destroy(&_mutex);
    pthread_cond_destroy(&_condition);
}

// Wait operation
void Semaphore::p() {
    pthread_mutex_lock(&_mutex);
    while (_count == 0) {
        pthread_cond_wait(&_condition, &_mutex);
    }
    _count--;
    pthread_mutex_unlock(&_mutex);
}

// Signal operation
void Semaphore::v() {
    pthread_mutex_lock(&_mutex);
    _count++;
    pthread_cond_signal(&_condition);
    pthread_mutex_unlock(&_mutex);
}