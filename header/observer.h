#ifndef OBSERVER_H
#define OBSERVER_H

#include "ordered_list.h"
#include "list.h"
#include "semaphore.h"
#include "buffer.h"
#include "console_logger.h"

// Fundamentals for Observer X Observed
template <typename A, typename T, typename Condition = void>
class Conditional_Data_Observer 
{
public:
    typedef A NIC_Address;
    typedef T Observed_Data;
    typedef Condition Observing_Condition;

    virtual void update(A& a, Condition c, T* d) {};

    void set_condition(Condition condition) {
        _condition = condition;
    }
    Condition rank() {
        return _condition;
    }
private:
    Condition _condition;
};

template <typename A, typename T, typename Condition = void>
class Conditionally_Data_Observed 
{
public:
    typedef A NIC_Address;
    typedef T Observed_Data;
    typedef Condition Observing_Condition;
    typedef Ordered_List<Conditional_Data_Observer<A, T, Condition>, Condition> Observers;

    Conditionally_Data_Observed() {
        //ConsoleLogger::print("Conditionally_Data_Observed: Initializing instance.");
    }
    ~Conditionally_Data_Observed() {}

    void attach(Conditional_Data_Observer<A, T, Condition>* o, Condition c) {
        //std::cout << "Protocol condition set: " << c << std::endl;
        o->set_condition(c);
        _observers.insert(o);
    }

    void detach(Conditional_Data_Observer<A, T, Condition>* o, Condition c) {
        _observers.remove(o);
    }

    bool notify(A& a, Condition c, T* d) {
        ConsoleLogger::print("Conditionally_Data_Observed: Notifying observers.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            if ((*obs)->rank() == c) {
                (*obs)->update(a, c, d);
                notified = true;
            }
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
        //ConsoleLogger::print("Concurrent_Observed: Attach");
        //std::cout << "Condition value: " << c << std::endl;
        o->set_condition(c);
        _observers.insert(o);
    }
    
    void detach(Concurrent_Observer<D, C> * o, C c) {
        _observers.remove(o);
    }
    
    bool notify(C c, D * d) {
        //ConsoleLogger::print("Concurrent_Observed: Starting to notify concurrent observers.");
        bool notified = false;
        for(typename Observers::Iterator obs = _observers.begin(); obs != _observers.end(); ++obs) {
            std::cout << "Rank: " << (*obs)->rank() << " AND c: " << c << std::endl; 
            if((*obs)->rank() == c) {
                ConsoleLogger::print("Concurrent_Observed: Notifying concurrent observers.");
                (*obs)->update(c, d);
                notified = true;
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
    
    void update(C c, D * d) {
        _data.insert(d);
        _semaphore.v();
    }
    
    D * updated() {
        _semaphore.p();
        return _data.remove();
    }
    
    void set_condition(C condition) {
        _condition = condition;
    }
    C rank() {
        return _condition;
    }
private:
    Semaphore _semaphore;
    C _condition;
    List<D> _data;
};

#endif // OBSERVER_H