#ifndef _SYSERVER_MAP_H_
#define _SYSERVER_MAP_H_

#include "string.h"

typedef string_t mime_val_t;
typedef struct {
    int offset;
    void* processor;
} header_val_t;

typedef unsigned int hash_t;

typedef string_t map_key_t;

typedef union {
    mime_val_t mime;
    header_val_t header;
} map_val_t;


typedef struct map_slot map_slot_t;

struct map_slot {
    string_t key;
    map_val_t val;
    map_slot_t* next;
};

typedef struct {
    int size;
    int max_size;
    map_slot_t* data;
    map_slot_t* cur;
} map_t;

map_slot_t* sy_map_get(map_t* map, const map_key_t* key);
void sy_map_put(map_t* map, const string_t* key, const map_val_t* val);

#endif
