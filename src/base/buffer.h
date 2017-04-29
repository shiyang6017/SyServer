#ifndef _SYSERVER_BUFFER_H_
#define _SYSERVER_BUFFER_H_

#include "string.h"
#include "util.h"

#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>


#define BUF_SIZE   2048

typedef struct {
    char* begin;
    char* end;
    char* limit;
    char data[BUF_SIZE + 1];
} buffer_t;

static inline void sy_buffer_init(buffer_t* buffer) {
    buffer->begin = buffer->data;
    buffer->end = buffer->data;
    buffer->limit = buffer->data + BUF_SIZE;
}

static inline void sy_buffer_clear(buffer_t* buffer) {
    buffer_init(buffer);
}

static inline int sy_buffer_size(buffer_t* buffer) {
    return buffer->end - buffer->begin;
}

static inline int sy_buffer_append_u8(buffer_t* buffer, uint8_t val) {
    if (buffer->limit <= buffer->end)
        return 0;
    *buffer->end++ = val;
    return 1;
}

static inline int sy_buffer_append_u16le(buffer_t* buffer, uint16_t val) {
    int len = buffer_append_u8(buffer, val & 0xff);
    return len + buffer_append_u8(buffer, (val >> 8) & 0xff);
}

static inline int sy_buffer_append_u32le(buffer_t* buffer, uint32_t val) {
    int len = buffer_append_u16le(buffer, val & 0xffff);
    return len + buffer_append_u16le(buffer, (val >> 16) & 0xffff);
}

static inline void sy_buffer_discard_parsed(buffer_t* buffer) {
    int size = buffer_size(buffer);
    memmove(buffer->data, buffer->begin, size);
    
    buffer->begin = buffer->data;
    buffer->end = buffer->begin + size;
}

static inline bool sy_buffer_full(buffer_t* buffer) {
    return buffer->end >= buffer->limit;
}

static inline int sy_buffer_margin(buffer_t* buffer) {
    return buffer->limit - buffer->end;
}

static inline void sy_buffer_append(buffer_t* des, buffer_t* src) {
    assert(des->data != src->data);
    assert(buffer_margin(des) >= buffer_size(src));
    
    memcpy(des->end, src->begin, buffer_size(src));
    des->end += buffer_size(src);
}

int sy_buffer_recv(buffer_t* buffer, int fd);
int sy_buffer_send(buffer_t* buffer, int fd);
int sy_buffer_append_string(buffer_t* buffer, const string_t* str);
int sy_buffer_print(buffer_t* buffer, const char* format, ...);
void sy_print_buffer(buffer_t* buffer);

#define sy_buffer_append_cstring(buffer, cstr) buffer_append_string(buffer, &STRING(cstr))

#endif
