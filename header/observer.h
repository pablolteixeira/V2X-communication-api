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
        std::pair<unsigned int, D*>* pair = new std::pair<unsigned int, D*>(id, d);
        _data.insert(pair);
        _semaphore.v();
    }
    
    std::pair<unsigned int, D*>* updated() {
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

/*
==292426==ERROR: AddressSanitizer: attempting free on address which was not malloc()-ed: 0x6100000000a0 in thread T3
    #0 0x7f86830c551f in operator delete(void*) ../../../../src/libsanitizer/asan/asan_new_delete.cc:165
    #1 0x55d84100e34f in __gnu_cxx::new_allocator<std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > >::deallocate(std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> >*, unsigned long) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x3134f)
    #2 0x55d84100e22a in std::allocator_traits<std::allocator<std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > > >::deallocate(std::allocator<std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > >&, std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> >*, unsigned long) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x3122a)
    #3 0x55d841028e63 in std::__cxx11::_List_base<std::pair<unsigned int, Buffer<Ethernet::Frame>*>, std::allocator<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > >::_M_put_node(std::_List_node<std::pair<unsigned int, Buffer<Ethernet::Frame>*> >*) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4be63)
    #4 0x55d84102860c in std::__cxx11::list<std::pair<unsigned int, Buffer<Ethernet::Frame>*>, std::allocator<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > >::_M_erase(std::_List_iterator<std::pair<unsigned int, Buffer<Ethernet::Frame>*> >) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4b60c)
    #5 0x55d841026b89 in std::__cxx11::list<std::pair<unsigned int, Buffer<Ethernet::Frame>*>, std::allocator<std::pair<unsigned int, Buffer<Ethernet::Frame>*> > >::pop_front() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x49b89)
    #6 0x55d84102442e in List<std::pair<unsigned int, Buffer<Ethernet::Frame>*> >::remove() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4742e)
    #7 0x55d841021050 in Concurrent_Observer<Buffer<Ethernet::Frame>, unsigned short>::updated() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x44050)
    #8 0x55d84101d045 in Communicator<Protocol<NIC<RawSocketEngine> > >::receive(Message*, unsigned int*) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x40045)
    #9 0x55d8410120c8 in SmartData::receive() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x350c8)
    #10 0x55d84102a52c in void std::__invoke_impl<void, void (SmartData::*)(), SmartData*>(std::__invoke_memfun_deref, void (SmartData::*&&)(), SmartData*&&) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4d52c)
    #11 0x55d84102a342 in std::__invoke_result<void (SmartData::*)(), SmartData*>::type std::__invoke<void (SmartData::*)(), SmartData*>(void (SmartData::*&&)(), SmartData*&&) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4d342)
    #12 0x55d84102a222 in void std::thread::_Invoker<std::tuple<void (SmartData::*)(), SmartData*> >::_M_invoke<0ul, 1ul>(std::_Index_tuple<0ul, 1ul>) (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4d222)
    #13 0x55d84102a189 in std::thread::_Invoker<std::tuple<void (SmartData::*)(), SmartData*> >::operator()() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4d189)
    #14 0x55d84102a0f1 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (SmartData::*)(), SmartData*> > >::_M_run() (/home/pablo/Desktop/INE-5424-SO2/bin/main+0x4d0f1)
    #15 0x7f8682ea9df3  (/lib/x86_64-linux-gnu/libstdc++.so.6+0xd6df3)
    #16 0x7f8682c4e608 in start_thread /build/glibc-B3wQXB/glibc-2.31/nptl/pthread_create.c:477
    #17 0x7f8682b73352 in __clone (/lib/x86_64-linux-gnu/libc.so.6+0x11f352)
*/