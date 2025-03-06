//
// Created by korot on 12/02/2025.
//

#ifndef MRD_HASTABLE_H
#define MRD_HASTABLE_H
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>
#include "MRD_utility.h"
#define container_of(ptr, T, member) ((T *)( (char *)ptr - offsetof(T, member) ))
namespace MRD{
    // hashtable node, should be embedded into the payload


    struct HNode {
        HNode *next = nullptr;
        uint64_t hcode = 0;
    };
    // a simple fixed-sized hashtable
    struct HTab {
        HNode** tab; // array of slots
        size_t mask = 0;    // power of 2 array size, 2^n - 1
        size_t size = 0;    // number of keys

        void init(size_t n);
        void insert(HNode *node);

        HNode **lookup(HNode &key, bool (&eq)(HNode &, HNode &)) const;
        HNode *detach(HNode **from);
        void clear();
    };


    class HashMap {
    public:
        HNode *erase(HNode& key, bool (&eq)(HNode&, HNode&));
        void   clear();
        size_t size() const;
        void insert(HNode *node);
        HNode *lookup(HNode &key, bool (&eq)(HNode&, HNode&)) const;
    private:
        const size_t k_rehashing_work = 128;
        const size_t k_max_load_factor = 8;
        void help_rehashing();

        void trigger_rehashing();



        HTab newer;
        HTab older;
        size_t migrate_pos = 0;
    };
}



#endif //MRD_HASTABLE_H
