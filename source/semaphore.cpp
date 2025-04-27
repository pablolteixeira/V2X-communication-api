#include "../header/semaphore.h"

Semaphore::Semaphore(int initial = 0) : _count(initial) {}

Semaphore::~Semaphore() {}

void Semaphore::p() { 
    std::unique_lock<std::mutex> lock(_mutex);
    
    _condition.wait(lock, [this] {
        return _count > 0;
    });
    _count--;
}

bool Semaphore::try_p() {
    std::unique_lock<std::mutex> lock(_mutex, std::try_to_lock);
    if (!lock.owns_lock()) {
        return false;
    }
    
    if (_count == 0) {
        return false;
    }
    
    _count--;
    return true;
}

void Semaphore::v() { // Signal operation
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _count++;
    }

    _condition.notify_one();
}

int Semaphore::count() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _count;
}