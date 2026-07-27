#include "hashmap.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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

static size_t hash_string(const char *key) {
    size_t hash = 0;

    while(*key != '\0') {
        hash = hash * 31 + (unsigned char)*key;
        key++;
    }

    return hash;
}

static struct entry *entry_create(const char *key, void *value) {
    struct entry *entry = malloc(sizeof(*entry));

    if (entry == NULL) {
        return NULL;
    }
    
    entry->key = malloc(strlen(key) + 1);

    if (entry->key == NULL) {
        free(entry);
        return NULL;
    }

    strcpy(entry->key, key);
    entry->value = value;
    entry->next = NULL;

    return entry;
}

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

bool hashmap_put(struct hashmap *map, const char *key, void *value) {
    if (map == NULL || key == NULL) {
        return false;
    }

    size_t bucket_index = hash_string(key) % map->bucket_count;

    struct entry *previous = NULL;
    struct entry *current = map->buckets[bucket_index];

    while(current != NULL) {
        if (strcmp(key, current->key) == 0) {
            current->value = value;
            return true;
        }
        previous = current;
        current = current->next;
    }

    struct entry *entry = entry_create(key, value);

    if (entry == NULL) {
        return false;
    }
    
    if (previous == NULL) {
        map->buckets[bucket_index] = entry;
    } else {
        previous->next = entry;
    }

    map->entry_count++;
    
    return true;
}

void *hashmap_get(struct hashmap *map, const char *key) {
    if (map == NULL || key == NULL) {
        return NULL;
    }

    size_t bucket_index = hash_string(key) % map->bucket_count;
    struct entry *current = map->buckets[bucket_index];

    while (current != NULL) {
        if (strcmp(key, current->key) == 0) {
            return current->value;
        }
        current = current->next;
    }

    return NULL;
}

static void entry_destroy(struct entry *entry) {
    if (entry == NULL) {
        return;
    }

    free(entry->key);
    free(entry);
}

void hashmap_destroy(struct hashmap *map) {
    if (map == NULL) {
        return;
    }
    for (size_t i = 0; i < map->bucket_count; i++) {
        struct entry *entry = map->buckets[i];
        while(entry != NULL) {
            struct entry *next_entry = entry->next;
            entry_destroy(entry);
            entry = next_entry;
        }
    }
    free(map->buckets);
    free(map);
}