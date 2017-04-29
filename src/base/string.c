#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <stdarg.h>
#include <assert.h>
/* ptr != NULL: err*/
int sy_string_print(const char* format, ...)
{
    int ret = 0;
    va_list args;
    va_start(args, format);
    const char* ptr;
    for (ptr = format; *ptr != 0; ++ptr) {
        if (ptr[0] == '%' && ptr[1] == '*' && (ptr[2] == 'S' || ptr[2] == 's')) {
            string_t* str = va_arg(args, string_t*);
            for (int j = 0; j < str->len; ++j) {
                printf("%c", str->data[j]);
            }
            ptr += 2;
        } else {
            ret += printf("%c", ptr[0]);
        }
    }
    va_end(args);
    return ret;
}
int sy_string_cmp(const string_t* lhs, const string_t* rhs) 
{
    if (lhs->len == rhs->len && lhs->data == rhs->data) {
        return 0;
    }
    if (lhs->data == NULL) {
        return -1;
    }
    if (rhs->data == NULL) {
        return 1;
    }
    const char* lptr = lhs->data, *rptr = rhs->data;
    for (; *lptr != '\0' && *rptr != '\0'; ++lptr, ++rptr) {
        if (*lptr != *rptr) {
            return *lptr > *rptr ? 1: - 1;
        }
    }
    if (*rptr == '\0' && *lptr == '\0') {
        return 0;
    }
    return *rptr == '\0' ? 1: -1;
}

bool sy_string_eq(const string_t* lhs, const string_t* rhs) {
    return string_cmp(lhs, rhs) == 0 ? true : false;
}

/*
int main() {
    string_t str1 = STRING_INIT("HELLO");
    string_t str2 = {0};
    printf("%d", string_cmp(&str1, &str2));
    printf("%d", string_eq(&str1, &str2));
    return 0;
}
*/