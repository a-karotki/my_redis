//
// Created by korot on 11/03/2025.
//

#ifndef MRD_ZSET_H
#define MRD_ZSET_H
#include "MRD_avl.h"
#include "MRD_hashtable.h"
namespace MRD {
    struct ZNode {
        AVLNode tree_node;
        HNode h_node;

        double score = 0;
        size_t len = 0;
        std::string name = std::string();
        ZNode(const char* name_, size_t len_, double score_);
        ZNode(const std::string& name, double score_);
        // ~ZNode();
        static ZNode *offset(ZNode *node, int64_t offset);
    };
    struct ZSet {
        AVLNode *root = nullptr;
        HashMap hmap;

        ZSet() : hmap(){}; //zombie
        ~ZSet();
        ZSet(const ZSet & rhs);
        ZSet(ZSet && other) noexcept;
        ZSet& operator=(ZSet rhs);
        ZSet& operator=(ZSet &&rhs) noexcept;

        bool rem(const std::string &name);
        void update(ZNode * node, double score);

        bool insert(const char* name, size_t len, double score);

        int64_t rank(const std::string &name) const;


        ZNode *seekge(double score, const char* name, size_t len);

        ZNode *seekle(double score, const char* name, size_t len);

        ZNode *lookup(const char *name, size_t len) const;
        friend void swap(ZSet &lhs, ZSet &rhs) noexcept;
    private:

        void del(ZNode* node);


        void tree_insert(ZNode* node);

        bool tree_delete(ZNode *node);

        void tree_dispose(AVLNode *node);

        static void set_cpy(AVLNode* tree_node, ZSet *to_cpy);

        static bool hcmp(HNode &node_, HNode &key);

        static bool zless(AVLNode *lhs, AVLNode *rhs);

        static bool zless(AVLNode *lhs, double score, const char *name, size_t len);

        static bool zgreater(AVLNode *lhs, double score, const char *name, size_t len);

        static bool zeq(AVLNode *lhs, AVLNode *rhs);
    };


}



#endif //MRD_ZSET_H
