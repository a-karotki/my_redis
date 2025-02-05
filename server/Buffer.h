//
// Created by korot on 03/02/2025.
//

#ifndef BUFFER_H
#define BUFFER_H
#include <cstdint>
#include <cstring>
#include <MRD_utility.h>
namespace MRD {
    struct Buffer {
    public:
        uint8_t *buffer_begin;
        uint8_t *buffer_end;
        uint8_t *data_begin;
        uint8_t *data_end;
        size_t data_length;
        size_t buf_size;

        Buffer(size_t size = 64 * MRD::MAX_MSG) {
            buffer_begin = new uint8_t[size];
            buffer_end = buffer_begin + size;
            data_begin = buffer_begin;
            data_end = buffer_begin;
            data_length = 0;
            buf_size = size;
        }
        ~Buffer() {
            delete[] buffer_begin;
        }
        void consume(size_t n) {
            data_begin += n;
            data_length -= n;
            if (data_begin > buffer_end) {
                data_begin = buffer_end;
                data_length = 0;
            }
        }
        void append(const uint8_t* data, const size_t len) {
            if (len > buf_size) {
                resize(buf_size - (len + data_length));
            }
            else if (data_begin + len > buffer_end) {
                memmove(buffer_begin, data_begin, data_length);
                data_begin = buffer_begin;
                data_end = data_begin + data_length;
            }
            if (data_end + len > buffer_end) {
                resize(len - (buffer_end - data_end));;
            }
            memcpy(data_end, data, len);
            data_length += len;
            data_end += len;
        }
        void append(const std::vector<uint8_t>& data, const size_t len) {
            append(data.data(), len);
        }
        void append(const Buffer& src, const size_t len) {
            append(src.buffer_begin, len);
        }
        void resize(size_t size) {
            uint8_t* new_buff = new uint8_t[buf_size + size];
            memcpy(new_buff, data_begin, data_length);
            delete[] buffer_begin;
            buffer_begin = new_buff;
            data_begin = buffer_begin;
            data_end = data_begin + data_length;
            buffer_end = buffer_begin + buf_size + size;
            buf_size += size;
        }
        bool empty() {
            return data_length == 0;
        }
    };
}

#endif //BUFFER_H
