#ifndef _SYSERVER_POOL_H_
#define _SYSERVER_POOL_H_

#include "util.h"
#include "vector.h"

typedef union {
    void* next;
} sy_chunk_slot_t;

typedef struct {
    void* data;
} sy_chunk_t;

typedef struct {
    /* Number of objects per chunk */
    int width;
    int chunk_size;
    int nallocated;
    void* cur;
    sy_vector_t chunks;
} sy_pool_t;

int sy_chunk_init(sy_chunk_t* chunk, int width, int size);
int sy_pool_init(sy_pool_t* pool, int width, int chunk_size, int nchunks);
void* sy_pool_alloc(sy_pool_t* pool);
void sy_pool_clear(sy_pool_t* pool);
void sy_pool_free(sy_pool_t* pool, void* x);
#endif
