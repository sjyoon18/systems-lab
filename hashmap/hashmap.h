#ifndef HASHMAP_H
#define HASHMAP_H

#include <stddef.h>

struct hashmap;

struct hashmap *hashmap_create(size_t bucket_count);
void hashmap_destroy(struct hashmap *map);

#endif