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
        content = make_unique(c);
    }

    Node() {
        parent = nullptr;
        children = vector<shared_node>();
        is_leaf = false;
        content = nullptr;
    }
    shared_node parent;
    vector<shared_node> children;
    bool is_leaf;
    unique_ptr<T> content;
};

template <typename T>
class Tree {
    using shared_node = shared_ptr<Node<T>>;
    shared_node root;
    int size = 0;

public:
    void insert(shared_node target, const shared_node& new_node) {
        target->children.push_back(new_node);
        new_node->parent = std::move(target);
        new_node->is_leaf = true;
        size++;
    }
};



#endif //PACE_TREE_H
