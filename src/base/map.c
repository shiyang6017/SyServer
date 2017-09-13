#include "map.h"
#include "string.h"

#include <stddef.h>

static sy_hash_t sy_string_hash(const sy_string_t* str);

sy_map_slot_t* sy_map_get(sy_map_t* map, const sy_map_key_t* key) {
    sy_hash_t hash = sy_string_hash(key);
    sy_map_slot_t* slot = &map->data[hash % map->size];
    if (slot->key.data == NULL)
        return NULL;
    while (slot && !sy_string_eq(&slot->key, key)) {
        slot = slot->next;
    }
    return slot;
}

void sy_map_put(sy_map_t* map, const sy_string_t* key, const sy_map_val_t* val) {
    sy_hash_t hash = sy_string_hash(key);
    sy_map_slot_t* slot = &map->data[hash % map->size];
    
    if (slot->key.data == NULL) {
        slot->key = *key;
        slot->val = *val;
        return;
    }
    
    sy_map_slot_t* new_slot = map->cur++;
    new_slot->key = *key;
    new_slot->val = *val;
    new_slot->next = slot->next;
    slot->next = new_slot;
}

static sy_hash_t sy_string_hash(const sy_string_t* str) {
    sy_hash_t hash = 0;
    for (int i = 0; i < str->len; ++i)
        hash = (hash * 31) + str->data[i];
    return hash;
}
