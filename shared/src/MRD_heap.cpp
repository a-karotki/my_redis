//
// Created by korot on 20/03/2025.
//

#include "MRD_heap.h"
#include <vector>

using namespace MRD;

size_t HeapItem::parent(size_t pos) {
    return (pos + 1) / 2 - 1;
}

size_t HeapItem::left(size_t pos) {
    return pos * 2 + 1;
}

size_t HeapItem::right(size_t pos) {
    return pos * 2 + 2;
}

void HeapItem::down(HeapItem *a, size_t pos, size_t size) {
    HeapItem t = a[pos];
    while (true) {
        size_t l = left(pos);
        size_t r = right(pos);
        size_t min_pos = pos;
        uint64_t min_val = t.val;
        if (l < size && a[l].val < min_val) {
            min_pos = l;
            min_val = a[l].val;
        }
        if (r < size && a[r].val < min_val) {
            min_pos = r;
            min_val = a[r].val;
        }
        if (min_pos == pos)
            break;
        a[pos] = a[min_pos];
        *a[pos].ref = pos;
        pos = min_pos;
    }
    a[pos] = t;
    *a[pos].ref = pos;
}

void HeapItem::up(HeapItem *a, size_t pos) {
    HeapItem t = a[pos];
    while (pos > 0 && a[parent(pos)].val > t.val) {
        a[pos] = a[parent(pos)];
        *a[pos].ref = pos;
        pos = parent(pos);
    }
    a[pos] = t;
    *a[pos].ref = pos;
}

void HeapItem::update(HeapItem *a, size_t pos, size_t len) {
    if (pos > 0 && a[parent(pos)].val > a[pos].val) {
        up(a, pos);
    } else {
        down(a, pos, len);
    }
}

void HeapItem::erase(std::vector<HeapItem> &a, size_t pos) {
    a[pos] = a.back();
    a.pop_back();
    if (pos < a.size()) {
        update(a.data(), pos, a.size());
    }
}

void HeapItem::upsert(std::vector<HeapItem> &a, size_t pos, HeapItem t) {
    if (pos < a.size()) {
        a[pos] = t;
    }
    else {
        pos = a.size();
        a.push_back(t);
    }
    update(a.data(), pos, a.size());
}



