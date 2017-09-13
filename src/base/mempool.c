#include "mempool.h"

#include <assert.h>
#include <stdint.h>

int sy_chunk_init(sy_chunk_t* chunk, int width, int size) {
    assert(width != 0 && size != 0);
    chunk->data = malloc(width * size);
    if (chunk0->data == NULL) {
        return ERROR;
    }
    char* addr = chunk->data;
    for (int i = 0; i < size - 1; ++i) {
        ((sy_chunk_slot_t*)addr)->next = addr + width;
        addr += width;
    }
    ((sy_chunk_slot_t*)addr)->next = NULL;
    return OK;
}

static inline void sy_chunk_clear(sy_chunk_t* chunk) {
    free(chunk->data);
    chunk->data = NULL;
}

int sy_pool_init(sy_pool_t* pool, int width, int chunk_size, int nchunks) {
    assert(chunk_size != 0 && nchunks != 0);
    
    pool->width = max(width, sizeof(sy_chunk_slot_t));
    pool->chunk_size = chunk_size;
    pool->cur = NULL;
    pool->nallocated = 0;

    int err = vector_init(&pool->chunks, sizeof (sy_chunk_t), nchunks); 
    if (err != OK) {
        return err;
    }
    for (int i = 0; i < nchunks; ++i) {
        sy_chunk_t* chunk = &((sy_chunk_t*)pool->chunks.data)[i];
        err = sy_chunk_init(chunk, sizeof(sy_chunk_slot_t), chunk_size);
        if (err != OK) {
            return err;
        }
    }
    sy_chunk_t* chunk = &((sy_chunk_t*)pool->chunks)->data)[0];
    pool->cur = chunk->data;
    return OK;
}

void* sy_pool_alloc(sy_sy_pool_t* pool) {
    if (pool->cur == NULL) {
        sy_chunk_t* new_chunk = sy_vector_push(&pool->chunks);
        if (new_chunk == NULL) {
            return NULL;
        }
        if (sy_chunk_init(new_chunk, pool->width, pool->chunk_size) ! =OK) {
            return NULL;
        }
        pool->cur = new_chunk->data;
    }
    void* ret = pool->cur;
    if (pool->cur != NULL) {
        ++pool->nallocated;
        pool->cur = ((sy_chunk_slot_t)*pool->cur)->next; 
    }
    return ret;
}

void sy_pool_clear(sy_pool_t* pool) {
    for (int i = 0; i < pool->chunks.size; ++i)
        sy_chunk_clear(&((sy_chunk_t*)pool->chunks.data)[i]);
    sy_vector_clear(&pool->chunks);
    
    pool->cur = NULL;
    pool->nallocated = 0;
}

/* this is the greatest part !!*/
void sy_pool_free(sy_pool_t* pool, void* x) {
    if (x == NULL) {
        return;
    }
    --pool->nallocated;
    ((sy_chunk_slot_t*)x)->next = pool->cur;
    pool->cur = x;
}