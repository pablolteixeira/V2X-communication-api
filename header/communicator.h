#ifndef COMMUNICATOR_H
#define COMMUNICATOR_H

// Communication End-Point (for client classes)

#include "observer.h"
#include "message.h"
#include "console_logger.h"

template <typename Channel>
class Communicator: public Concurrent_Observer<typename Channel::Observer::Observed_Data>
{
    typedef Concurrent_Observer<typename Channel::Observer::Observed_Data> Observer;

public:
    typedef typename Channel::NICBuffer Buffer;
    typedef typename Channel::Address Address;

public:
    Communicator(Channel* channel, Address address): _channel(channel), _address(address) {
        _channel->attach(this);
    }

    ~Communicator() { Channel::detach(this); }
    
    bool send(const Message * message, Address from, Address to) {
        ConsoleLogger::print("Communicator: Sending message.");
        return (_channel->send(from, to, message->data(),
            message->size()) > 0);
    }

    bool receive(Message * message) {
        Buffer * buf = Observer::updated(); // block until a notification is triggered
        if (!buf) return false;
        typename Channel::Address from;
        int size = _channel->receive(buf, from, message->data(), message->max_size());
        if(size > 0) {
            message->size(size);
            return true;
        }

        return false;
    }

private:
    void update(typename Channel::Observed * obs, Buffer * buf) {
        Observer::update(buf); // releases the thread waiting for data
    }

private:
    Channel * _channel;
    Address _address;
};

#endif // COMMUNICATOR_H