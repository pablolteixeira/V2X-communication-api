#ifndef OBSERVER_H
#define OBSERVER_H

#include <vector>

#include "ordered_list.h"
#include "list.h"
#include "semaphore.h"
#include "buffer.h"
#include "console_logger.h"

// Fundamentals for Observer X Observed
template <typename T, typename Condition = void>
class Conditional_Data_Observer 
{
public:
    typedef T Observed_Data;
    typedef Condition Observing_Condition;

    virtual void update(T* d, unsigned int id) {};

    void set_condition(Condition condition) {
        _condition = condition;
    }
    Condition rank() {
        return _condition;
    }
private:
    Condition _condition;
};

template <typename T, typename Condition = void>
class Conditionally_Data_Observed 
{
public:
    typedef T Observed_Data;
    typedef Condition Observing_Condition;
    typedef Ordered_List<Conditional_Data_Observer<T, Condition>, Condition> Observers;

    Conditionally_Data_Observed() {
        ConsoleLogger::print("Conditionally_Data_Observed: Initializing instance.");
    }
    ~Conditionally_Data_Observed() {}

    void attach(Conditional_Data_Observer<T, Condition>* o, Condition c) {
        //std::cout << "Protocol condition set: " << c << std::endl;
        o->set_condition(c);
        _observers.insert(o);
    }

    void detach(Conditional_Data_Observer<T, Condition>* o, Condition c) {
        _observers.remove(o);
    }

    bool notify(Condition c, unsigned int id, T* d) {
        //ConsoleLogger::print("Conditionally_Data_Observed: Notifying observers.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            //ConsoleLogger::print(std::to_string((*obs)->rank()) + " - " + std::to_string(c));
            if ((*obs)->rank() == c) {
                (*obs)->update(d, id);
                notified = true;
            }
        }
        return notified;
    }

private:
    Observers _observers;
};

// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore

template<typename D, typename C>
class Concurrent_Observer;
 
template<typename D, typename C>
class Concurrent_Observed
{
    friend class Concurrent_Observer<D, C>;

public:
    typedef D Observed_Data;
    typedef Ordered_List<Concurrent_Observer<D, C>, C> Observers;

public:
    Concurrent_Observed() {}
    ~Concurrent_Observed() {}
    
    void attach(Concurrent_Observer<D, C> * o, C c) {
        o->set_condition(c);
        _observers.insert(o);
    }
    
    void detach(Concurrent_Observer<D, C> * o, C c) {
        _observers.remove(o);
    }
    
    bool notify(C c, unsigned int id, D * d) {
        //ConsoleLogger::print("Concurrent_Observed: Starting to notify concurrent observers.");
        bool notified = false;
        Observers tmp_observers;

        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            if ((*obs)->rank() == c || c == 0) {
                tmp_observers.insert((*obs));
                //ConsoleLogger::print("Concurrent_Observed: Notifying concurrent observers.");
                notified = true;
            }
        }

        if (notified) {
            d->set_reference_counter(tmp_observers.size());

            for (auto observer : tmp_observers) {
                observer->update(c, id, d);
            }
        }

        return notified;
    }

private:
    Observers _observers;
};

template<typename D, typename C>
class Concurrent_Observer
{
    friend class Concurrent_Observed<D, C>;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;

public:
    Concurrent_Observer(): _semaphore(0) {
        //ConsoleLogger::print("Concurrent_Observer: Initializing instance.");
    }
    ~Concurrent_Observer() {}
    
    void update(C c, unsigned int id, D * d) {
        std::pair<unsigned int, D*> pair = std::make_pair(id, d);
        _data.insert(pair);
        _semaphore.v();
    }
    
    std::pair<unsigned int, D*> updated() {
        _semaphore.p();
        return _data.remove();
    }

    void stop() {
        _semaphore.v();
    }

    void set_condition(C condition) {
        _condition = condition;
    }

    C rank() {
        return _condition;
    }
private:
    Semaphore _semaphore;
    List<std::pair<unsigned int, D*>> _data;
    C _condition;
};

#endif // OBSERVER_H