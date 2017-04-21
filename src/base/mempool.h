#ifndef _SYSERVER_POOL_H_
#define _SYSERVER_POOL_H_

#include "util.h"
#include "vector.h"

typedef union {
    void* next;
} chunk_slot_t;

typedef struct {
    void* data;
} chunk_t;

typedef struct {
    /* Number of objects per chunk */
    int width;
    int chunk_size;
    int nallocated;
    void* cur;
    vector_t chunks;
} pool_t;

int chunk_init(chunk_t* chunk, int width, int size);
int pool_init(pool_t* pool, int width, int chunk_size, int nchunks);
void* pool_alloc(pool_t* pool);
void pool_clear(pool_t* pool);

#endif