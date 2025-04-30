#ifndef QUEUE_H
#define QUEUE_H

#include <mutex>

template <typename T, size_t SIZE>
class Queue {
public:
    Queue() : head(0), tail(0), count(0) {}
    
    bool add(T* value) {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (count == SIZE) {
            return false;
        }
        
        data[tail] = value;
        tail = (tail + 1) % SIZE;
        count++;
        return true;
    }
    
    T* remove() {
        std::lock_guard<std::mutex> lock(mutex);
        
        if (count == 0) {
            return nullptr;
        }
        
        T* value = data[head];
        data[head] = nullptr;
        head = (head + 1) % SIZE;
        count--;
        return value;
    }
private:
    T* data[SIZE];
    size_t head;
    size_t tail;
    size_t count;
    std::mutex mutex;
};

#endif // QUEUE_H