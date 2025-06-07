#ifndef NODE_H
#define NODE_H

template<typename T, typename V>
class Node
{
    T key;
    V value;

    Node* prev;
    Node* next;

    Node() : prev(nullptr), next(nullptr);
    Node(T key, V value) : key(key), value(value), prev(nullptr), next(nullptr);
};

#endif // NODE_H