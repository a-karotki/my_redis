//
// Created by korot on 09/03/2025.
//
#include <gtest/gtest.h>
#include "MRD_utility.h"
#include "MRD_buffer.h"

using namespace MRD;
namespace {
    //Uses TLV-encoding
    class OutTest : public testing::Test {
    protected:
        OutTest() {
            b1 = new Buffer(2048);
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
        static uint64_t get_int(const uint8_t* tag_ptr) {
            uint64_t res;
            memcpy(&res, tag_ptr + 1, 8);
            return res;
        }
        Buffer* b1;
    };
    TEST_F(OutTest, Nil) {
        out_nil(*b1);
        EXPECT_EQ(get_tag(b1->data_begin), TAG_NIL);
    }
    TEST_F(OutTest, Str) {
        const std::string str = "Hello, world";
        out_str(*b1, str.data(), str.length());
        uint8_t tag = get_tag(b1->data_begin);
        EXPECT_EQ(tag, TAG_STR);
        uint32_t len = get_len(b1->data_begin);
        ASSERT_EQ(len, str.length());
        std::string res = get_str(b1->data_begin, len);
        EXPECT_EQ(res, str);

    }
    TEST_F(OutTest, EmptyStr) {
        const std::string str{};
        out_str(*b1, str.data(), str.length());
        uint8_t tag = get_tag(b1->data_begin);
        EXPECT_EQ(tag, TAG_STR);
        uint32_t len = get_len(b1->data_begin);
        ASSERT_EQ(len, str.length());
        std::string res = get_str(b1->data_begin, len);
        EXPECT_EQ(res, str);

    }
    TEST_F(OutTest, LargeStr) {
        const std::string str(13000, 'x');
        out_str(*b1, str.data(), str.length());
        uint8_t tag = get_tag(b1->data_begin);
        EXPECT_EQ(tag, TAG_STR);
        uint32_t len = get_len(b1->data_begin);
        ASSERT_EQ(len, str.length());
        std::string res = get_str(b1->data_begin, len);
        EXPECT_EQ(res, str);

    }
    TEST_F(OutTest, Int) {
        int val = 231321;
        out_int(*b1, val);
        EXPECT_EQ(get_tag(b1->data_begin), TAG_INT);
        EXPECT_EQ(get_int(b1->data_begin), val);

    }
    TEST_F(OutTest, Err) {
        std::string msg = "TEST Output";
        out_err(*b1, ERR_BAD_TYPE ,msg);
        uint8_t tag = get_tag(b1->data_begin);
        EXPECT_EQ(tag, TAG_ERR);
        tag = get_tag(b1->data_begin + 1);
        EXPECT_EQ(tag, ERR_BAD_TYPE);
        uint32_t len = get_len(b1->data_begin + 1);
        ASSERT_EQ(len, msg.length());
        std::string res = get_str(b1->data_begin + 1, len);
        EXPECT_EQ(res, msg);
    }
    TEST_F(OutTest, ArrOfNil) {
        out_arr(*b1, 10);
        for (int i = 0; i != 10; ++i) {
            out_int(*b1, i);
        }
        EXPECT_EQ(get_tag(b1->data_begin), TAG_ARR);
        EXPECT_EQ(get_len(b1->data_begin), 10);

        uint8_t *p = b1->data_begin + 5;
        int i = 0;
        while (p < b1->data_end) {
            EXPECT_EQ(get_tag(p), TAG_INT);
            EXPECT_EQ(get_int(p), i++);
            p += 9;;
        }
    }
    TEST_F(OutTest, ArrOfStr) {
        out_arr(*b1, 10);
        for (int i = 0; i != 10; ++i) {
            out_str(*b1, std::to_string(i).data(), 1);
        }
        EXPECT_EQ(get_tag(b1->data_begin), TAG_ARR);
        EXPECT_EQ(get_len(b1->data_begin), 10);

        uint8_t *p = b1->data_begin + 5;
        int i = 0;
        while (p < b1->data_end) {
            EXPECT_EQ(get_tag(p), TAG_STR);
            uint32_t len = get_len(p);
            ASSERT_EQ(len, 1);
            std::string res = get_str(p, len);
            ASSERT_EQ(res, std::to_string(i++).data());
            p += 1 + 4 + len;
        }
    }
    TEST_F(OutTest, ArrOfStr_long) {
        out_arr(*b1, 10);
        for (int i = 0; i != 1000; ++i) {
            std::string s(i, 'x');
            out_str(*b1, s.data(), i);
        }
        EXPECT_EQ(get_tag(b1->data_begin), TAG_ARR);
        EXPECT_EQ(get_len(b1->data_begin), 10);

        uint8_t *p = b1->data_begin + 5;
        int i = 0;
        while (p < b1->data_end) {
            EXPECT_EQ(get_tag(p), TAG_STR);
            uint32_t len = get_len(p);
            ASSERT_EQ(len, i);
            std::string res = get_str(p, len);
            std::string s(i++, 'x');
            ASSERT_EQ(res, s);
            p += 1 + 4 + len;
        }
    }
    TEST_F(OutTest, Consume) {
        for (int i = 0; i != 1000; ++i) {
            out_int(*b1, i);
        }
        int i = 0;
        while (!b1->empty()) {
            EXPECT_EQ(get_tag(b1->data_begin), TAG_INT);
            EXPECT_EQ(get_int(b1->data_begin), i++);
            b1->consume(9);
        }
    }
}