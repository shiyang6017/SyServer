#ifndef _SYSERVER_QUEUE_H_
#define _SYSERVER_QUEUE_H_

#include "list.h"
#include "mempool.h"

#define QUEUE_WIDTH(type)   LIST_WIDTH(type) 


typedef list_node_t queue_node_t;

typedef struct {
    list_t container;
} queue_t;

static inline int sy_queue_init(queue_t* queue, pool_t* pool) {
    return sy_list_init(&queue->container, pool);
}

static inline void sy_queue_clear(queue_t* queue) {
    sy_list_clear(&queue->container);
}

static inline void* sy_queue_alloc(queue_t* queue) {
    return (void*)(&sy_list_alloc(&queue->container)->data);
}

static inline int sy_queue_push(queue_t* queue, void* x) {
    list_t* list = &queue->container;
    void* node_x = (char*)x - sizeof(list_node_t*) * 2;
    return sy_list_insert(list, list_tail(list), node_x);
}

static inline void sy_queue_pop(queue_t* queue) {
    list_t* list = &queue->container;
    sy_list_delete(list, sy_list_head(list));
}

static inline int sy_queue_size(queue_t* queue) {
    return queue->container.size;
}

static inline int sy_queue_empty(queue_t* queue) {
    return sy_queue_size(queue) == 0;
}

static inline void* sy_queue_front(queue_t* queue) {
    list_t* list = &queue->container;
    list_node_t* head = sy_list_head(list);
    if (head == NULL)
        return NULL;
    return &head->data;
}

static inline void* sy_queue_back(queue_t* queue) {
    list_t* list = &queue->container;
    list_node_t* tail = sy_list_tail(list);
    if (tail == &list->dummy)
        return NULL;
    return &tail->data;
}

#endif
