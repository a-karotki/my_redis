//
// Created by korot on 12/02/2025.
//
#include <cassert>
#include <cstdlib>

#include "MRD_hashtable.h"

// #include <bits/unordered_map.h>
#include "MRD_buffer.h"
using namespace MRD;

void HTab::init(size_t n) {
    tab = static_cast<HNode **>(calloc(n, sizeof(HNode *)));
    mask = n - 1;
    size = 0;
}

void HTab::insert(HNode *node) {
    size_t pos = node->hcode & mask;
    HNode *next = tab[pos];
    node->next = next;
    tab[pos] = node;
    size++;
}

HTab::~HTab() {
    free(tab);
}

HTab::HTab(HTab &&other) noexcept : HTab() {
    swap(*this, other);
}

HTab & HTab::operator=(HTab &&rhs) noexcept {
    swap(*this, rhs);
    return *this;
}



// hashtable look up subroutine.
// Pay attention to the return value. It returns the address of
// the parent pointer that owns the target node,
// which can be used to delete the target node.
HNode **HTab::lookup(HNode &key, bool (&eq)(HNode &, HNode &)) const {
    if (!tab) {
        return nullptr;
    }
    size_t pos = key.hcode & mask;
    HNode **from = &tab[pos]; //incoming pointer to the target
    for (HNode* cur; (cur = *from) != nullptr; from = &cur->next) {
        if (cur->hcode == key.hcode && eq(*cur, key)) {
            return from;  // may be a node, may be a slot
        }
    }
    return nullptr;
}
// remove a node from the chain
HNode *HTab::detach(HNode **from) {
    HNode* node = *from;    // the target node
    *from = node->next;    // update the incoming pointer to the target
    size--;
    return node;
}

bool HTab::foreach(bool(*f)(HNode *, void *), void *arg) const {
    for (size_t i = 0; mask != 0 && i <= mask; ++i) {
        for (HNode *node = tab[i]; node != nullptr; node = node->next) {
            if (!f(node, arg)) {
                return false;
            }
        }
    }
    return true;
}

void HTab::clear() {
    free(tab);
    tab = nullptr;
    mask = 0;
    size = 0;
}

void MRD::swap(HTab &lhs, HTab &rhs) noexcept {
    std::swap(lhs.tab, rhs.tab);
    std::swap(lhs.mask, rhs.mask);
    std::swap(lhs.size, rhs.size);
}

void HashMap::help_rehashing() {
    size_t nwork = 0;
    while (nwork < k_rehashing_work && older.size > 0) {
        //find a non-empty slot
        HNode **from = &older.tab[migrate_pos];
        if (!*from) {
            migrate_pos++;
            continue;
        }
        newer.insert(older.detach(from));
        nwork++;
    }
    if (older.size == 0 && !older.tab) {
        older.clear();
        older = HTab{};
    }
}
void HashMap::trigger_rehashing() {
    assert(older.tab == nullptr);
    older = std::move(newer);
    newer.init((newer.mask + 1) * 2);
    migrate_pos = 0;
}
HNode* HashMap::lookup(HNode& key, bool (&eq)(HNode&, HNode&)) const {
    HNode **from = newer.lookup(key, eq);
    if (from) {
        return *from;
    }
    from = older.lookup(key, eq);
    if (from) {
        return *from;
    }
    return nullptr;
}

void HashMap::foreach(bool(*f)(HNode *, void *), void *arg) {
    if (!newer.foreach(f, arg)) return;
    older.foreach(f, arg);
}

void HashMap::insert(HNode *node) {
    if (!newer.tab) {
        newer.init(4);
    }
    newer.insert(node);
    if (!older.tab) {
        size_t threshold = (newer.mask + 1) * k_max_load_factor;
        if (newer.size >= threshold) {
            trigger_rehashing();
        }
    }
    help_rehashing();
}


HNode *HashMap::erase(HNode &key, bool (&eq)(HNode&, HNode&)) {
    help_rehashing();
    if (HNode **from = newer.lookup(key, eq)) {
        return newer.detach(from);
    }
    if (HNode **from = older.lookup(key, eq)) {
        return older.detach(from);
    }
    return nullptr;
}
void HashMap::clear() {
    newer.clear();
    older.clear();
    migrate_pos = 0;
}

size_t HashMap::size() const {
    return newer.size + older.size;
}


void MRD::swap(HashMap &lhs, HashMap &rhs) noexcept {
    swap(lhs.newer, rhs.newer);
    swap(lhs.older, rhs.older);
    std::swap(lhs.migrate_pos, rhs.migrate_pos);
}
