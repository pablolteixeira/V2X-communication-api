#ifndef MESSAGE_H
#define MESSAGE_H

#include <cstddef>
#include <chrono>

#include "ethernet.h"
#include "u64_type.h"

struct Origin {
    Ethernet::Address mac;
    unsigned short port;
};

class Message 
{
public:
    enum Type {
        UNDEFINED,
        INTEREST,
        RESPONSE,
        PTP,
        JOIN,
        EXIT
    };

    struct MessageHeader {
        Type type;
        size_t payload_size;
    };

    template <typename T>
    struct TypedMessage {
        MessageHeader header;
        T payload;
    };

    const MessageHeader* get_header() const {
        return reinterpret_cast<const MessageHeader*>(_buffer);
    }
    
    MessageHeader* get_header() {
        return reinterpret_cast<MessageHeader*>(_buffer);
    }

    Type get_type() const {
        return get_header()->type;
    }
    
    void set_type(Type type) {
        get_header()->type = type;
    }

    template <typename T>
    void set_payload(const T& data) {
        if (sizeof(MessageHeader) + sizeof(T) <= _max_size) {
            MessageHeader* header = get_header();
            
            unsigned char* payload_ptr = _buffer + sizeof(MessageHeader);
            memcpy(payload_ptr, &data, sizeof(T));
            
            header->payload_size = sizeof(T);
            
            _size = sizeof(MessageHeader) + sizeof(T);
        }
    }
    
    template <typename T>
    T* get_payload() {
        if (_size >= sizeof(MessageHeader)) {
            return reinterpret_cast<T*>(_buffer + sizeof(MessageHeader));
        }
        return nullptr;
    }

    struct InterestMessage {
        unsigned int type;
        std::chrono::microseconds period;
    };

    struct ResponseMessage {
        unsigned int type;
        int value;
    };

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