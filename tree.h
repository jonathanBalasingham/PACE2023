//
// Created by jonathan on 5/7/23.
//

#ifndef PACE_TREE_H
#define PACE_TREE_H

#include <memory>
#include <utility>
#include <vector>

using namespace  std;

template <typename T> class Node;

template <typename T>
class Node {
public:
    using shared_node = shared_ptr<Node<T>>;
    Node(T c) {
        parent = nullptr;
        children = vector<shared_node>();
        is_leaf = false;
        content = c;
    }

    Node() {
        parent = nullptr;
        children = vector<shared_node>();
        is_leaf = false;
        content = T();
    }
    shared_node parent;
    vector<shared_node> children;
    bool is_leaf;
    T content;
};

template <typename T>
class Tree {
    using shared_node = shared_ptr<Node<T>>;
    shared_node root;
    int size = 0;

public:
    Tree() {
        size = 0;
        root = make_shared<Node<T>>(Node<T>());
    }

    shared_node get_root(){ return root; }

    void insert(shared_node target, shared_node new_node) {
        target->children.push_back(new_node);
        new_node->parent = target;
        new_node->is_leaf = true;
        size++;
    }
};



#endif //PACE_TREE_H
