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

#include "MRD_buffer.h"
#include "MRD_server.h"

#include <iomanip>
#include <map>
#include <MRD_heap.h>
using namespace MRD;
// int Server::fd = -1;
DList Server::idle_list{};
std::vector<HeapItem> Server::ttl_heap{};
static void fd_set_nb(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    if (errno) {
        die("fcntl error");
    }
}

Conn *Server::handle_accept(int fd) {
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
    conn->last_active_ms = get_monotonic_msec();
    DList::insert_before(&conn->idle_node, &idle_list);
    return conn;
}

uint64_t Server::next_timer_ms() {
    if (DList::empty(&idle_list))
        return -1;
    uint64_t now_ms = get_monotonic_msec();
    uint64_t next_ms = -1;
    if (!DList::empty(&idle_list)){
        Conn *conn = container_of(idle_list.next, Conn,  idle_node);
        next_ms = conn->last_active_ms + IDLE_TIMEOUT_MS;
    }
    if (!ttl_heap.empty() && ttl_heap[0].val < next_ms) {
        next_ms = ttl_heap[0].val;
    }
    if (next_ms == -1)
        return next_ms;
    if (next_ms <= now_ms)
        return 0;
    return next_ms - now_ms;
}

void Server::conn_destroy(Conn *conn) {
    (void)close(conn->fd);
    fd2conn[conn->fd] = nullptr;
    DList::detach(&conn->idle_node);
    delete conn;
}

void Server::entry_del(Entry *ent) {
    entry_set_ttl(ent, -1);
    delete ent;
}

void Server::process_timers() {
    uint64_t now_ms = get_monotonic_msec();
    while (!DList::empty(&idle_list)) {
        Conn *conn = container_of(idle_list.next, Conn, idle_node);
        uint64_t next_ms = conn->last_active_ms + IDLE_TIMEOUT_MS;
        if (next_ms >= now_ms)
            break;
        std::cout << "Removing idle connection: " << conn->fd << std::endl;
        conn_destroy(conn);
    }
    size_t works = 0;
    while (!ttl_heap.empty() && ttl_heap[0].val < now_ms && works++ < MAX_WORKS) {
        Entry *ent = container_of(ttl_heap[0].ref, Entry, heap_idx);
        std::cout << "Removing expired entry: " << ent->key << std::endl;
        g_data.erase(ent->node, entry_eq);
        entry_del(ent);
    }
}

void Server::entry_set_ttl(Entry *ent, int64_t ttl_ms) {
    if (ttl_ms < 0 && ent->heap_idx != static_cast<size_t>(-1)) {
        HeapItem::erase(ttl_heap, ent->heap_idx);
        ent->heap_idx = -1;
    }
    else if (ttl_ms >= 0) {
        uint64_t expire_at = get_monotonic_msec() + ttl_ms;
        HeapItem item = {expire_at, &ent->heap_idx};
        HeapItem::upsert(ttl_heap, ent->heap_idx, item);
    }
}


int32_t Server::parse_req(const uint8_t *&data, size_t size, std::vector<std::string> &out) {
    const auto *end = data + size;
    uint8_t tag = -1;
    uint32_t nstr = 0;
    if (!read_u32(data, end, nstr)) {
        return -1;
    }
    if (nstr > MAX_CMDS) {
        return -1;
    }
    while (out.size() < nstr) {
        uint32_t len = 0;
        uint8_t tag = -1;
        if (!read_u8(data,end, tag) || tag != TAG_STR) {
            return -1;
        }
        if (!read_u32(data, end, len)) {
            return -1;
        }
        out.push_back(std::string());
        if (!read_str(data, end, len, out.back())) {
            return -1;
        }
    }
    if (data != end) {
        return -1;
    }
    return 0;
}

// std::map<std::string, std::string> Server::g_data;
HashMap Server::g_data;

size_t Server::do_request(std::vector<std::string> &cmd, Buffer &out) {
    for (auto& x : cmd[0])
        x = static_cast<char>(toupper(x));

    if (cmd.size() == 2 && cmd[0] == "GET") {
        return do_get(cmd, out);
    }
    if (cmd.size() == 3 && cmd[0] == "SET") {
        return do_set(cmd, out);
    }
    if (cmd.size() == 2 && cmd[0] == "DEL") {
        return do_del(cmd, out);
    }
    if (cmd.size() == 4 && cmd[0] == "ZADD") {
        return do_zadd(cmd, out);
    }
    if (cmd.size() == 3 && cmd[0] == "ZREM") {
        return do_zrem(cmd, out);
    }
    if (cmd.size() == 6 && cmd[0] == "ZQUERYGE") {
        return do_zqueryge(cmd, out);
    }
    if (cmd.size() == 6 && cmd[0] == "ZQUERYLE") {
        return do_zqueryle(cmd, out);
    }
    if (cmd.size() == 4 && cmd[0] == "ZCOUNT") {
        return do_zcount(cmd, out);
    }
    if (cmd.size() == 3 && cmd[0] == "ZRANK") {
        return do_zrank(cmd, out);
    }
    if (cmd.size() == 3 && cmd[0] == "EXPIRE") {
        return do_expire(cmd, out);
    }
    if (cmd.size() == 2 && cmd[0] == "TTL") {
        return do_ttl(cmd, out);
    }
    return out_err(out, ERR_NOT_FOUND, "command not found");
    return 0;
}
void Server::response_begin(Buffer& out) {
    out.append_u32(0);
}

void Server::response_end(Buffer& out, const uint32_t rv) {
    if (rv > MAX_MSG) {
        out.consume(out.data_length);
        out.append_u32(0);
        uint32_t val = out_err(out, ERR_OUT_OF_RANGE, "response is too long");
        auto ptr = out.data_end - val - 4;
        memcpy(ptr, &val, 4);
    }
    auto len_ptr = out.data_end - rv - 4;
    if (len_ptr < out.data_begin ) {
        out.consume(out.data_length);
        out.append_u32(0);
        uint32_t val = out_err(out, ERR_UNEXPECTED, "unexpected error in error");
        auto ptr = out.data_end - val - 4;
        memcpy(ptr, &val, 4);
    }
    memcpy(len_ptr, &rv, 4);
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

    std::vector<std::string> cmd;
    if (parse_req(request, len, cmd) < 0) {
        msg("bad request");
        conn->want_close = true;
        return false;
    }
    response_begin(conn->outgoing);
    const uint32_t rv = do_request(cmd, conn->outgoing);
    response_end(conn->outgoing, rv);
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
    uint8_t buf[64 * 1024];
    ssize_t rv = read(conn->fd, buf, sizeof(buf));
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
    conn->incoming.append(buf, static_cast<size_t>(rv));
    while (try_one_request(conn));
    if (!conn->outgoing.empty()) {
        conn->want_read = false;
        conn->want_write = true;
        return handle_write(conn);
    }
}
// throws std::invalid_argument if object wasn't found
// throws std::runtime_error if found Entry is of a wrong type
Entry *Server::data_lookup(std::string &name) {
    LookupKey key;
    key.key = name;
    key.node.hcode = str_hash(key.key.data(), key.key.size());
    HNode *found = g_data.lookup(key.node, key_eq);
    return found ? container_of(found, Entry, node) : nullptr;
}

ZSet *Server::expect_zset(Entry *entry) {
    if (!entry)
        return nullptr;
    if (entry->type != E_SET)
        return nullptr;
    return &std::get<ZSet>(entry->val);
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
        int32_t timeout_ms = static_cast<int32_t>(next_timer_ms());
        int rv = poll(poll_args.data(), (nfds_t) poll_args.size(), timeout_ms);
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
            if (ready == 0) {
                continue;
            }
            Conn* conn = fd2conn[poll_args[i].fd];
            conn->last_active_ms = get_monotonic_msec();
            DList::detach(&conn->idle_node);
            DList::insert_before(&idle_list, &conn->idle_node);
            if (ready & POLLIN) {
                handle_read(conn);
            }
            if (ready & POLLOUT) {
                handle_write(conn);
            }
            if (ready & POLLERR || conn->want_close) {
                conn_destroy(conn);
            }
        }
        process_timers();
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
//get key
uint32_t Server::do_get(std::vector<std::string> &cmd, Buffer &out) {
    const Entry* entry = data_lookup(cmd[1]);
    if (!entry)
        return out_err(out, ERR_NOT_FOUND, "get");
    if (entry->type != E_STR)
        return out_err(out, ERR_BAD_TYPE, "get: expected KV-pair");
    const auto& val = std::get<std::string>(entry->val);
    assert(val.size() <= MAX_MSG);
    return out_str(out, val.data(), val.length());
}
//set key val
uint32_t Server::do_set(std::vector<std::string> &cmd, Buffer &out) {
    Entry* entry = data_lookup(cmd[1]);
    if (entry){
        if (entry->type != E_STR)
            return out_err(out, ERR_BAD_TYPE, "set: wrong data type, expected KV-pair");
        entry->val = cmd[2];
    }
    else {
        entry = Entry::EntryKV(cmd[1], cmd[2]);
        g_data.insert(&entry->node);
    }
    entry_set_ttl(entry, DEFAULT_TTL_MS);
    return out_nil(out);
}
//del key
uint32_t Server::do_del(std::vector<std::string> &cmd, Buffer &out) {
    LookupKey key;
    key.key = cmd[1];
    key.node.hcode = str_hash(key.key.data(), key.key.size());
    HNode *found = g_data.erase(key.node, key_eq);
    if (found) {
        // deallocate the pair
        Entry *e = container_of(found, Entry, node);
        entry_del(e);
        return out_int(out, 1); //number of removed elements
    }
    return out_err(out,ERR_NOT_FOUND, "del: given key does not exist");
}
//zadd key score name
uint32_t Server::do_zadd(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    double score;
    std::string name;
    try {
        score = std::stod(cmd[2]);
        name = cmd[3];
    }
    catch (std::invalid_argument& exc) {
        return out_err(out, ERR_BAD_TYPE, exc.what());
    }
    catch (std::exception& exc) {
        return out_err(out, ERR_UNEXPECTED, exc.what());
    }
    if (!e) {
        Entry *entry = Entry::EntryS(cmd[1]);
        entry_set_ttl(entry, DEFAULT_TTL_MS);
        g_data.insert(&entry->node);
        std::get<ZSet>(entry->val).insert(name.data(), name.length(), score);
        return out_nil(out);
    }
    else {
        if (e->type != E_SET)
            return out_err(out, ERR_BAD_TYPE, "zadd: expexcted set Entry");
        std::get<ZSet>(e->val).insert(name.data(), name.length(), score);
        entry_set_ttl(e, DEFAULT_TTL_MS);
        return out_nil(out);
    }
}
//zrem key name
uint32_t Server::do_zrem(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "zrem");
    }
    if (e->type != E_SET) {
        return out_err(out, ERR_BAD_TYPE, "zrem");
    }
    ZSet &zset = std::get<ZSet>(e->val);
    zset.rem(cmd[2]);
    return out_int(out, 1); //number of remove elements
}

//TODO: move inside ZSet class
//zqueryge key score name offset limit
uint32_t Server::do_zqueryge(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "zqueryge");
    }
    if (e->type != E_SET) {
        return out_err(out, ERR_BAD_TYPE, "zqueryge");
    }
    ZSet &zset = std::get<ZSet>(e->val);

    double score;
    std::string name;
    int64_t offset;
    int64_t limit;
    try {
        score = std::stod(cmd[2]);
        name = cmd[3];
        offset = std::stol(cmd[4]);
        limit = std::stol(cmd[5]);
    }
    catch (std::invalid_argument& exc) {
        return out_err(out, ERR_BAD_TYPE, exc.what());
    }
    catch (std::exception& exc) {
        return out_err(out, ERR_UNEXPECTED, exc.what());
    }

    ZNode *znode = zset.seekge(score, name.data(), name.size());
    znode = ZNode::offset(znode, offset);
    size_t ctx = out_begin_arr(out);
    uint32_t n = 0;
    size_t b_written = 0;
    while (znode && n / 2 < limit) {
        b_written += out_str(out, znode->name.data(), znode->len);
        b_written += out_dbl(out, znode->score);
        znode = ZNode::offset(znode, +1);
        n += 2;
    }
    b_written += out_end_arr(out, ctx + b_written, n);
    return b_written;
}
//zqueryle key score name offset limit
uint32_t Server::do_zqueryle(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "zqueryge");
    }
    if (e->type != E_SET) {
        return out_err(out, ERR_BAD_TYPE, "zqueryge");
    }
    ZSet &zset = std::get<ZSet>(e->val);

    double score;
    std::string name;
    int64_t offset;
    int64_t limit;
    try {
        score = std::stod(cmd[2]);
        name = cmd[3];
        offset = std::stol(cmd[4]);
        limit = std::stol(cmd[5]);
    }
    catch (std::invalid_argument& exc) {
        return out_err(out, ERR_BAD_TYPE, exc.what());
    }
    catch (std::exception& exc) {
        return out_err(out, ERR_UNEXPECTED, exc.what());
    }

    ZNode *znode = zset.seekle(score, name.data(), name.size());
    znode = ZNode::offset(znode, offset);
    size_t ctx = out_begin_arr(out);
    int64_t n = 0;
    size_t b_written = 0;
    while (znode && n / 2 < limit) {
        b_written += out_str(out, znode->name.data(), znode->len);
        b_written += out_dbl(out, znode->score);
        znode = ZNode::offset(znode, -1);
        n += 2;
    }
    b_written += out_end_arr(out, ctx + b_written, static_cast<uint32_t>(n));
    return b_written;
}
//zrank key name
uint32_t Server::do_zrank(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "zqueryge");
    }
    if (e->type != E_SET) {
        return out_err(out, ERR_BAD_TYPE, "zqueryge");
    }
    ZSet &zset = std::get<ZSet>(e->val);

    std::string &name = cmd[2];
    const int64_t rank = zset.rank(name);

    entry_set_ttl(e, DEFAULT_TTL_MS);
    return out_int(out, rank);
}
//zcount key min_score max_score
uint32_t Server::do_zcount(std::vector<std::string> &cmd, Buffer &out){
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "zqueryge");
    }
    if (e->type != E_SET) {
        return out_err(out, ERR_BAD_TYPE, "zqueryge");
    }
    ZSet &zset = std::get<ZSet>(e->val);

    double min_score = stod(cmd[2]);
    double max_score = stod(cmd[3]);
    int64_t count = 0;
    ZNode *found = zset.seekge(min_score, nullptr, 0);
    while (found && found->score <= max_score) {
        ++count;
        AVLNode* node = AVLNode::offset(&found->tree_node, +1);
        found = container_of(node, ZNode, tree_node);
    }
    return out_int(out, count);
}
//EXPIRE name time_ms
uint32_t Server::do_expire(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e) {
        return out_err(out, ERR_NOT_FOUND, "expire");
    }
    int64_t ttl_ms;
    try {
        ttl_ms = std::stoi(cmd[2]);
    }
    catch (std::invalid_argument& exc) {
        return out_err(out, ERR_BAD_TYPE, exc.what());
    }
    catch (std::exception& exc) {
        return out_err(out, ERR_UNEXPECTED, exc.what());
    }
    entry_set_ttl(e, ttl_ms);
    return out_nil(out);
}
//TTL name
uint32_t Server::do_ttl(std::vector<std::string> &cmd, Buffer &out) {
    Entry *e = data_lookup(cmd[1]);
    if (!e)
        return out_err(out, ERR_NOT_FOUND, "ttl");
    return out_int(out, static_cast<int64_t>(ttl_heap[e->heap_idx].val));
}


int main(int argc, char** argv) {
    Server::init();
    Server::main_loop();
}
