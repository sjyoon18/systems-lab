#include "hashmap.h"

#include <stdio.h>

int main(void) {
    struct hashmap *map = hashmap_create(8);

    if (map == NULL) {
        fprintf(stderr, "failed to create hashmap\n");
        return 1;
    }

    hashmap_destroy(map);

    return 0;
}