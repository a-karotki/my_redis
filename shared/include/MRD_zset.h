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

        ZNode() = default;
        ZNode(const char* name_, size_t len_, double score_);
        // ~ZNode();
        static ZNode *offset(ZNode *node, int64_t offset);
    };
    struct ZSet {
        AVLNode *root = nullptr;
        HashMap hmap;

        ZNode *lookup(const char *name, size_t len) const;

        void del(ZNode* node);

        void update(ZNode * node, double score);

        bool insert(const char* name, size_t len, double score);

        ZNode *seekge(double score, const char* name, size_t len);

        ZNode *seekle(double score, const char* name, size_t len);

    private:
        void tree_insert(ZNode* node);

        bool tree_delete(ZNode *node);

        static bool hcmp(HNode &node_, HNode &key);

        static bool zless(AVLNode *lhs, AVLNode *rhs);

        static bool zless(AVLNode *lhs, double score, const char *name, size_t len);

        static bool zgreater(AVLNode *lhs, double score, const char *name, size_t len);

        static bool zeq(AVLNode *lhs, AVLNode *rhs);
    };
}



#endif //MRD_ZSET_H
