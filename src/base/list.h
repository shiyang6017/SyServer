#ifndef _SYSERVER_LIST_H_
#define _SYSERVER_LIST_H_


#include "mempool.h"

typedef struct sy_list_node{
    struct sy_list_node* next;
    struct sy_list_node* prev;
    char data;  
}sy_list_node_t;

#define SY_LIST_WIDTH(type) (2 * sizeof(sy_list_node_t*) + sizeof(type))


typedef struct {
    int size;
    sy_list_node_t dummy;
    sy_pool_t* pool;
} sy_list_t;

int sy_list_insert(sy_list_t* list, sy_list_node_t* pos, sy_list_node_t* new_node);
int sy_list_delete(sy_list_t* list, sy_list_node_t* x);

static inline sy_list_node_t* sy_list_head(sy_list_t* list) {
    return list->dummy.next;
}

static inline sy_list_node_t* sy_list_tail(sy_list_t* list) {
    return list->dummy.prev;
}

static inline int sy_list_init(sy_list_t* list, sy_pool_t* pool) {
    list->size = 0;
    list->dummy.next = NULL;             
    list->dummy.prev = &list->dummy;   
    list->pool = pool;
    return OK;
}

static inline void sy_list_clear(sy_list_t* list) {
    while (sy_list_head(list) != NULL) {
        sy_list_delete(list, sy_list_head(list));
    }
}

static inline sy_list_node_t* sy_list_alloc(sy_list_t* list) {
    return sy_pool_alloc(list->pool);
}

static inline void sy_list_free(sy_list_t* list, sy_list_node_t* x) {
    sy_pool_free(list->pool, x);
}

#endif