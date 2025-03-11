//
// Created by korot on 10/03/2025.
//

#ifndef MRD_AVL_H
#define MRD_AVL_H
#include <cassert>
#include <MRD_hashtable.h>
#include <string>
#include "MRD_utility.h"
namespace MRD{
    struct AVLNode{
        AVLNode* parent;
        AVLNode* left;
        AVLNode* right;
        uint32_t height; //subtree hight
        uint32_t count; //subtree size

        void init() {
            left = right = parent = nullptr;
            height = 1;
            count = 1;
        }
        void update() {
            height = std::max(h(left), h(right)) + 1;
            count = 1 + cnt(left) + cnt(right);
        }
        static AVLNode *del(AVLNode *node);

        static AVLNode *fix(AVLNode* node);

        static uint32_t h(const AVLNode* node);

        static uint32_t cnt(const AVLNode* node);
    protected:

        static AVLNode *rot_left(AVLNode *node);

        static AVLNode *rot_right(AVLNode *node);

        static AVLNode *fix_left(AVLNode *node);

        static AVLNode *fix_right(AVLNode *node);

        static AVLNode *del_easy(AVLNode *node);
    };

}


#endif //MRD_AVL_H
