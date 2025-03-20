//
// Created by korot on 20/03/2025.
//

#include "MRD_DList.h"
using namespace MRD;

DList::DList() {
    prev = next = this;
}

bool DList::empty(const DList *node) {
    return node->next == node;
}

void DList::detach(DList *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

void DList::insert_before(DList *node, DList *target) {
    node->prev = target->prev;
    target->prev->next = node;
    target->prev = node;
    node->next = target;
}

