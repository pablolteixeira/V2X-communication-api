#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "node.h"
#include <unordered_map>

template <typename T, typename V>
class LRU_Cache
{
private:
    typedef Node<T, V> NodeT;

    int capacity;
    std::unordered_map<T, NodeT*> cache;

    NodeT* head;
    NodeT* tail;

    void remove(NodeT* node) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    void add(NodeT* node) {
        node->next = head->next;
        node->next->prev = node;
        head->next = node;
        node->prev = head;
    }

public:
    LRU_Cache(int capacity) {
        this->capacity = capacity;
        head = new NodeT();
        tail = new NodeT();

        head->next = tail;
        tail->next = head;
    }
    ~LRU_Cache() {
        NodeT* current = head;
        while (current != tail) {
            NodeT* next = current->next;
            delete current;
            current = next;
        }

        delete head;
        delete tail;
    }

    V* get(const T& key) {
        if (cache.find(key) == cache.end()) {
            return nullptr;
        }
        NodeT* node = cache[key];

        remove(node);
        add(node);

        return &(node->value); 
    }

    void put(const T& key, const V& value) {
        if (cache.find(key) != cache.end()) {
            NodeT* existing = cache[key];
            remove(existing);
            delete existing;
        }

        NodeT* node = new NodeT(key, value);
        cache[key] = node;
        add(node);

        if (cache.size() > capacity) {
            NodeT* last = tail->prev;
            cache.erase(last->key);
            remove(last);
            delete last;
        }
    }

    void erase(const T& key) {
        if (cache.find(key) != cache.end()) {
            NodeT* node = cache[key];

            remove(node);
            cache.erase(node->key);

            delete node;
        }
    }
};

#endif