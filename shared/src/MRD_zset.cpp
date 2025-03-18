//
// Created by korot on 11/03/2025.
//

#include "MRD_zset.h"
using namespace MRD;

ZNode::ZNode(const char *name_, size_t len_, double score_) : name(len_, '\0') {
    h_node.next = nullptr;
    h_node.hcode = str_hash(name_, len_);
    score = score_;
    len = len_;
    memcpy(name.data(), name_, len);
}

ZNode::ZNode(const std::string &name, double score_) : ZNode(name.data(), name.length(), score_) {
}

ZNode * ZNode::offset(ZNode *node, int64_t offset) {
    AVLNode *tnode = node ? AVLNode::offset(&node->tree_node, offset) : nullptr;
    return tnode ? container_of(tnode, ZNode, tree_node) : nullptr;
}

ZNode *ZSet::lookup(const char *name, size_t len) const {
    if (!root)
        return nullptr;
    HKey key;
    key.node.hcode = str_hash(name, len);
    key.name = name;
    key.len = len;
    HNode *found = hmap.lookup(key.node, hcmp);
    return found ? container_of(found, ZNode, h_node) : nullptr;
}

void ZSet::del(ZNode *node) {
    HKey key;
    key.node.hcode = str_hash(node->name.data(), node->len);
    key.name = node->name.data();
    key.len = node->len;
    HNode *found = hmap.erase(key.node, hcmp);
    root = AVLNode::del(&node->tree_node);
    delete node;
}

ZSet::~ZSet() {
    hmap.clear();
    tree_dispose(root);
}



ZSet::ZSet(ZSet &&other) noexcept : ZSet() {
    swap(*this, other);
}

ZSet& ZSet::operator=(ZSet rhs) {
    swap(*this, rhs);
    return *this;
}

ZSet & ZSet::operator=(ZSet &&rhs) noexcept {
    swap(*this, rhs);
    return *this;
}


void ZSet::set_cpy(AVLNode* node, ZSet *to_cpy) {
    if (node == nullptr)
        return;
    ZNode *rhs_node = container_of(node, ZNode, tree_node);;
    to_cpy->insert(rhs_node->name.data(), rhs_node->name.length(), rhs_node->score);
    set_cpy(node->left, to_cpy);
    set_cpy(node->right, to_cpy);
}

ZSet::ZSet(const ZSet &rhs){
    set_cpy(rhs.root, this);
}

bool ZSet::rem(const std::string &name) {
    ZNode *node = lookup(name.data(), name.length());
    if (!node)
        return false;
    del(node);
    return true;
}

void ZSet::update(ZNode *node, double score) {
    root = AVLNode::del(&node->tree_node);
    node->tree_node = AVLNode();
    node->score = score;
    tree_insert(node);
}


bool ZSet::insert(const char *name, size_t len, double score) {
    if (ZNode* node = lookup(name, len)) {
        update(node, score);
        return false;
    }
    auto node = new ZNode(name, len, score);
    hmap.insert(&node->h_node);
    tree_insert(node);
    return true;
}

int64_t ZSet::rank(const std::string &name) const {
    ZNode *node = lookup(name.data(), name.length());
    if (!node)
        return 0;
    return AVLNode::rank(&node->tree_node);
}

void ZSet::tree_insert(ZNode *node) {
    AVLNode *parent = nullptr;
    AVLNode **from = &root;
    while (*from) {
        parent = *from;
        from = zless(&node->tree_node, parent) ? &parent->left : &parent->right;
    }
    *from = &node->tree_node;
    node->tree_node.parent = parent;
    root = AVLNode::fix(&node->tree_node);
}

bool ZSet::tree_delete(ZNode *node) {
    AVLNode *cur = root;
    while (cur) {
        if (zeq(&node->tree_node, cur))
            break;
        cur = zless(&node->tree_node, cur) ? cur->left : cur->right;
    }
    if (!cur)
        return false;
    root = AVLNode::del(cur);
    delete container_of(cur, ZNode, tree_node);
    return true;
}
void ZSet::tree_dispose(AVLNode *node) {
    if (!node) {
        return;
    }
    tree_dispose(node->left);
    tree_dispose(node->right);
    delete container_of(node, ZNode, tree_node);
}


ZNode *ZSet::seekge(const double score, const char *name, const size_t len) {
    AVLNode *found = nullptr;
    AVLNode *node = root;
    while (node) {
        if (zless(node, score, name, len)) {
            node = node->right;
            continue;
        }
        found = node;
        node = node->left;
    }
    return found ? container_of(found, ZNode, tree_node) : nullptr;
}

ZNode * ZSet::seekle(double score, const char *name, size_t len) {
    AVLNode *found = nullptr;
    AVLNode *node = root;
    while (node) {
        if (zgreater(node, score, name, len)) {
            node = node->left;
            continue;
        }
        found = node;
        node = node->right;
    }
    return found ? container_of(found, ZNode, tree_node) : nullptr;
}



bool ZSet::hcmp(HNode &node_, HNode &key) {
    ZNode *znode = container_of(&node_, ZNode, h_node);
    HKey *hkey = container_of(&key, HKey, node);
    if (znode->len != hkey->len) {
        return false;
    }
    return 0 == memcmp(znode->name.data(), hkey->name, znode->len);
}

bool ZSet::zless(AVLNode *lhs, AVLNode *rhs) {
    ZNode *l = container_of(lhs, ZNode, tree_node);
    ZNode *r = container_of(rhs, ZNode, tree_node);
    if (l->score != r->score)
        return l->score < r->score;
    int res = memcmp(l->name.data(), r->name.data(), std::min(l->len, r->len));
    return (res != 0) ? (res < 0) : (l->len < r->len);
}
bool ZSet::zless(AVLNode *lhs, const double score, const char *name, const size_t len) {
    ZNode *l = container_of(lhs, ZNode, tree_node);
    if (l->score != score)
        return l->score < score;
    int res = memcmp(l->name.data(), name, std::min(l->len, len));
    return (res != 0) ? (res < 0) : (l->len < len);
}
bool ZSet::zgreater(AVLNode *lhs, const double score, const char *name, const size_t len) {
    ZNode *l = container_of(lhs, ZNode, tree_node);
    if (l->score != score)
        return l->score > score;
    int res = memcmp(l->name.data(), name, std::min(l->len, len));
    return (res != 0) ? (res > 0) : (l->len > len);
}

bool ZSet::zeq(AVLNode *lhs, AVLNode *rhs) {
    ZNode *l = container_of(lhs, ZNode, tree_node);
    ZNode *r = container_of(rhs, ZNode, tree_node);
    if (l->score == r->score)
        return true;
    if (l->len != r->len)
        return false;
    return  0 == memcmp(l->name.data(), r->name.data(), l->len);
}

void MRD::swap(ZSet &lhs, ZSet &rhs) noexcept {
    std::swap(lhs.root, rhs.root);
    swap(lhs.hmap, rhs.hmap);
}
