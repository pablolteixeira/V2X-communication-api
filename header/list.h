#ifndef LIST_H
#define LIST_H

#include <list>
#include <mutex>

template<typename T>
class List 
{
public:
    List() {}
    ~List() {}

    void insert(T* item) {
        std::lock_guard<std::mutex> lock(_items_mutex);
        _items.push_back(item);
    }

    T* remove() {
        std::lock_guard<std::mutex> lock(_items_mutex);
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
    std::mutex _items_mutex;
};

#endif // LIST_H
