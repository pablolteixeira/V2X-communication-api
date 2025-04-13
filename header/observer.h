#ifndef OBSERVER_H
#define OBSERVER_H

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

    virtual void update(Condition c, T* d) {};
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
        _observers.insert(o);
    }

    void detach(Conditional_Data_Observer<T, Condition>* o, Condition c) {
        _observers.remove(o);
    }

    bool notify(Condition c, T* d) {
        ConsoleLogger::print("Conditionally_Data_Observed: Notifying observers.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            (*obs)->update(c, d);
            notified = true;
        }
        return notified;
    }

private:
    Observers _observers;
};

// Conditional Observer x Conditionally Observed with Data decoupled by a Semaphore

template<typename D, typename C = void>
class Concurrent_Observer;
 
template<typename D, typename C = void>
class Concurrent_Observed
{
    friend class Concurrent_Observer<D, C>;

public:
    typedef D Observed_Data;
    typedef C Observing_Condition;
    typedef Ordered_List<Concurrent_Observer<D, C>, C> Observers;

public:
    Concurrent_Observed() {}
    ~Concurrent_Observed() {}
    
    void attach(Concurrent_Observer<D, C> * o, C c) {
        _observers.insert(o);
    }
    
    void detach(Concurrent_Observer<D, C> * o, C c) {
        _observers.remove(o);
    }
    
    bool notify(C c, D * d) {
        ConsoleLogger::print("Concurrent_Observed: Initializing instance.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            /*if(obs->rank() == c) {
                obs->update(c, d);
                notified = true;
            }*/
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
        ConsoleLogger::print("Concurrent_Observer: Initializing instance.");
    }
    ~Concurrent_Observer() {}
    
    void update(C c, D * d) {
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