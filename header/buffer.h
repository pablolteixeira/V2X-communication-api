#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>

template<typename T>
class Buffer {
public:
    Buffer(size_t max_size) : _max_size(max_size), _size(0) {
        _data = new unsigned char[max_size];
    }
    
    ~Buffer() {
        delete[] _data;
    }
    
    T* frame() {
        return reinterpret_cast<T*>(_data);
    }
    
    size_t size() const {
        return _size;
    }
    
    void size(size_t s) {
        // Just update logical size, no reallocation
        _size = (s <= _max_size) ? s : _max_size;
    }
    
private:
    unsigned char* _data;
    size_t _max_size;
    size_t _size;
};

#endif // BUFFER_H