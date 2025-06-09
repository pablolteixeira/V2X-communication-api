#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <mutex>
#include <condition_variable>
#include <atomic>

// Semaphore implementation for thread synchronization
class Semaphore 
{
public:
    Semaphore(int initial);

    ~Semaphore();

    void p();
    bool try_p();

    void v();

    int count();
private:
    int _count;
    std::mutex _mutex;
    std::condition_variable _condition;
};

#endif // SEMAPHORE_H