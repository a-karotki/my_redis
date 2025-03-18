//
// Created by korot on 16/02/2025.
//
#include "MRD_hashtable.h"
#include <gtest/gtest.h>
using namespace MRD;
namespace {
    struct Entry{
        HNode node;
        std::string key;
        std::string val;
    };
    struct LookupKey {  // for lookup only
        HNode node;
        std::string key;
    };
    static bool key_eq(HNode &lhs, HNode &rhs) {
        const struct LookupKey *lk = container_of(&lhs, struct LookupKey, node);
        const struct LookupKey *re = container_of(&rhs, struct LookupKey, node);
        return lk->key == re->key;
    }
    static bool entry_eq(HNode &lhs, HNode &rhs) {
        const struct Entry *le = container_of(&lhs, struct Entry, node);
        const struct Entry *re = container_of(&rhs, struct Entry, node);
        return le->key == re->key;
    }
    TEST(HashTab, Swap) {
        HTab h1{};
        HTab h2{};
        h1.init(100);
        h2.init(100);
        for (int i = 0; i != 100; ++i) {
            Entry *e1 = new Entry();
            e1->key = std::to_string(i);
            e1->val = std::to_string(i);
            e1->node.hcode = str_hash(e1->key);
            Entry *e2 = new Entry();
            e2->key = std::to_string(i);
            e2->val = std::to_string(-i);
            e2->node.hcode = str_hash(e2->key);
            h1.insert(&e1->node);
            h2.insert(&e2->node);
            LookupKey lk1;
            LookupKey lk2;
            lk1.key = lk2.key = std::to_string(i);
            lk1.node.hcode = lk2.node.hcode = str_hash(lk1.key);
            HNode* found1 = *h1.lookup(lk1.node, key_eq);
            const std::string& val = container_of(found1, Entry, node)->val;
            ASSERT_EQ(val, std::to_string(i));
            HNode* found2 = *h2.lookup(lk2.node, key_eq);
            const std::string& val2 = container_of(found2, Entry, node)->val;
            ASSERT_EQ(val2, std::to_string(-i));
        }
        swap(h1, h2);
        // HNode** temp = h2.tab;
        // h2.tab = h1.tab;
        // h1.tab = temp;
        for (int i = 0; i != 100; ++i) {
            LookupKey lk1;
            LookupKey lk2;
            lk1.key = lk2.key = std::to_string(i);
            lk1.node.hcode = lk2.node.hcode = str_hash(lk1.key);
            HNode* found1 = *h1.lookup(lk1.node, key_eq);
            const std::string& val = container_of(found1, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(-i));
            HNode* found2 = *h2.lookup(lk2.node, key_eq);
            const std::string& val2 = container_of(found2, Entry, node)->val;
            EXPECT_EQ(val2, std::to_string(i));
        }
    }

    TEST(HashTab, Destructor) {
        HTab *h1 = new HTab();
        h1->init(100);
        delete h1;
    }

    TEST(HashTab, MoveAssignmet) {
        HTab h1{};
        h1.init(100);
        for (int i = 0; i != 100; ++i) {
            Entry *e1 = new Entry();
            e1->key = std::to_string(i);
            e1->val = std::to_string(i);
            e1->node.hcode = str_hash(e1->key);
            h1.insert(&e1->node);
        }
        HTab h2 = std::move(h1);
        for (int i = 0; i != 100; ++i) {
            LookupKey lk1;
            lk1.key = std::to_string(i);
            lk1.node.hcode = str_hash(lk1.key);
            HNode* found1 = *h2.lookup(lk1.node, key_eq);
            const std::string& val = container_of(found1, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(i));
        }
    }

    TEST(HashTab, MoveConstructor) {
        HTab h1{};
        h1.init(100);
        for (int i = 0; i != 100; ++i) {
            Entry *e1 = new Entry();
            e1->key = std::to_string(i);
            e1->val = std::to_string(i);
            e1->node.hcode = str_hash(e1->key);
            h1.insert(&e1->node);
        }
        HTab h2(std::move(h1));
        for (int i = 0; i != 100; ++i) {
            LookupKey lk1;
            lk1.key = std::to_string(i);
            lk1.node.hcode = str_hash(lk1.key);
            HNode* found1 = *h2.lookup(lk1.node, key_eq);
            const std::string& val = container_of(found1, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(i));
        }
    }

    HashMap map;
    TEST(HashmapTest, EmptyLookup) {
        LookupKey lk;
        lk.key = "Hello";
        lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
        HNode* node = map.lookup(lk.node, key_eq);
        ASSERT_EQ(node, nullptr);
    }
    TEST(HashmapTest, Insertion) {
        auto *e = new Entry();
        e->key = "Hello";
        e->val = "World";
        e->node.hcode = str_hash(e->key.data(), e->key.length());
        map.insert(&e->node);
        LookupKey lk;
        lk.key = "Hello";
        lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
        HNode* node = map.lookup(lk.node, key_eq);
        const std::string &val = container_of(node, Entry, node)->val;
        ASSERT_EQ(val, "World");
        delete e;
        map.clear();
    }
    TEST(HashmapTest, Erasing) {
        auto *e = new Entry();
        e->key = "Hello";
        e->val = "World";
        e->node.hcode = str_hash(e->key.data(), e->key.length());
        map.insert(&e->node);
        LookupKey lk;
        lk.key = "Hello";
        lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
        HNode* node = map.erase(lk.node, key_eq);
        const std::string &val = container_of(node, Entry, node)->val;
        ASSERT_EQ(val, "World");
        node = map.lookup(lk.node, key_eq);
        ASSERT_EQ(node, nullptr);
        delete e;
        map.clear();
    }
    TEST(HashmapTest, CollisionsHandling) {
        auto *e1 = new Entry();
        e1->key = "Hello";
        e1->val = "World";
        e1->node.hcode = 1;
        auto *e2 = new Entry();
        e2->key = "Bye";
        e2->val = "Byee";
        e2->node.hcode = 5;
        map.insert(&e1->node);
        map.insert(&e2->node);
        LookupKey lk1;
        lk1.key = "Hello";
        lk1.node.hcode = 1;
        HNode* node1 = map.lookup(lk1.node, key_eq);
        const std::string &val1 = container_of(node1, Entry, node)->val;
        ASSERT_EQ(val1, "World");
        LookupKey lk2;
        lk2.key = "Bye";
        lk2.node.hcode = 5;
        HNode* node2 = map.lookup(lk2.node, key_eq);
        const std::string &val2 = container_of(node2, Entry, node)->val;
        ASSERT_EQ(val2, "Byee");
        EXPECT_EQ(node1, node2->next);
        delete e1;
        delete e2;
        map.clear();
    }
    TEST(HashmapTest, ResizingBehaviour) {
        for (int i = 0; i < 1000; ++i) {
            auto e = new Entry();
            e->key = std::to_string(i);
            e->val = std::to_string(-i);
            e->node.hcode = str_hash(e->key.data(), e->key.length());
            map.insert(&e->node);
        }
        LookupKey lk;
        for (int i = 0; i < 1000; ++i) {
            lk.key = std::to_string(i);
            lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
            HNode* found = map.lookup(lk.node, key_eq);
            ASSERT_NE(found, nullptr);
            const std::string& val = container_of(found, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(-i));
        }
        for (int i = 999; i > -1; --i) {
            lk.key = std::to_string(i);
            lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
            HNode* node = map.erase(lk.node, key_eq);
            const std::string& val = container_of(node, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(-i));
            delete container_of(node, Entry, node);
        }
        map.clear();
    }
    TEST(HashmapTest, ResizingLarger) {
        for (int i = 0; i <= 100000; ++i) {
            auto e = new Entry();
            e->key = std::to_string(i);
            e->val = std::to_string(-i);
            e->node.hcode = str_hash(e->key.data(), e->key.length());
            map.insert(&e->node);
        }
        LookupKey lk;
        for (int i = 100000; i > -1; --i) {
            lk.key = std::to_string(i);
            lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
            HNode* node = map.erase(lk.node, key_eq);
            const std::string& val = container_of(node, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(-i));
            delete container_of(node, Entry, node);
        }
        map.clear();
    }
    TEST(HashmapTest, SizeCorectness) {
        for (int i = 0; i < 100; ++i) {
            auto e = new Entry();
            e->key = std::to_string(i);
            e->val = std::to_string(i);
            e->node.hcode = str_hash(e->key.data(), e->key.length());
            map.insert(&e->node);
        }
        LookupKey lk;
        EXPECT_EQ(map.size(), 100);
        for (int i = 0; i < 100; ++i) {
            lk.key = std::to_string(i);
            lk.node.hcode = str_hash(lk.key.data(), lk.key.length());
            HNode* node = map.erase(lk.node, key_eq);
            const std::string& val = container_of(node, Entry, node)->val;
            EXPECT_EQ(val, std::to_string(i));
            delete container_of(node, Entry, node);
            EXPECT_EQ(map.size(), 99 - i);
        }
        map.clear();
        EXPECT_EQ(map.size(), 0);
    }
    static bool cb_keys(HNode *node, void *arg) {
        Buffer &out = *static_cast<Buffer *>(arg);
        const std::string &key = container_of(node, Entry, node)->key;
        out_str(out, key.data(), key.size());
        return true;
    }
    static uint8_t get_tag(const uint8_t* ptr) {
        return *ptr;
    }
    static uint32_t get_len(const uint8_t* tag_ptr) {
        uint32_t res;
        memcpy(&res, tag_ptr + 1, 4);
        return res;
    }
    static std::string get_str(const uint8_t* tag_ptr, size_t n) {
        auto *str_ptr = reinterpret_cast<const char*>(tag_ptr + 5); //not UB
        return std::string{str_ptr, n};
    }
    TEST(HashmapTest, Foreach) {
        std::set<std::string> s1{};
        for (int i = 0; i < 100; ++i) {
            auto e = new Entry();
            e->key = std::to_string(i);
            e->val = std::to_string(i);
            e->node.hcode = str_hash(e->key.data(), e->key.length());
            map.insert(&e->node);
            s1.insert(std::to_string(i));
        }
        Buffer b{4096};
        // out_arr(b, map.size());
        map.foreach(cb_keys, &b);
        int i = 0;
        std::set<std::string> s2{};
        while (!b.empty()) {
            EXPECT_EQ(get_tag(b.data_begin), TAG_STR);
            uint32_t len = get_len(b.data_begin);
            std::string res = get_str(b.data_begin, len);
            s2.insert(res);
            b.consume(1 + 4 + len);
        }
        EXPECT_EQ(s1, s2);
    }
}