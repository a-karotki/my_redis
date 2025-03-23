//
// Created by korot on 03/02/2025.
//

#ifndef MRD_SERVER_H
#define MRD_SERVER_H

#define MAX_CMDS 500

#include <map>
#include "MRD_DList.h"
#include <variant>
#include <vector>
#include "MRD_buffer.h"
#include "MRD_hashtable.h"
#include "MRD_utility.h"
#include "MRD_zset.h"
#include <ctime>
#include "MRD_heap.h"

namespace MRD {

    enum uin32_t{
        E_INIT = 0,
        E_STR = 1,
        E_SET = 2,
    };

    struct Entry {
        struct HNode node;  // hashtable node
        std::string key;
        uint32_t type = 0;
        size_t heap_idx = -1;

        std::variant<
            std::string, //KV value
            ZSet//set
        > val;
    public:
        static Entry* EntryKV(const std::string& name , const std::string& val_) {
            auto* res = new Entry(name, E_STR);
            res->val = std::string(val_);
            return res;
        }
        static Entry* EntryS(const std::string& name) {
            auto *e = new Entry(name, E_SET);
            return e;
        }
    private:
        Entry(const std::string& name ,uint32_t type) : node{nullptr, str_hash(name)} {
            key = name;
            this->type = type;
            if (type == E_STR) {
                val = std::string();
            }
            if (type == E_SET) {
                val.emplace<ZSet>();
            }
        }
    };
    struct LookupKey {  // for lookup only
        HNode node;
        std::string key;
    };
    static bool key_eq(HNode &lhs, HNode &rhs) {
        const struct LookupKey *lk = container_of(&lhs, struct LookupKey, node);
        const struct LookupKey *re = container_of(&rhs, struct LookupKey, node);
        return lk->key == re->key;
    }
    static bool entry_eq(HNode &lhs, HNode &rhs) {
        const struct Entry *le = container_of(&lhs, struct Entry, node);
        const struct Entry *re = container_of(&rhs, struct Entry, node);
        return le->key == re->key;
    }
    struct Conn {
        int fd = -1;
        // application's intention, for the event loop
        bool want_read = false;
        bool want_write = false;
        bool want_close = false;
        // buffered input and output
        Buffer incoming{};  // data to be parsed by the application
        Buffer outgoing{};  // responses generated by the application
        //timer
        uint64_t last_active_ms = 0;
        DList idle_node{};
    };



    class Server {
    public:
        static void init(int argc, char **argv);

        /*
         * Expects to receive data in format:
         *  (uint32_t)data_length (uint32_t)word_count (uint8_t)TAG_STR (uint32_t)str_len (uint8_t*)string
         * Responds in format:
         *  (uint32_t)data_length (uint8_t)TAG (type)val_matching_the_tag
         */
        static int main_loop();

        static void end_run();
    private:
        static inline int fd;
        static inline std::vector<Conn* > fd2conn;
        static HashMap g_data;
        static DList idle_list;
        static std::vector<HeapItem> ttl_heap;
        static bool is_running;

        static constexpr uint64_t IDLE_TIMEOUT_MS = 300000;
        static constexpr uint64_t DEFAULT_TTL_MS = 900000;
        static constexpr size_t MAX_WORKS = 2000;
        // static std::map<std::string, std::string> g_data;

        static Conn *handle_accept(int fd);

        static uint64_t next_timer_ms();

        static void conn_destroy(Conn * conn);

        static void entry_del(Entry * ent);

        static void process_timers();

        static void entry_set_ttl(Entry* ent, int64_t ttl_ms);

        //NOTE: handles only string tag
        static int32_t parse_req(const uint8_t *&data, size_t size, std::vector<std::string> &out);

        static size_t do_request(std::vector<std::string> &cmd, Buffer &out);

        static void response_end(Buffer &out, uint32_t rv);

        static void response_begin(Buffer &out);

        static bool try_one_request(Conn *conn);

        static void handle_write(Conn *conn);

        static void handle_read(Conn *conn);

        static Entry *data_lookup(std::string &name);

        static ZSet *expect_zset(Entry *entry);
        //GET key
        static uint32_t do_get(std::vector<std::string> &cmd, Buffer &out);
        //SET key val
        static uint32_t do_set(std::vector<std::string> &cmd, Buffer &out);
        //DEL key
        static uint32_t do_del(std::vector<std::string> &cmd, Buffer &out);
        //ZADD key score name
        static uint32_t do_zadd(std::vector<std::string> &cmd, Buffer &out);
        //ZREM key name
        static uint32_t do_zrem(std::vector<std::string> &cmd, Buffer &out);

        //ZQUERYGE key score name offset limit
        static uint32_t do_zqueryge(std::vector<std::string> &cmd, Buffer &out);

        //ZQYERYLE key score name offset limit
        static uint32_t do_zqueryle(std::vector<std::string> &cmd, Buffer &out);

        //ZRANK key name
        static uint32_t do_zrank(std::vector<std::string> &cmd, Buffer &out);

        //ZCOUNT key min_score max_score
        static uint32_t do_zcount(std::vector<std::string> &cmd, Buffer &out);
        //EXPIRE name time_ms (-1 to remove expiration)
        static uint32_t do_expire(std::vector<std::string> &cmd, Buffer &out);
        //TTL name (doesn't refresh ttl)
        static uint32_t do_ttl(std::vector<std::string> &cmd, Buffer &out);

        static uint32_t do_echo(std::vector<std::string> &cmd, Buffer &out);
    };
}
#endif //MRD_SERVER_H
