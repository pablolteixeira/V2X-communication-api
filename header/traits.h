#ifndef TRAITS_H
#define TRAITS_H

template<typename T>
class Traits 
{
public:
    static const unsigned int SEND_BUFFERS = 16;
    static const unsigned int RECEIVE_BUFFERS = 16;
    static const unsigned int ETHERNET_PROTOCOL_NUMBER = 0x8888;
    static const unsigned int NUM_COMPONENTS = 5;
    static const unsigned int NUM_VEHICLE = 2;

};

#endif // TRAITS_H