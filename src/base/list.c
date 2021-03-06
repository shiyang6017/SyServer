#include "list.h"

#include <assert.h>

int sy_list_insert(sy_list_t* list, sy_list_node_t* pos, sy_list_node_t* new_node) {
    assert(pos != NULL);
    new_node->next = pos->next;
    if (new_node->next != NULL)
        new_node->next->prev = new_node;
    else
        list->dummy.prev = new_node;    // The tail
    new_node->prev = pos;
    pos->next = new_node;
    
    ++list->size;
    return OK;
}

int sy_list_delete(sy_list_t* list, sy_list_node_t* x) {
    assert(list->size > 0 && x != &list->dummy);
    x->prev->next = x->next;
    if (x->next != NULL)
        x->next->prev = x->prev;
    else
        list->dummy.prev = x->prev; // The tail
        
    sy_list_free(list, x);
    --list->size;
    return OK;
}
