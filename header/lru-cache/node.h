#ifndef NODE_H
#define NODE_H

template<typename T, typename V>
class Node
{
public:
    T key;
    V value;

    Node* prev;
    Node* next;
    Node() : prev(nullptr), next(nullptr) {};
    Node(T key, const V value) : key(key), prev(nullptr), next(nullptr) {
        this->value = value; 
    }
};

#endif // NODE_H