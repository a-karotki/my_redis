#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <MRD_utility.h>
#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <iostream>
#include <memory>

#include "Buffer.h"
#include "MRD_server.h"

using namespace MRD;
// int Server::fd = -1;
static void fd_set_nb(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    if (errno) {
        die("fcntl error");
    }
}
static Conn *handle_accept(int fd) {
    struct sockaddr_in client_addr = {};
    socklen_t socklen = sizeof(client_addr);
    int connfd = accept(fd, (struct sockaddr *)&client_addr, &socklen);
    if (connfd < 0) {
        return nullptr;
    }
    uint32_t ip = client_addr.sin_addr.s_addr;
    fprintf(stderr, "new client from %u.%u.%u.%u:%u\n",
           ip & 255, (ip >> 8) & 255, (ip >> 16) & 255, ip >> 24,
           ntohs(client_addr.sin_port)
    );
    fd_set_nb(connfd);
    Conn* conn = new Conn();
    conn->fd = connfd;
    conn->want_read = true;
    return conn;
}

bool Server::try_one_request(Conn* conn) {
    if (conn->incoming.data_length < 4) {
        return false;
    }
    uint32_t len = 0;
    memcpy(&len, conn->incoming.data_begin, 4);
    if (len > MAX_MSG) {
        msg("message too long");
        conn->want_close = true;
        return false;
    }
    if (4 + len > conn->incoming.data_length) {
        return false;
    }
    const uint8_t *request = &conn->incoming.data_begin[4];
    std::cout << "len:" << len << " data:" << std::string((const char*)request, len > 100 ? 100 : len) << std::endl;
    conn->outgoing.append((const uint8_t*)&len, 4);
    conn->outgoing.append(request, len);
    conn->incoming.consume(4 + len);
    return true;
}

void Server::handle_write(Conn* conn) {
    ssize_t rv = write(conn->fd, conn->outgoing.data_begin, conn->outgoing.data_length);
    if (rv <= 0 && errno != EAGAIN) {
        return;
    }
    if (rv <= 0) {
        conn->want_close = true;
        return;
    }
    conn->outgoing.consume((size_t)rv);
    if (conn->outgoing.empty()) {
        conn->want_read = true;
        conn->want_write = false;
    }
}

void Server::handle_read(Conn* conn) {
    std::unique_ptr<uint8_t> buf(new uint8_t[MAX_MSG]);
    ssize_t rv = read(conn->fd, buf.get(), MAX_MSG);
    if (rv < 0) {
        conn->want_close = true;
        return;
    }
    if (rv == 0) {
        if (conn->incoming.empty()) {
            msg("client closed");
        } else {
            msg("unexpected EOF");
        }
        conn->want_close = true;
        return;
    }
    conn->incoming.append(buf.get(), static_cast<size_t>(rv));
    while (try_one_request(conn));
    if (!conn->outgoing.empty()) {
        conn->want_read = false;
        conn->want_write = true;
        return handle_write(conn);
    }
}

int Server::main_loop() {

    std::vector<struct pollfd> poll_args;
    while (true) {
        poll_args.clear();
        struct pollfd server_pfd = {fd, POLLIN, 0};
        poll_args.push_back(server_pfd);
        for (Conn* conn : fd2conn) {
            if (!conn) {
                continue;
            }
            struct pollfd pfd = {conn->fd, POLLERR, 0};
            if (conn->want_read) {
                pfd.events |= POLLIN;
            }
            if (conn->want_write) {
                pfd.events |= POLLOUT;
            }
            poll_args.push_back(pfd);
        }
        int rv = poll(poll_args.data(), (nfds_t) poll_args.size(), -1);
        if (rv < 0 && errno == EINTR) {
            continue;
        }
        if (rv < 0) {
            die("poll()");
        }
        if (poll_args[0].revents) {
            if (Conn* conn = handle_accept(fd)) {
                if (fd2conn.size() <= (size_t) conn->fd) {
                    fd2conn.resize(conn->fd + 1);
                }
                fd2conn[conn->fd] = conn;
            }
        }
        for (size_t i = 1; i < poll_args.size(); ++i) {
           uint32_t ready = poll_args[i].revents;
            Conn* conn = fd2conn[poll_args[i].fd];
            if (ready & POLLIN) {
                handle_read(conn);
            }
            if (ready & POLLOUT) {
                handle_write(conn);
            }
            if (ready & POLLERR || conn->want_close) {
                (void)close(conn->fd);
                fd2conn[conn->fd] = nullptr;
                delete conn;
            }
        }
    }
    return 0;
}

void Server::init() {
    Server::fd = -1;
    std::cout << "Starting server...\n" << std::endl;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        die("socket()");
    }

    // this is needed for most server applications
    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    // bind
    struct sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);    // wildcard address 0.0.0.0
    int rv = bind(fd, (const sockaddr *)&addr, sizeof(addr));
    if (rv) {
        die("bind()");
    }

    // listen
    rv = listen(fd, SOMAXCONN);
    if (rv) {
        die("listen()");
    }
}
int main() {
    Server::init();
    Server::main_loop();
}
