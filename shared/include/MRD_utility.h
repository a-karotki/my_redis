#ifndef MRD_UTILITY_H
#define MRD_UTILITY_H

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <vector>
#include <cassert>
#include <iostream>
#include <string>
#include "MRD_buffer.h"
#include <time.h>
#ifndef DBL_SIZE
#define DBL_SIZE sizeof(double)
#endif

namespace MRD{

    static uint64_t get_monotonic_msec() {
        struct timespec tv = {0, 0};
        clock_gettime(CLOCK_MONOTONIC, &tv);
        return static_cast<uint64_t>(tv.tv_sec) * 1000 + tv.tv_nsec / 1000 / 1000;
    }

    enum uint_8t {
        TAG_NIL = 0,    // nil
        TAG_ERR = 1,    // error code + msg
        TAG_STR = 2,    // string
        TAG_INT = 3,    // int64
        TAG_DBL = 4,    // double
        TAG_ARR = 5,    // array
        ERR_BAD_TYPE = 0,
        ERR_NOT_FOUND = 1,
        ERR_OUT_OF_RANGE = 2,
        ERR_UNEXPECTED = 3,
    };
    static std::string get_err(uint_8t err) {
        switch (err) {
            case ERR_BAD_TYPE:
                return "ERR_BAD_TYPE";
            case ERR_NOT_FOUND:
                return "ERR_NOT_FOUND";
            case ERR_OUT_OF_RANGE:
                return "ERR_OUT_OF_RANGE";
            case ERR_UNEXPECTED:
                return "ERR_UNEXPECTED";
            default:
                return "Unknown error code";
        }
    }
    //return bytes written
    static uint32_t out_nil(Buffer &out) {
        out.append_u8(TAG_NIL);
        return 1;
    }
    static uint32_t out_str(Buffer &out, const char* s, size_t size) {
        out.append_u8(TAG_STR);
        out.append_u32(size);
        uint8_t interpreted[size];
        memcpy(interpreted, s, size);
        out.append(interpreted, size);
        return 1 + 4 + size;
    }
    static uint32_t out_str(Buffer& out, const std::string& s) {
        return out_str(out, s.data(), s.length());
    }
    static uint32_t out_int(Buffer &out, int64_t val) {
        out.append_u8(TAG_INT);
        out.append_i64(val);
        return 1 + 8;
    }
    static uint32_t out_dbl(Buffer &out, double val) {
        out.append_u8(TAG_DBL);
        out.append_dbl(val);
        return 1 + DBL_SIZE;
    }
    static uint32_t out_arr(Buffer &out, uint32_t n) {
        out.append_u8(TAG_ARR);
        out.append_u32(n);
        return 1 + 4;
    }
    static uint32_t out_begin_arr(Buffer &out) {
        return out_arr(out, 0) - 1; //bytes written without tag
    }
    /*
     *ctx - bytes written without tag
     *n - number of elements
     *fills the number of elements in the beginning of the array
     */
    static uint32_t out_end_arr(Buffer &out, uint32_t ctx, uint32_t n) {
        assert(*(out.data_end - ctx - 1) == TAG_ARR);
        memcpy(out.data_end- ctx, &n, 4);
        return 1 + 4;
    }
    static uint32_t out_err(Buffer &out, uint8_t err_typ, const std::string& msg) {
        out.append_u8(TAG_ERR);
        out.append_u8(err_typ);
        out.append_u32(msg.length());
        uint8_t interpreted[msg.length()];
        memcpy(interpreted, msg.data(), msg.length());
        out.append(interpreted, msg.length());
        return 1 + 1 + 4 + msg.length();
    }

    static bool read_u32(const uint8_t *&cur, const uint8_t *end, uint32_t &out) {
        if (cur + 4 > end) {
            return false;
        }
        memcpy(&out, cur, 4);
        cur += 4;
        return true;
    }
    static bool read_u8(const uint8_t *&cur, const uint8_t *end, uint8_t &out) {
        if (cur + 1 > end) {
            return false;
        }
        memcpy(&out, cur, 1);
        cur += 1;
        return true;
    }

    static bool read_str(const uint8_t *&cur, const uint8_t *end, size_t n, std::string &out) {
        if (cur + n > end) {
            return false;
        }
        out.assign(cur, cur + n);
        cur += n;
        return true;
    }

    constexpr size_t MAX_MSG = 1 << 20;
    static void msg(const char *msg) {
        std::cout << msg << std::endl;
    }

    static void die(const char *msg) {
        int err = errno;
        std::cerr << err << " " << msg << std::endl;
        abort();
    }
    static int32_t read_full(int fd, char *buf, size_t n) {
        while (n > 0) {
            ssize_t rv = read(fd, buf, n);
            if (rv <= 0) {
                return -1;  // error, or unexpected EOF
            }
            assert((size_t)rv <= n);
            n -= (size_t)rv;
            buf += rv;
        }
        return 0;
    }

    static int32_t write_all(int fd, const char *buf, size_t n) {
        while (n > 0) {
            ssize_t rv = write(fd, buf, n);
            if (rv <= 0) {
                return -1;  // error
            }
            assert((size_t)rv <= n);
            n -= (size_t)rv;
            buf += rv;
        }
        return 0;
    }
    static int32_t write_all(const int fd, const Buffer& buf) {
        const auto p = reinterpret_cast<const char *>(buf.data_begin); //not UB
        return write_all(fd, p, buf.data_length);
    }
    static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len) {
        buf.insert(buf.end(), data, data + len);
    }
    static void buf_consume(std::vector<uint8_t> &buf, size_t n) {
        buf.erase(buf.begin(), buf.begin() + n);
    }

    static uint64_t str_hash(const uint8_t *data, size_t len) {
        uint32_t h = 0x811C9DC5;
        for (size_t i = 0; i < len; i++) {
            h = (h + data[i]) * 0x01000193;
        }
        return h;
    }
    static uint64_t str_hash(const char* data, size_t len) {
        uint8_t interpreted[len];
        memcpy(interpreted, data, len);
        return str_hash(interpreted, len);
    }
    static uint64_t str_hash(const std::string& str) {
        return str_hash(str.data(), str.length());
    }
}

#endif
