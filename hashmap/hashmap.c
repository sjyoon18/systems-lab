#include "hashmap.h"

#include <stdlib.h>

struct entry {
    char *key;
    void *value;
    struct entry *next;
};

struct hashmap {
    size_t bucket_count;
    size_t entry_count;
    struct entry **buckets;
};

struct hashmap *hashmap_create(size_t bucket_count) {
    if (bucket_count == 0) {
        return NULL;
    }

    struct hashmap *map = malloc(sizeof(*map));
    
    if (map == NULL) {
        return NULL;
    }

    map->buckets = calloc(bucket_count, sizeof(*map->buckets));

    if (map->buckets == NULL) {
        free(map);
        return NULL;
    }

    map->bucket_count = bucket_count;
    map->entry_count = 0;

    return map;
}

void hashmap_destroy(struct hashmap *map) {
    if (map == NULL) {
        return;
    }

    free(map->buckets);
    free(map);
}