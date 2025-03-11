//
// Created by korot on 10/03/2025.
//

#include "MRD_avl.h"

MRD::AVLNode * MRD::AVLNode::del(AVLNode *node) {
    if (!node->left || !node->right) {
        return del_easy(node);
    }
    //successor
    AVLNode *victim = node->right;
    while (victim->left) {
        victim = victim->left;
    }
    AVLNode *root = del_easy(victim);
    *victim = *node;
    if (victim->left) {
        victim->left->parent = victim;
    }
    if (victim->right) {
        victim->right->parent = victim;
    }
    AVLNode **from = &root;
    AVLNode *parent = node->parent;
    if (parent) {
        from = parent->left == node ? &parent->left : &parent->right;
    }
    *from = victim;
    return root;
}

MRD::AVLNode * MRD::AVLNode::fix(AVLNode *node) {
    while (true) {
        AVLNode **from = &node;
        AVLNode *parent = node->parent;
        if (parent)
            from = parent->left == node ? &parent->left : &parent->right;
        node->update();
        uint32_t l = AVLNode::h(node->left);
        uint32_t r = AVLNode::h(node->right);
        if (l == r + 2)
            *from = fix_left(node);
        else if (l + 2 == r) {
            *from = fix_right(node);
        }
        if (!parent) {
            return *from;
        }
        node = parent;
    }
}

uint32_t MRD::AVLNode::h(const AVLNode *node) {
    return node ? node->height : 0;
}

uint32_t MRD::AVLNode::cnt(const AVLNode *node) {
    return node ? node->count : 0;
}

MRD::AVLNode * MRD::AVLNode::rot_left(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->right;
    AVLNode *inner = new_node->left;
    // node <-> inner
    node->right = inner;
    if (inner)
        inner->parent = node;
    // parent <- new node
    new_node->parent = parent;
    // new_node <-> node
    new_node->left = node;
    node->parent = new_node;

    node->update();
    new_node->update();
    return new_node;
}

MRD::AVLNode * MRD::AVLNode::rot_right(AVLNode *node) {
    AVLNode *parent = node->parent;
    AVLNode *new_node = node->left;
    AVLNode *inner = new_node->right;
    // node <-> inner
    node->left = inner;
    if (inner)
        inner->parent = node;
    // parent <- new_node
    new_node->parent = parent;
    //new_node <-> node
    new_node->right = node;
    node->parent = new_node;

    node->update();
    new_node->update();
    return new_node;
}

MRD::AVLNode * MRD::AVLNode::fix_left(AVLNode *node) {
    if (AVLNode::h(node->left->left) < AVLNode::h(node->left->right)) {
        node->left = rot_left(node->left);
    }
    return rot_right(node);
}

MRD::AVLNode * MRD::AVLNode::fix_right(AVLNode *node) {
    if (AVLNode::h(node->right->right) < AVLNode::h(node->right->left)) {
        node->right = rot_right(node->right);
    }
    return rot_left(node);
}

MRD::AVLNode * MRD::AVLNode::del_easy(AVLNode *node) {
    assert(!node->left || !node->right);
    AVLNode* child = node->left ? node->left : node->right;
    AVLNode* parent = node->parent;
    if (child)
        child->parent = parent;
    if (!parent)
        return child;
    AVLNode **from = parent->left == node ? &parent->left : &parent->right;
    *from = child;
    return fix(parent);
}
