#ifndef TRAITS_H
#define TRAITS_H

template<typename T>
class Traits 
{
public:
    static const unsigned int SEND_BUFFERS = 16;
    static const unsigned int RECEIVE_BUFFERS = 16;
    static const unsigned int ETHERNET_PROTOCOL_NUMBER = 0x8888;
    static const unsigned int NUM_COMPONENTS = 4;
    static const unsigned int NUM_VEHICLE = 2;
    static const unsigned int NUM_RSU = 4;

    unsigned int pick_random_quadrant() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distr(1, NUM_RSU);
        return distr(gen);
    }
};

#endif // TRAITS_H