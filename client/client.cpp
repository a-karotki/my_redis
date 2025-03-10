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
        uint32_t len = 0;
        Buffer b{};
        b.append_u32(len);
        len += out_arr(b, cmd.size());
        for (const std::string& s: cmd) {
            len += out_str(b, s);
        }
        if (len > MAX_MSG) {
            return -1;
        }
        *b.data_begin = len;
        return write_all(fd, b);
    }
    static int32_t print_response(const uint8_t *data, size_t size) {
    if (size < 1) {
        msg("bad response");
        return -1;
    }
    switch (data[0]) {
    case TAG_NIL:
        printf("(nil)\n");
        return 1;
    case TAG_ERR:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int32_t code = 0;
            uint32_t len = 0;
            memcpy(&code, &data[1], 4);
            memcpy(&len, &data[1 + 4], 4);
            if (size < 1 + 8 + len) {
                msg("bad response");
                return -1;
            }
            printf("(err) %d %.*s\n", code, len, &data[1 + 8]);
            return 1 + 8 + len;
        }
    case TAG_STR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            if (size < 1 + 4 + len) {
                msg("bad response");
                return -1;
            }
            printf("(str) %.*s\n", len, &data[1 + 4]);
            return 1 + 4 + len;
        }
    case TAG_INT:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            int64_t val = 0;
            memcpy(&val, &data[1], 8);
            printf("(int) %ld\n", val);
            return 1 + 8;
        }
    case TAG_DBL:
        if (size < 1 + 8) {
            msg("bad response");
            return -1;
        }
        {
            double val = 0;
            memcpy(&val, &data[1], 8);
            printf("(dbl) %g\n", val);
            return 1 + 8;
        }
    case TAG_ARR:
        if (size < 1 + 4) {
            msg("bad response");
            return -1;
        }
        {
            uint32_t len = 0;
            memcpy(&len, &data[1], 4);
            printf("(arr) len=%u\n", len);
            size_t arr_bytes = 1 + 4;
            for (uint32_t i = 0; i < len; ++i) {
                int32_t rv = print_response(&data[arr_bytes], size - arr_bytes);
                if (rv < 0) {
                    return rv;
                }
                arr_bytes += (size_t)rv;
            }
            printf("(arr) end\n");
            return (int32_t)arr_bytes;
        }
    default:
        msg("bad response");
        return -1;
    }
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
        int32_t rv = print_response((uint8_t *)&rbuf[4], len);
        if (rv > 0 && (uint32_t)rv != len) {
            msg("bad response");
            rv = -1;
        }
        return rv;
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
