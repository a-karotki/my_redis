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
    static int32_t send_req(int fd, const std::vector<std::string> &cmd) {
        uint32_t len = 4;
        for (const std::string &s : cmd) {
            len += 4 + s.size();
        }
        if (len > MAX_MSG) {
            return -1;
        }

        char wbuf[4 + MAX_MSG];
        memcpy(&wbuf[0], &len, 4);  // assume little endian
        uint32_t n = cmd.size();
        memcpy(&wbuf[4], &n, 4);
        size_t cur = 8;
        for (const std::string &s : cmd) {
            uint32_t p = (uint32_t)s.size();
            memcpy(&wbuf[cur], &p, 4);
            memcpy(&wbuf[cur + 4], s.data(), s.size());
            cur += 4 + s.size();
        }
        return write_all(fd, wbuf, 4 + len);
    }

    static int32_t read_res(int fd) {
        // 4 bytes header
        char rbuf[4 + MAX_MSG + 1];
        errno = 0;
        int32_t err = read_full(fd, rbuf, 4);
        if (err) {
            if (errno == 0) {
                msg("EOF");
            } else {
                msg("read() error");
            }
            return err;
        }

        uint32_t len = 0;
        memcpy(&len, rbuf, 4);  // assume little endian
        if (len > MAX_MSG) {
            msg("too long");
            return -1;
        }

        // reply body
        err = read_full(fd, &rbuf[4], len);
        if (err) {
            msg("read() error");
            return err;
        }

        // print the result
        uint32_t rescode = 0;
        if (len < 4) {
            msg("bad response");
            return -1;
        }
        memcpy(&rescode, &rbuf[4], 4);
        printf("server says: [%u] %.*s\n", rescode, len - 4, &rbuf[8]);
        return 0;
    }

    int main(int argc, char **argv) {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            die("socket()");
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
        int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
        // if (rv) {
        //     die("connect");
        // }

        std::vector<std::string> cmd{};
        for (int i = 1; i < argc; ++i) {
            cmd.push_back(argv[i]);
        }
        int32_t err = send_req(fd, cmd);
        // int32_t err = 1;
        if (err) {
            goto L_DONE;
        }
        err = read_res(fd);
        if (err) {
            goto L_DONE;
        }

        L_DONE:
            close(fd);
        return 0;
    }
}
int main(int argc, char **argv) {
    MRD::main(argc, argv);
}
