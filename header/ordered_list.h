#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include <iostream>
#include <list>
#include <iterator>

template<typename T, typename C = void>
class Ordered_List 
{
public:
    class Iterator
    {
    public:
        Iterator(typename std::list<T*>::iterator it) _it(it) {};
        Iterator& operator++() {
            ++_it;
            return *this;
        }
        bool operator!=(const Iterator& other) const {
            return it != other.it;
        }
        T* operator*() const {
            return *_it;
        }
    private:
        typename std::list<T*>::iterator _it;
    }

    Ordered_List() {}
    ~Ordered_List() {}

    void insert(T* item) {
        if (!item) return;

        auto it = _items.begin();

        while (it != _items.end() && **it < *item) {
            if (**it == *item) { 
                return;
            }

            ++it;
        }

        _items.insert(it, item);        
    }

    void remove(T* item) {
        if (!item) return;
        _items.remove(item);
    }

    void print() const {
        std::cout << "[";
        for (auto it = _items.begin(); it != _items.end(); it++) {
            std::cout << **it << " ";
        }
    
        std::cout << "]" << std::endl;
    }

    Iterator begin() { 
        return Iterator(_items.begin()); 
    }
    Iterator end() { 
        return Iterator(_items.end()); 
    }

private:
    std::list<T*> _items;
};

#endif // ORDERED_LIST_H