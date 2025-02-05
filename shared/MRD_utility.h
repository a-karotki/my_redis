#ifndef MRD_UTILITY_H
#define MRD_UTILITY_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <vector>
#include <assert.h>

namespace MRD{

    const size_t MAX_MSG = 32 << 20;
    static void msg(const char *msg) {
        fprintf(stderr, "%s\n", msg);
    }

    static void die(const char *msg) {
        int err = errno;
        fprintf(stderr, "[%d] %s\n", err, msg);
        abort();
    }
    static int32_t read_all(int fd, uint8_t *buf, size_t n) {
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

    static int32_t write_all(int fd, const uint8_t *buf, size_t n) {
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
    static void buf_append(std::vector<uint8_t> &buf, const uint8_t *data, size_t len) {
        buf.insert(buf.end(), data, data + len);
    }
    static void buf_consume(std::vector<uint8_t> &buf, size_t n) {
        buf.erase(buf.begin(), buf.begin() + n);
    }
}

#endif
