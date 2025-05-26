#include "../header/message.h"

// Constructor
Message::Message(size_t max_size) 
    : _buffer(new unsigned char[max_size]), _size(0), _max_size(max_size) {
}

// Destructor
Message::~Message() {
    delete[] _buffer;
}

// Get mutable data pointer
unsigned char* Message::data() {
    return _buffer;
}

// Get const data pointer
const unsigned char* Message::data() const {
    return _buffer;
}

// Get current size
size_t Message::size() const {
    return _size;
}

// Set size
void Message::size(size_t s) {
    _size = (s > _max_size) ? _max_size : s;
}

// Get maximum size
size_t Message::max_size() const {
    return _max_size;
}

Message::Message(const Message& other)
    : _size(other._size), _max_size(other._max_size)
{
    _buffer = new unsigned char[_max_size];
    std::memcpy(_buffer, other._buffer, _size);
}

Message& Message::operator=(const Message& other)
{
    if (this != &other) {
        delete[] _buffer;
        _max_size = other._max_size;
        _size = other._size;
        _buffer = new unsigned char[_max_size];
        std::memcpy(_buffer, other._buffer, _size);
    }
    return *this;
}