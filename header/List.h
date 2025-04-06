#ifndef LIST_H
#define LIST_H

#include <list>

template<typename T>
class List 
{
public:
    List() {}
    ~List() {}

    void insert(T* item) {
        _items.push_back(item);
    }

    T* remove() {
        if (_items.empty()) return nullptr;
        T* item = _items.front();
        _items.pop_front();
        return item;
    }

    bool empty() const {
        return _items.empty();
    }

private:
    std::list<T*> _items;
};

#endif // LIST_H
