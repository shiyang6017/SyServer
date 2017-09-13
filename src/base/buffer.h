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
} sy_buffer_t;

static inline void sy_buffer_init(sy_buffer_t* buffer) {
    buffer->begin = buffer->data;
    buffer->end = buffer->data;
    buffer->limit = buffer->data + BUF_SIZE;
}

static inline void sy_buffer_clear(sy_buffer_t* buffer) {
    sy_buffer_init(buffer);
}

static inline int sy_buffer_size(sy_buffer_t* buffer) {
    return buffer->end - buffer->begin;
}

static inline int sy_buffer_append_u8(sy_buffer_t* buffer, uint8_t val) {
    if (buffer->limit <= buffer->end)
        return 0;
    *buffer->end++ = val;
    return 1;
}

static inline int sy_buffer_append_u16le(sy_buffer_t* buffer, uint16_t val) {
    int len = sy_buffer_append_u8(buffer, val & 0xff);
    return len + sy_buffer_append_u8(buffer, (val >> 8) & 0xff);
}

static inline int sy_buffer_append_u32le(sy_buffer_t* buffer, uint32_t val) {
    int len = sy_buffer_append_u16le(buffer, val & 0xffff);
    return len + sy_buffer_append_u16le(buffer, (val >> 16) & 0xffff);
}

static inline void sy_buffer_discard_parsed(sy_buffer_t* buffer) {
    int size = sy_buffer_size(buffer);
    memmove(buffer->data, buffer->begin, size);
    
    buffer->begin = buffer->data;
    buffer->end = buffer->begin + size;
}

static inline bool sy_buffer_full(sy_buffer_t* buffer) {
    return buffer->end >= buffer->limit;
}

static inline int sy_buffer_margin(sy_buffer_t* buffer) {
    return buffer->limit - buffer->end;
}

static inline void sy_buffer_append(sy_buffer_t* des, sy_buffer_t* src) {
    assert(des->data != src->data);
    assert(sy_buffer_margin(des) >= sy_buffer_size(src));
    
    memcpy(des->end, src->begin, sy_buffer_size(src));
    des->end += sy_buffer_size(src);
}

int sy_buffer_recv(sy_buffer_t* buffer, int fd);
int sy_buffer_send(sy_buffer_t* buffer, int fd);
int sy_buffer_append_string(sy_buffer_t* buffer, const sy_string_t* str);
int sy_buffer_print(sy_buffer_t* buffer, const char* format, ...);
void sy_print_buffer(sy_buffer_t* buffer);

#define sy_buffer_append_cstring(buffer, cstr) sy_buffer_append_string(buffer, &SY_STRING(cstr))

#endif
