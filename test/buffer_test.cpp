//
// Created by korot on 06/03/2025.
//
#include "Buffer.h"
#include <gtest/gtest.h>
using namespace MRD;

TEST(BufferTest, Constructor) {
    Buffer buf{128};
    EXPECT_EQ(buf.buf_size, 128);
    EXPECT_TRUE(buf.empty());
    EXPECT_EQ(buf.buffer_begin + 128, buf.buffer_end);
}
TEST(BufferTest, Append) {
    Buffer buf{128};
    uint8_t test_data[] = {1, 2, 3, 4, 5};
    buf.append(test_data, sizeof(test_data));
    EXPECT_EQ(buf.data_length, 5);
    EXPECT_EQ(buf.data_begin[0], 1);
    EXPECT_EQ(buf.data_begin, buf.buffer_begin);
    EXPECT_EQ(buf.data_end, buf.data_begin + 5);
    EXPECT_FALSE(buf.empty());
}
TEST(BufferTest, Consume) {
    Buffer buf{128};
    uint8_t test_data[] = {1, 2, 3, 4, 5};
    buf.append(test_data, sizeof(test_data));
    ASSERT_EQ(buf.data_begin[2], 3);
    ASSERT_EQ(buf.data_length, 5);
    buf.consume(2);
    EXPECT_EQ(buf.data_length, 3);
    EXPECT_EQ(buf.data_begin[0], 3);
}
TEST(BufferTest, Resize) {
    Buffer buf{128};
    uint8_t test_data[] = {1, 2, 3, 4, 5};
    buf.append(test_data, sizeof(test_data));
    ASSERT_EQ(buf.buf_size, 128);
    ASSERT_EQ(buf.data_length, 5);
    buf.resize(128);
    EXPECT_GE(buf.buf_size, 256);
    EXPECT_EQ(buf.data_length, 5);
}
TEST(BufferTest, VectorHandling) {
    Buffer buf{128};
    std::vector<uint8_t> test_data = {1, 2, 3, 4, 5};
    buf.append(test_data);
    EXPECT_EQ(buf.data_length, 5);
    EXPECT_EQ(buf.data_begin[0], 1);
    EXPECT_EQ(buf.data_begin, buf.buffer_begin);
    EXPECT_EQ(buf.data_end, buf.data_begin + 5);
}
TEST(BufferTest, AutomaticResize) {
    Buffer buf{16};
    std::vector<uint8_t> test_data(64, 1);
    buf.append(test_data);
    EXPECT_GE(buf.buf_size, 64);
    EXPECT_EQ(buf.data_length, 64);
    for (uint8_t* i = buf.data_begin; i != buf.data_end; ++i) {
        EXPECT_EQ(*i, 1);
    }
    std::vector<uint8_t> test_data2(32, 1);
    buf.append(test_data2);
    EXPECT_GE(buf.buf_size, 96);
    EXPECT_EQ(buf.data_length, 96);
    EXPECT_EQ(buf.data_end, buf.data_begin + 96);
    for (uint8_t* i = buf.data_begin; i != buf.data_end; ++i) {
        EXPECT_EQ(*i, 1);
    }
}
TEST(BufferTest, ConsumptionOutOfBounds) {
    Buffer buf{32};
    std::vector<uint8_t> test_data(32, 1);
    buf.append(test_data);
    buf.consume(128);
    EXPECT_EQ(buf.data_length, 0);
    EXPECT_EQ(buf.buf_size, 32);
    std::vector<uint8_t> test_data2(128, 32);
    buf.append(test_data2);
    for (uint8_t* i = buf.data_begin; i != buf.data_end; ++i) {
        EXPECT_EQ(*i, 32);
    }
}
TEST(BufferTest, NullAppend) {
    Buffer buf{128};
    uint8_t test_data[] = {1, 2, 3, 4, 5};
    buf.append(test_data, 2);
    ASSERT_EQ(buf.data_length, 2);
    auto prev_end = buf.data_end;
    auto prev_len = buf.data_length;
    buf.append(test_data, 0);
    EXPECT_EQ(buf.data_length, prev_len);
    EXPECT_EQ(buf.data_end, prev_end);
}