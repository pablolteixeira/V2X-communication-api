#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#include "semaphore.h"
#include "buffer.h"
#include <cstddef>

template <typename T, size_t SIZE>
class BufferPool
{
public:
    typedef Buffer<T> BufferType;

    BufferPool(size_t buffer_size) : 
        _free_buffers(SIZE) {
        for (size_t i = 0; i < SIZE; i++) {
            _buffers[i] = new BufferType(buffer_size);
            _buffers[i]->set_reference_counter(0);
            _in_use[i] = false;
        }
    }

    ~BufferPool() {
        for (size_t i = 0; i < SIZE; i++) {
            delete _buffers[i];
        }
    }

    BufferType* alloc() {
        ConsoleLogger::log("Buffer Pool: Requesting Buffer - Semaphore P -> " + std::to_string(_free_buffers.count()));
        _free_buffers.p();
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
        
            for (size_t i = 0; i < SIZE; i++) {
                if (!_in_use[i]) {
                    _in_use[i] = true;
                    _buffers[i]->set_reference_counter(1);
                    return _buffers[i];
                }
            }
        }
        
        return nullptr; // Should never reach here
    }
    
    void free(BufferType* buf) {
        ConsoleLogger::log("Buffer Pool: Free Buffer");
        bool was_in_use = false;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            
            for (size_t i = 0; i < SIZE; i++) {
                if (_buffers[i] == buf) {
                    int reference_counter = _buffers[i]->decrease_reference_counter();
                    
                    if (reference_counter <= 0) {
                        was_in_use = _in_use[i];
                        _in_use[i] = false;
                    }
                    break;
                }
            }
        }
        //ConsoleLogger::log("Buffer Pool: Freed Buffer");
        // If the buffer was in use, increment the semaphore
        if (was_in_use) {
            ConsoleLogger::log("Buffer Pool: Buffer was in use - Semaphore V -> " + std::to_string(_free_buffers.count()));
            _free_buffers.v();
        }
    }

private:
    BufferType* _buffers[SIZE];
    bool _in_use[SIZE];
    
    Semaphore _free_buffers;
    std::mutex _mutex;
};

#endif // BUFFER_POOL_H