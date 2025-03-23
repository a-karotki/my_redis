#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
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
        b.append_u32(cmd.size());
        len += 4;
        for (const std::string& s: cmd) {
            len += out_str(b, s);
        }
        if (len > MAX_MSG) {
            return -1;
        }
        memcpy(b.data_begin, &len, 4);
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
        return 0;
    case TAG_ERR:
        {
            if (size < 6) {
                msg("bad response");
                return -1;
            }
            uint_8t code;
            uint32_t len = 0;
            memcpy(&code, &data[1], 1);
            memcpy(&len, &data[2], 4);
            if (size < 1 + 1 + 4 + len) {
                msg("bad response");
                return -1;
            }
            std::cout << "(err) " << get_err(code) << " msg_len = " << len << ' ' << std::string_view(reinterpret_cast<const char*>(&data[6]), len) << std::endl;
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
            std::cout << "(str) len = " << len << ' ' << std::string_view(reinterpret_cast<const char*>(&data[5]), len) << std::endl;
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
            std::cout << "(int) " << val << std::endl;
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
            std::cout << "(dbl) "  << val << std::endl;
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
            std::cout << "(arr) len=" << len << std::endl;
            size_t arr_bytes = 1 + 4;
            for (uint32_t i = 0; i < len; ++i) {
                int32_t rv = print_response(&data[arr_bytes], size - arr_bytes);
                if (rv < 0) {
                    return rv;
                }
                arr_bytes += (size_t)rv;
            }
            std::cout << "(arr) end" << std::endl;
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
        /*if (rv > 0 && (uint32_t)rv != len) {
            msg("bad response");
            rv = -1;
        }*/
        return rv;
    }

    int main() {
        int fd = socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0) {
            die("socket()");
        }

        struct sockaddr_in addr = {};
        addr.sin_family = AF_INET;
        addr.sin_port = ntohs(1234);
        addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);  // 127.0.0.1
        int rv = connect(fd, (const struct sockaddr *)&addr, sizeof(addr));
        if (rv) {
            die("connect");
        }
        while (true) {
            std::string str;
            std::vector<std::string> cmd{};
            std::getline(std::cin, str, '\n');
            size_t pos = 0;
            if (str == "EXIT")
                break;
            if (str.empty())
                continue;
            while ((pos = str.find(' ')) != std::string::npos) {
                std::string token = str.substr(0, pos);
                if (token == " ")
                    continue;
                cmd.push_back(token);
                str.erase(0, pos + 1);
            }
            if (!str.empty())
                cmd.push_back(str);
            std::cout << "Sending: ";
            for (auto& str1: cmd) {
                std::cout << str1 << ' ';
            }
            std::cout << std::endl;
            int32_t err = send_req(fd, cmd);
            // int32_t err = 1;
            if (err) {
                std::cout << "server closed the connection" << std::endl;
                break;
            }
            err = read_res(fd);
            if (err < 0){
                std::cout << "server closed the connection" << std::endl;
                break;
            }
        }

        close(fd);
        return 0;
    }
}
int main() {
    MRD::main();
}
