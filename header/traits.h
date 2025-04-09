#ifndef TRAITS_H
#define TRAITS_H

template<typename T>
class Traits 
{
public:
    static const unsigned int SEND_BUFFERS = 10;
    static const unsigned int RECEIVE_BUFFERS = 10;
    static const unsigned int ETHERNET_PROTOCOL_NUMBER = 0x8888;
};

#endif // TRAITS_H