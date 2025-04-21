#ifndef RING_POOL_H
#define RING_POOL_H

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
            _in_use[i] = false;
        }
    }

    ~BufferPool() {
        for (size_t i = 0; i < SIZE; i++) {
            delete _buffers[i];
        }
    }

    BufferType* alloc() {
        _free_buffers.p();
        
        std::lock_guard<std::mutex> lock(_mutex);
        
        for (size_t i = 0; i < SIZE; i++) {
            if (!_in_use[i]) {
                _in_use[i] = true;
                return _buffers[i];
            }
        }
        
        return nullptr; // Should never reach here
    }

    void free(BufferType* buf) {
        bool was_in_use = false;
        
        {
            std::lock_guard<std::mutex> lock(_mutex);
            
            for (size_t i = 0; i < SIZE; i++) {
                if (_buffers[i] == buf) {
                    was_in_use = _in_use[i];
                    _in_use[i] = false;
                    break;
                }
            }
        }
        
        // If the buffer was in use, increment the semaphore
        if (was_in_use) {
            _free_buffers.v();
        }
    }

private:
    BufferType* _buffers[SIZE];
    bool _in_use[SIZE];
    
    Semaphore _free_buffers;
    std::mutex _mutex;
};

#endif // RING_POOL_H