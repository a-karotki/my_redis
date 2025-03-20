//
// Created by korot on 20/03/2025.
//

#ifndef MRD_HEAP_H
#define MRD_HEAP_H
#include <cstdint>
#include <vector>
#include <cstddef>


namespace MRD {
    //min heap
    struct HeapItem {
        uint64_t val;
        size_t *ref; //points to heap index in intrusive structure
        static size_t parent(size_t pos);
        static size_t left(size_t pos);
        static size_t right(size_t pos);

        static void down(HeapItem* a, size_t pos, size_t size);
        static void up(HeapItem* a, size_t pos);
        static void update(HeapItem* a, size_t pos, size_t len);
        static void erase(std::vector<HeapItem>& a, size_t pos);

        static void upsert(std::vector<HeapItem> & a, size_t pos, HeapItem t);
    };
}


#endif //MRD_HEAP_H
