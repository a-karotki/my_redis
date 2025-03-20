//
// Created by korot on 20/03/2025.
//

#ifndef DLIST_H
#define DLIST_H


namespace MRD {
    struct DList {
        DList *next = this;
        DList *prev = this;

        DList();
        static bool empty(const DList *node);
        static void detach(DList *node);
        //insert node before target;
        static void insert_before(DList *node, DList *target);
    };
}


#endif //DLIST_H
