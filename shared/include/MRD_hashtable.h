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
        HNode** tab = nullptr; // array of slots
        size_t mask = 0;    // power of 2 array size, 2^n - 1
        size_t size = 0;    // number of keys

        void init(size_t n);
        void insert(HNode *node);

        HTab() = default; //zombie
        ~HTab();
        HTab(const HTab& other) = delete;
        HTab& operator=(HTab& rhs) = delete;
        HTab(HTab&& other) noexcept;
        HTab& operator=(HTab&& rhs) noexcept;

        HNode **lookup(HNode &key, bool (&eq)(HNode &, HNode &)) const;
        HNode *detach(HNode **from);
        bool foreach(bool (*f) (HNode*, void *), void* arg) const;
        void clear();
        friend void swap(HTab& lhs, HTab& rhs) noexcept;
    };


    class HashMap {
    public:
        HNode *erase(HNode& key, bool (&eq)(HNode&, HNode&));
        void   clear();
        size_t size() const;
        void insert(HNode *node);
        HNode *lookup(HNode &key, bool (&eq)(HNode&, HNode&)) const;
        void foreach(bool (*f) (HNode*, void*), void* arg);
        friend void swap(HashMap& lhs, HashMap& rhs) noexcept;
    private:
        const size_t k_rehashing_work = 128;
        const size_t k_max_load_factor = 8;
        void help_rehashing();

        void trigger_rehashing();



        HTab newer;
        HTab older;
        size_t migrate_pos = 0;
    };
    struct HKey {
        HNode node;
        const char *name = nullptr;
        size_t len = 0;
    };
}



#endif //MRD_HASTABLE_H
