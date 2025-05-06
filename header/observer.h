#ifndef OBSERVER_H
#define OBSERVER_H

#include "ordered_list.h"
#include "list.h"
#include "semaphore.h"
#include "buffer.h"
#include "console_logger.h"
#include <vector>

// Fundamentals for Observer X Observed
template <typename T, typename Condition = void>
class Conditional_Data_Observer 
{
public:
    typedef T Observed_Data;
    typedef Condition Observing_Condition;

    virtual void update(T* d) {};

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
        //ConsoleLogger::print("Conditionally_Data_Observed: Initializing instance.");
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

    bool notify(Condition c, T* d) {
        ConsoleLogger::print("Conditionally_Data_Observed: Notifying observers.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            ConsoleLogger::print(std::to_string((*obs)->rank()) + " - " + std::to_string(c));
            if ((*obs)->rank() == c) {
                (*obs)->update(d);
                notified = true;
            }
        }
        return notified;
    }

private:
    Observers _observers;
};

// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore

template<typename D>
class Concurrent_Observer;
 
template<typename D>
class Concurrent_Observed
{
    friend class Concurrent_Observer<D>;

public:
    typedef D Observed_Data;
    typedef Ordered_List<Concurrent_Observer<D>> Observers;

public:
    Concurrent_Observed() {}
    ~Concurrent_Observed() {}
    
    void attach(Concurrent_Observer<D> * o) {
        _observers.insert(o);
    }
    
    void detach(Concurrent_Observer<D> * o) {
        _observers.remove(o);
    }
    
    bool notify(D * d) {
        //ConsoleLogger::print("Concurrent_Observed: Starting to notify concurrent observers.");
        bool notified = false;

        d->set_reference_counter(_observers.size());
        
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            ConsoleLogger::print("Concurrent_Observed: Notifying concurrent observers."); 
            (*obs)->update(d);
            notified = true;
        }

        return notified;
    }

private:
    Observers _observers;
};

template<typename D>
class Concurrent_Observer
{
    friend class Concurrent_Observed<D>;

public:
    typedef D Observed_Data;

public:
    Concurrent_Observer(): _semaphore(0) {
        //ConsoleLogger::print("Concurrent_Observer: Initializing instance.");
    }
    ~Concurrent_Observer() {}
    
    void update(D * d) {
        std::cout << "OBSERVED BUFFER POINTER ADDED: " << d << std::endl;
        _data.insert(d);
        _semaphore.v();
    }
    
    D * updated() {
        _semaphore.p();
        return _data.remove();
    }
private:
    Semaphore _semaphore;
    List<D> _data;
};

#endif // OBSERVER_H