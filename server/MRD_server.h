//
// Created by korot on 03/02/2025.
//

#ifndef MRD_SERVER_H
#define MRD_SERVER_H

#include <map>
#include <vector>
#include "Buffer.h"
#include "MRD_hashtable.h"
#include "MRD_utility.h"
#include <>
namespace MRD {

    struct Entry{
        HNode node;
        std::string key;
        std::string val;
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
    };



    class Server {
    public:
        struct Response {
            //Response status
            enum {
                RESP_OK = 0,
                RESP_ERR = 1,
                RESP_NX = 2, // key not found
            };
            uint32_t status = RESP_OK;
            std::vector<uint8_t> data;
        };

        static void init();

        static int main_loop();

    private:
        static inline int fd;
        static inline std::vector<Conn* > fd2conn;
        static HashMap g_data;

        // static std::map<std::string, std::string> g_data;

        static int32_t parse_req(const uint8_t *&data, size_t size, std::vector<std::string> &out);

        static size_t do_request(std::vector<std::string> &cmd, Buffer &out);

        static void response_end(Buffer &out, uint32_t rv);

        static void response_begin(Buffer &out);

        static bool try_one_request(Conn *conn);

        static void handle_write(Conn *conn);

        static void handle_read(Conn *conn);

        static uint32_t do_get(std::vector<std::string> &cmd, Buffer &out);
        static uint32_t do_set(std::vector<std::string> &cmd, Buffer &out);
        static uint32_t do_del(std::vector<std::string> &cmd, Buffer &out);
    };
}
#endif //MRD_SERVER_H
