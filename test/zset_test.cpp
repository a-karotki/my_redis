//
// Created by korot on 13/03/2025.
//
#include <gtest/gtest.h>
#include "MRD_zset.h"
#include <vector>
// #include "MRD_utility.h"
namespace {
    struct Entry {
        std::string name;
        double score;
    };

    class ZSetTest : public testing::Test {
    protected:
        ZSetTest() {
            for (size_t i = 1; i <= val_count; ++i) {
                std::string name = std::to_string(i);
                double score = static_cast<double>(std::rand()) / static_cast<double>(i);
                set.insert(name.data(), name.length(), score);
                entries.push_back(Entry{name, score});
            }
        }
    public:
        size_t val_count = 1000;
        MRD::ZSet set;
        std::vector<Entry> entries;
    };

    //run with valgrind
    TEST_F(ZSetTest, Dtor) {
        using namespace MRD;
        ZSet *zset = new ZSet();
        for (size_t i = 1; i <= val_count; ++i) {
            std::string name = std::to_string(i);
            double score = static_cast<double>(std::rand()) / static_cast<double>(i);
            zset->insert(name.data(), name.length(), score);
        }
        delete zset;
    }

    TEST_F(ZSetTest, CopyCtor) {
        using namespace MRD;
        ZSet zset(set);
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = zset.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
            zset.rem(name);
        }
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = set.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
        }
    }
    TEST_F(ZSetTest, CopyAssign) {
        using namespace MRD;
        ZSet zset = set;
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = zset.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
            zset.rem(name);
        }
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = set.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
        }
    }
    TEST_F(ZSetTest, MoveCtor) {
        using namespace MRD;
        ZSet zset(std::move(set));
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = zset.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
            zset.rem(name);
        }
    }
    TEST_F(ZSetTest, MoveAssign) {
        using namespace MRD;
        ZSet zset = std::move(set);
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = zset.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
            zset.rem(name);
        }
    }
    TEST_F(ZSetTest, Lookup) {
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = set.lookup(name.data(), name.length());
            ASSERT_EQ(entries[i].name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
        }
    }

    TEST_F(ZSetTest, Del) {
        std::set<size_t> used{};
        for (size_t i = 0; i < 10; ++i) {
            size_t index = std::rand() % (val_count - i);
            while (used.contains(index))
                index = std::rand() % (val_count - i);
            used.insert(index);
            std::string name = entries[index].name;
            MRD::ZNode *found = set.lookup(name.data(), name.length());
            ASSERT_EQ(entries[index].name, found->name);
            ASSERT_DOUBLE_EQ(entries[index].score, found->score);
            set.rem(name);
            found = set.lookup(name.data(), name.length());
            EXPECT_EQ(found, nullptr);
        }
    }

    TEST_F(ZSetTest, Update) {
        for (int i = static_cast<int>(val_count) - 1; i >= 0; --i) {
            std::string name = entries[i].name;
            MRD::ZNode *found = set.lookup(name.data(), name.length());
            ASSERT_EQ(name, found->name);
            EXPECT_DOUBLE_EQ(entries[i].score, found->score);
            set.update(found, i);
            found = set.lookup(name.data(), name.length());
            ASSERT_EQ(name, found->name);
            EXPECT_DOUBLE_EQ(found->score, i);
            set.insert(name.data(), name.length(), -i);
            found = set.lookup(name.data(), name.length());
            ASSERT_EQ(name, found->name);
            EXPECT_DOUBLE_EQ(found->score, -i);
        }
    }
    bool less(Entry &l, Entry &r){
        if (l.score != r.score)
            return l.score < r.score;
        return  0 > l.name.compare(r.name);
    }
    TEST_F(ZSetTest, Rank) {
        Entry *min = &entries[0];
        Entry *max = &entries[0];
        for (int i = 0; i < val_count; ++i) {
            if (less(entries[i], *min)) {
                min = &entries[i];
            }
            if (!less(entries[i], *max)) {
                max = &entries[i];
            }
        }
        ASSERT_GE(max->score, min->score);
        size_t rank_max = set.rank(max->name);
        size_t rank_min = set.rank(min->name);
        ASSERT_GE(rank_max, rank_min);
        ASSERT_EQ(rank_min, 1);
        ASSERT_EQ(rank_max, val_count);
    }

    TEST_F(ZSetTest, SeekGE) {
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            double score = entries[i].score;
            MRD::ZNode *node = set.seekge(score, name.data(), name.length());
            EXPECT_EQ(node->name, name);
            EXPECT_DOUBLE_EQ(node->score, score);
            name = entries[i].name;
            score = entries[i].score - 1;
            node = set.seekge(score, name.data(), name.length());
            EXPECT_GE(node->score, score);
            EXPECT_LE(std::abs(node->score - score), 1);
        }
    }

    TEST_F(ZSetTest, SeekLE) {
        for (size_t i = 0; i != val_count; ++i) {
            std::string name = entries[i].name;
            double score = entries[i].score;
            MRD::ZNode *node = set.seekle(score, name.data(), name.length());
            EXPECT_EQ(node->name, name);
            EXPECT_DOUBLE_EQ(node->score, score);
            name = entries[i].name;
            score = entries[i].score + 1;
            node = set.seekle(score, name.data(), name.length());
            EXPECT_LE(node->score, score);
            EXPECT_LE(std::abs(node->score - score), 1);
        }
    }
}