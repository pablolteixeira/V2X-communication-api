#ifndef ORDERED_LIST_H
#define ORDERED_LIST_H

#include <list>

template<typename T, typename C = void>
class Ordered_List 
{
public:
    class Iterator 
    {
    public:
        Iterator(typename std::list<T*>::iterator it) : _it(it) {}
        Iterator& operator++() { ++_it; return *this; }
        bool operator!=(const Iterator& other) const { return _it != other._it; }
        T* operator->() const { return *_it; }
        T* operator*() const { return *_it; }

    private:
        typename std::list<T*>::iterator _it;
    };

    Ordered_List() {}
    ~Ordered_List() {}

    void insert(T* item) {
        auto it = _items.begin();
        while (it != _items.end() && comp(**it, *item)) {
            ++it;
        }
        _items.insert(it, item);
    }

    void remove(T* item) {
        _items.remove(item);
    }

    Iterator begin() { return Iterator(_items.begin()); }
    Iterator end() { return Iterator(_items.end()); }

private:
    std::list<T*> _items;
    C comp;
};

#endif // ORDERED_LIST_H