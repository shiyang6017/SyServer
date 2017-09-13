#ifndef _SYSERVER_STRING_H_
#define _SYSERVER_STRING_H_

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char* data;
    int len;
} sy_string_t;

#define SY_STRING_INIT(cstr)   {cstr, sizeof(cstr) - 1}
#define SY_STRING(cstr)    (sy_string_t){cstr, sizeof(cstr) - 1}

static const sy_string_t SY_STRING_NULL = {NULL, 0};

static inline void sy_string_init(sy_string_t* str) {
    str->data = NULL;
    str->len = 0;
}

static inline char* sy_string_find(sy_string_t* str, char ch) {
    for (int i = 0; i < str->len; ++i)
        if (str->data[i] == ch)
            return &str->data[i];
    return NULL;
}

static inline sy_string_t sy_string_setto(char* cstr, int len) {
    return (sy_string_t){cstr, len};
}

static inline char* sy_string_end(sy_string_t* str) {
    return str->data + str->len;
}

static inline void sy_string_free(sy_string_t* str) {
    free(str->data);
}

int sy_string_print(const char* format, ...);
int sy_string_cmp(const sy_string_t* lhs, const sy_string_t* rhs);
bool sy_string_eq(const sy_string_t* lhs, const sy_string_t* rhs);

#endif
