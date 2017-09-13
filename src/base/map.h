#ifndef _SYSERVER_MAP_H_
#define _SYSERVER_MAP_H_

#include "string.h"

typedef sy_string_t sy_mime_val_t;
typedef struct {
    int offset;
    void* processor;
} sy_header_val_t;

typedef unsigned int sy_hash_t;


/* key */
typedef sy_string_t sy_map_key_t;

/* value */
typedef union {
    sy_mime_val_t mime;
    sy_header_val_t header;
} sy_map_val_t;


typedef struct sy_map_slot {
    sy_map_key_t key;
    sy_map_val_t val;
    struct sy_map_slot* next;
} sy_map_slot_t;

typedef struct {
    int size;
    int max_size;
    sy_map_slot_t* data;
    sy_map_slot_t* cur;
} sy_map_t;

sy_map_slot_t* sy_map_get(sy_map_t* map, const sy_map_key_t* key);
void sy_map_put(sy_map_t* map, const sy_string_t* key, const sy_map_val_t* val);

#endif
