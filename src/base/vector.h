#ifndef _SYSERVER_VECTOR_H_
#define _SYSERVER_VECTOR_H_

#include "util.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    int size;
    int capacity;
    int width;
    void* data;
} vector_t;

int sy_vector_init(vector_t* vec, int width, int size);
int sy_vector_reserve(vector_t* vec, int c);
int sy_vector_resize(vector_t* vec, int new_size);
void sy_vector_clear(vector_t* vec);

static inline void* sy_vector_at(vector_t* vec, int i) {
    if ( i >= vec->size) {
        return NULL;
    } else {
        return (void*)((char*)(vec->data) + i * vec->width);
    }
}

static inline void* sy_vector_back(vector_t* vec) {
    return vector_at(vec, vec->size - 1);
}

static inline void* sy_vector_push(vector_t* vec) {
    if (vector_resize(vec, vec->size + 1) != OK)
        return NULL;
    return vector_back(vec);
}

static inline void sy_vector_pop(vector_t* vec) {
    assert(vec->size > 0);
    --vec->size;
}

#endif
