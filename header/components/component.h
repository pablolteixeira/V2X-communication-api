#ifndef COMPONENT_H
#define COMPONENT_H

#include "component_messages.h"

// Component base class
class Component {
    public:
        Component(const std::string& name) : _id(name), _running(false) {}
        virtual ~Component() {
            stop();
        }
    
        virtual void start() {
            if (_running) return;
            _running = true;
            _thread = std::thread(&Component::run, this);
        }
    
        virtual void stop() {
            if (!_running) return;
            _running = false;
            if (_thread.joinable()) {
                _thread.join();
            }
        }
    
        const std::string& name() const { return _id; }
    
    protected:
        virtual void run() = 0;
    
        std::string _id;
        std::atomic<bool> _running;
        std::thread _thread;
    };

#endif