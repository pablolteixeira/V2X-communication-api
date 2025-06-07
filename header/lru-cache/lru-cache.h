#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include "node.h"
#include <unordered_map>

template <typename T, typename V>
class LRU_Cache
{
private:
    int capacity;
    std::unordered_map<T, Node<T, V>*> cache;

    Node* head;
    Node* tail;

    void remove(Node* node) {
        node->next->prev = node->prev;
        node->prev->next = node->next;
    }
    void add(Node* node) {
        node->next = head->next;
        node->next->prev = node;
        head->next = node;
        node->prev = head;
    }

public:
    LRU_Cache(int capacity) {
        this->capacity = capacity;
        head = new Node();
        tail = new Node();

        head->next = tail;
        tail->next = head;
    }
    ~LRU_Cache() {
        Node* current = head;
        while (current != tail) {
            Node* next = current->next;
            delete current;
            current = next;
        }

        delete head;
        delete tail;
    }

    V* get(const T& key) {
        if (cache.find(key) != cache.end()) {
            Node* node = cache[key];

            remove(node);
            add(node);

            return node->value; 
        }

        return nullptr;
    }
    void put(const T& key, const V& value) {
        if (cache.find(key) != cache.end()) {
            remove(cache[key]);
        }
        Node* node = new Node(key, value);
        add(node);

        cache[key] = node;

        if (cache.size() > capacity) {
            Node* lru = tail->prev;

            remove(lru);
            cache.erase(lru->key);
            delete lru;
        }
    }

    void erase(const T& key) {
        if (cache.find(key) != cache.end()) {
            Node* node = cache[key];

            remove(node);
            cache.erase(node->key);

            delete node;
        }
    }
};

#endif