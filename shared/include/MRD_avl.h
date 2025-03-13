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
        AVLNode();

        void update();

        static AVLNode *del(AVLNode *node);

        static AVLNode *fix(AVLNode* node);

        static uint32_t h(const AVLNode* node);

        static uint32_t cnt(const AVLNode* node);

        static AVLNode *successor(AVLNode * node);

        static AVLNode *predscessor(AVLNode * node);

        static int64_t rank(AVLNode *node);

        static AVLNode *offset(AVLNode *node, int64_t offset);
    protected:

        static AVLNode *rot_left(AVLNode *node);

        static AVLNode *rot_right(AVLNode *node);

        static AVLNode *fix_left(AVLNode *node);

        static AVLNode *fix_right(AVLNode *node);

        static AVLNode *del_easy(AVLNode *node);
    };
}


#endif //MRD_AVL_H
