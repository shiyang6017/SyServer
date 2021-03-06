#ifndef _SY_UTIL_H_
#define _SY_UTIL_H_

#define OK                  (0)
#define AGAIN               (1)
#define EMPTY_LINE          (2)
#define CLOSED              (3)
#define ERROR               (-1)
#define ERR_INVALID_REQUEST (-2)
#define ERR_INVALID_METHOD  (-3)
#define ERR_INVALID_VERSION (-4)
#define ERR_INTERNAL_ERROR  (-5)
#define ERR_INVALID_HEADER  (-6)
#define ERR_INVALID_URI     (-7)

#define ERR_UNSUPPORTED_VERSION (-8)

#define ERR_NOMEM           (-9)

#define CRLF                "\r\n"

#define SY_ABORT_ON(cond, msg)          \
do {                                \
    if (cond) {                     \
        fprintf(stderr, "%s: %d: ", \
                __FILE__, __LINE__);\
        perror(msg);                \
        abort();                    \
    }                               \
} while (0)

#define SY_ERR_ON(cond, msg)           \
do {                                \
    if (cond) {                     \
        fprintf(stderr, "%s: %d: ", \
                __FILE__, __LINE__);\
        perror(msg);                \
    }                               \
} while (0)


static inline int min(int x, int y) {
    return x < y ? x: y;
}

static inline int max(int x, int y) {
    return x > y ? x: y;
}

void sy_error(const char* format, ...);
void sy_log(const char* format, ...);

#endif
