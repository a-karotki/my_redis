//
// Created by korot on 10/03/2025.
//

#include "MRD_avl.h"

MRD::AVLNode::AVLNode() {
    left = right = parent = nullptr;
    height = 1;
    count = 1;
}

void MRD::AVLNode::update() {
    height = std::max(h(left), h(right)) + 1;
    count = 1 + cnt(left) + cnt(right);
}

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

MRD::AVLNode * MRD::AVLNode::successor(AVLNode *node) {
    if (node->right) {
        node = node->right;
        while (node->left) {
            node = node->left;
        }
        return node;
    }
    while (AVLNode *parent = node->parent) {
        if (node == parent->left) {
            return parent;
        }
        node = parent;
    }
    return nullptr;
}

MRD::AVLNode * MRD::AVLNode::predscessor(AVLNode *node) {
    if (node->left) {
        node = node->left;
        while (node->right) {
            node = node->right;
        }
        return node;
    }
    while (AVLNode *parent = node->parent) {
        if (node == parent->right) {
            return parent;
        }
        node = parent;
    }
    return nullptr;
}

int64_t MRD::AVLNode::rank(AVLNode *node) {
    int64_t rank = 0;
    if (node->left) {
        rank += AVLNode::cnt(node->left);
    }
    AVLNode* parent = node->parent;
    if (parent && parent->right == node) {
        rank += AVLNode::cnt(parent->left) + 1;
    }
    return rank;
}

MRD::AVLNode * MRD::AVLNode::offset(AVLNode *node, int64_t offset) {
    int64_t pos =  0;
    while (pos != offset) {
        if (pos < offset && pos + AVLNode::cnt(node->right) >= offset) {
            node = node->right;
            pos += AVLNode::cnt(node->left) + 1;
        }
        else if (pos > offset && pos - AVLNode::cnt(node->left) <= offset) {
            node = node->left;
            pos -= AVLNode::cnt(node->right) + 1;
        }
        else {
            AVLNode *parent = node->parent;
            if (!parent)
                return nullptr;
            if (parent->right == node) {
                pos -= AVLNode::cnt(node->left) + 1;
            }
            else {
                pos += AVLNode::cnt(node->right) + 1;
            }
            node = parent;
        }
    }
    return node;
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
