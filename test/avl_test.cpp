//
// Created by korot on 10/03/2025.
//
#include "MRD_avl.h"
#include "MRD_utility.h"
#include <gtest/gtest.h>
using namespace MRD;
namespace {
    struct Data {
        AVLNode node;
        uint32_t val = 0;
    };

    struct Container {
        AVLNode *root = nullptr;
    };

    // class AVLTest : public testing::Test {
    //
    //
    // protected:
    //     AVLTest() {}
    //
    //
    // };
    static void add(Container &c, uint32_t val) {
        Data *data = new Data();    // allocate the data
        data->node = AVLNode();
        data->val = val;

        AVLNode *cur = nullptr;        // current node
        AVLNode **from = &c.root;   // the incoming pointer to the next node
        while (*from) {             // tree search
            cur = *from;
            uint32_t node_val = container_of(cur, Data, node)->val;
            from = (val < node_val) ? &cur->left : &cur->right;
        }
        *from = &data->node;        // attach the new node
        data->node.parent = cur;
        c.root = AVLNode::fix(&data->node);
    }

    static bool del(Container &c, uint32_t val) {
        AVLNode *cur = c.root;
        while (cur) {
            uint32_t node_val = container_of(cur, Data, node)->val;
            if (val == node_val) {
                break;
            }
            cur = val < node_val ? cur->left : cur->right;
        }
        if (!cur) {
            return false;
        }

        c.root = AVLNode::del(cur);
        delete container_of(cur, Data, node);
        return true;
    }

    static void avl_verify(AVLNode *parent, AVLNode *node) {
        if (!node) {
            return;
        }

        ASSERT_EQ(parent, node->parent);
        avl_verify(node, node->left);
        avl_verify(node, node->right);

        EXPECT_EQ(AVLNode::cnt(node), 1 + AVLNode::cnt(node->left) + AVLNode::cnt(node->right));

        uint32_t l = AVLNode::h(node->left);
        uint32_t r = AVLNode::h(node->right);
        EXPECT_TRUE(l == r || l == r - 1 || l - 1 == r);
        EXPECT_EQ(AVLNode::h(node), 1 + std::max(l, r));

        uint32_t val = container_of(node, Data, node)->val;
        if (node->left) {
            EXPECT_EQ(node->left->parent, node);
            EXPECT_LE(container_of(node->left, Data, node)->val, val);
        }
        if (node->right) {
            EXPECT_EQ(node->right->parent, node);
            EXPECT_GE(container_of(node->right, Data, node)->val, val);
        }
    }

    static void extract(AVLNode *node, std::multiset<uint32_t> &extracted) {
        if (!node) {
            return;
        }
        extract(node->left, extracted);
        extracted.insert(container_of(node, Data, node)->val);
        extract(node->right, extracted);
    }

    static void container_verify(Container &c, const std::multiset<uint32_t> &ref)
    {
        avl_verify(nullptr, c.root);
        EXPECT_EQ(AVLNode::cnt(c.root), ref.size());
        std::multiset<uint32_t> extracted;
        extract(c.root, extracted);
        EXPECT_EQ(extracted, ref);
    }
    static void dispose(Container &c) {
        while (c.root) {
            AVLNode *node = c.root;
            c.root = AVLNode::del(c.root);
            delete container_of(node, Data, node);
        }
    }

    TEST(AVLTest, Insert) {
        for (int i = 0; i != 100; ++i) {
            Container c;
            std::multiset<uint32_t> ref;
            for (int j = 0; j != 100; ++j) {
                if (i == j)
                    continue;
                add(c, j);
                ref.insert(j);
            }
            container_verify(c, ref);
            add(c, i);
            ref.insert(i);
            container_verify(c, ref);
            dispose(c);
        }
    }
    TEST(AVLTest, InsertDup) {
        for (int i = 0; i != 100; ++i) {
            Container c;
            std::multiset<uint32_t> ref;
            for (int j = 0; j != 100; ++j) {
                add(c, j);
                ref.insert(j);
            }
            container_verify(c, ref);
            add(c, i);
            ref.insert(i);
            container_verify(c, ref);
            dispose(c);
        }
    }
    TEST(AVLTest, InsertRand) {
        for (int i = 0; i != 100; ++i) {
            Container c;
            std::multiset<uint32_t> ref;
            for (int j = 1; j != 100; ++j) {
                uint32_t val = (uint32_t)rand() % (j * 100);
                add(c, val);
                ref.insert(val);
            }
            container_verify(c, ref);
            dispose(c);
        }
    }

    TEST(AVLTest, Remove) {
        for (int i = 0; i != 100; ++i) {
            Container c;
            std::multiset<uint32_t> ref;
            for (int j = 0; j != 100; ++j) {
                add(c, j);
                ref.insert(j);
            }
            container_verify(c, ref);
            EXPECT_TRUE(del(c, i));
            ref.erase(i);
            container_verify(c, ref);
            add(c, i * 3);
            ref.insert(i * 3);
            container_verify(c, ref);
            dispose(c);
        }
    }

    TEST(AVLTest, RemoveRand) {
        for (int i = 0; i != 100; ++i) {
            Container c;
            std::multiset<uint32_t> ref;
            std::vector<uint32_t> vals;
            for (int j = 1; j != 100; ++j) {
                uint32_t val = (uint32_t)rand() % (j * 100);
                add(c, val);
                ref.insert(val);
                vals.push_back(val);
            }
            container_verify(c, ref);
            for (auto x : vals) {
                ASSERT_TRUE(del(c, x));
                ref.erase(x);
            }
            container_verify(c, ref);
            for (int i = 0; i != vals.size() / 2; ++i) {
                add(c, vals[i]);
                ref.insert(vals[i]);
            }
            container_verify(c, ref);
            dispose(c);
        }
    }

    TEST(AVLTest, Offset) {
        Container c;
        for (uint32_t i = 0; i != 100; ++i) {
            add(c, i);
        }
        AVLNode *min = c.root;
        while (min->left) {
            min = min->left;
        }
        for (uint32_t i = 0; i != 100; ++i) {
            AVLNode* node =  AVLNode::offset(min, i);
            EXPECT_EQ(container_of(node, Data, node)->val, i);
            for (uint32_t j = 0; j != 100; ++j) {
                int64_t offset = static_cast<int64_t>(j) - static_cast<int64_t>(i);
                AVLNode *n2 = AVLNode::offset(node, offset);
                auto val = container_of(n2, Data, node)->val;
                EXPECT_EQ(val, j);
            }
            ASSERT_FALSE(AVLNode::offset(node, -i - 1));
            ASSERT_FALSE(AVLNode::offset(node, 100 - i));
        }
        dispose(c);
    }
}