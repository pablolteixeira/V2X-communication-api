#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <pthread.h>

// Semaphore implementation for thread synchronization
class Semaphore 
{
public:
    Semaphore(int initial);

    ~Semaphore();

    void p();

    void v();

private:
    int _count;
    pthread_mutex_t _mutex;
    pthread_cond_t _condition;
};

#endif