//
// Created by korot on 16/02/2025.
//
#include "MRD_hashtable.h"
#include <gtest/gtest.h>
using namespace MRD;
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
HashMap map;
TEST(HashmapTest, EmptyLookup) {LookupKey lk;
    lk.key = "Hello";
    lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
    HNode* node = map.lookup(lk.node, key_eq);
    ASSERT_EQ(node, nullptr);
}
TEST(HashmapTest, Insertion) {
    auto *e = new Entry();
    e->key = "Hello";
    e->val = "World";
    e->node.hcode = str_hash(reinterpret_cast<uint8_t*>(e->key.data()), e->key.length());
    map.insert(&e->node);
    LookupKey lk;
    lk.key = "Hello";
    lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
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
    e->node.hcode = str_hash(reinterpret_cast<uint8_t*>(e->key.data()), e->key.length());
    map.insert(&e->node);
    LookupKey lk;
    lk.key = "Hello";
    lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
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
        e->node.hcode = str_hash(reinterpret_cast<uint8_t*>(e->key.data()), e->key.length());
        map.insert(&e->node);
    }
    LookupKey lk;
    for (int i = 999; i > -1; --i) {
        lk.key = std::to_string(i);
        lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
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
        e->node.hcode = str_hash(reinterpret_cast<uint8_t*>(e->key.data()), e->key.length());
        map.insert(&e->node);
    }
    LookupKey lk;
    for (int i = 100000; i > -1; --i) {
        lk.key = std::to_string(i);
        lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
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
        e->node.hcode = str_hash(reinterpret_cast<uint8_t*>(e->key.data()), e->key.length());
        map.insert(&e->node);
    }
    LookupKey lk;
    EXPECT_EQ(map.size(), 100);
    for (int i = 0; i < 100; ++i) {
        lk.key = std::to_string(i);
        lk.node.hcode = str_hash(reinterpret_cast<uint8_t*>(lk.key.data()), lk.key.length());
        HNode* node = map.erase(lk.node, key_eq);
        const std::string& val = container_of(node, Entry, node)->val;
        EXPECT_EQ(val, std::to_string(i));
        delete container_of(node, Entry, node);
        EXPECT_EQ(map.size(), 99 - i);
    }
    map.clear();
    EXPECT_EQ(map.size(), 0);
}