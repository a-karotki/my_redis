#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <MRD_utility.h>
#include <vector>
#include <string>

namespace MRD {
    static int32_t read_res(int fd) {
        // 4 bytes header
        std::vector<uint8_t> rbuf;
        rbuf.resize(4);
        errno = 0;
        int32_t err = read_all(fd, &rbuf[0], 4);
        if (err) {
            if (errno == 0) {
                msg("EOF");
            } else {
                msg("read() error");
            }
            return err;
        }

        uint32_t len = 0;
        memcpy(&len, rbuf.data(), 4); // assume little endian
        if (len > MAX_MSG) {
            msg("too long");
            return -1;
        }

        // reply body
        rbuf.resize(4 + len);
        err = read_all(fd, &rbuf[4], len);
        if (err) {
            msg("read() error");
            return err;
        }

        // do something
        printf("len:%u data:%.*s\n", len, len < 100 ? len : 100, &rbuf[4]);
        return 0;
    }

    static int32_t send_req(int fd, const uint8_t *text, size_t len) {
        if (len > MAX_MSG) {
            return -1;
        }

        std::vector<uint8_t> wbuf;
        buf_append(wbuf, (const uint8_t *) &len, 4);
        buf_append(wbuf, text, len);
        return write_all(fd, wbuf.data(), wbuf.size());
    }

    int main() {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            die("socket()");
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK); // 127.0.0.1


        int rv = connect(fd, (const struct sockaddr *) &addr, sizeof(addr));
        if (rv) {
            die("connect");
        }

        std::vector<std::string> query_list = {
            "hello1", "hello2", "hello3",
            // a large message requires multiple event loop iterations
            std::string(MAX_MSG, 'z'),
            "hello5",
        };
        for (const std::string &s: query_list) {
            int32_t err = send_req(fd, (uint8_t *) s.data(), s.size());
            if (err) {
                goto L_DONE;
            }
        }
        for (size_t i = 0; i < query_list.size(); ++i) {
            int32_t err = read_res(fd);
            if (err) {
                goto L_DONE;
            }
        }

    L_DONE:
        close(fd);
        return 0;
    }
}

int main() {
    MRD::main();
}
