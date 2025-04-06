#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>

template<typename T>
class Buffer 
{
public:
    // Constructor
    Buffer() : _data(nullptr), _size(0) {}

    // Constructor with a specified size
    Buffer(size_t size) : _size(size) {
        _data = new unsigned char[size];
    }

    // Destructor
    ~Buffer() {
        if (_data) delete[] _data;
    }

    // Template method to get data as a specific type
    T* frame() {
        return reinterpret_cast<T*>(_data);
    }

    // Get buffer size
    unsigned int size() const {
        return _size;
    }

    // Set buffer size
    void size(unsigned int s) {
        // If we're changing size and either don't have memory allocated or need a different size
        if (s != _size) {
            // If we already have memory allocated, free it
            if (_data) {
                delete[] _data;
            }

            // Allocate new memory
            if (s > 0) {
                _data = new unsigned char[s];
            } else {
                _data = nullptr;
            }
            
            // Update the size
            _size = s;
        }
    }

private:
    unsigned char* _data;
    unsigned int _size;
};

#endif // BUFFER_H