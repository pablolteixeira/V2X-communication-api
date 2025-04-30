#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstddef>

class Message 
{
public:
    // Constructor with a specified maximum size
    Message(size_t max_size = 1500);

    // Destructor
    ~Message();

    Message(const Message& other);
    Message& operator=(const Message& other);

    // Get access to the raw data buffer
    unsigned char* data();
    const unsigned char* data() const;

    // Get current message size
    size_t size() const;

    // Set message size (cannot exceed max_size)
    void size(size_t s);

    // Get maximum capacity
    size_t max_size() const;

    // Template method to set data from a specific type
    template<typename T>
    void set_data(const T& data) {
        if(sizeof(T) <= _max_size) {
            memcpy(_buffer, &data, sizeof(T));
            _size = sizeof(T);
        }
    }

    // Template method to get data as a specific type (const version)
    template<typename T>
    T* get_data() {
        return reinterpret_cast<T*>(_buffer);
    }

private:
    unsigned char* _buffer;     // Dynamic array of bytes
    size_t _size;               // Current message size
    size_t _max_size;           // Maximum capacity
};

#endif // MESSAGE_H