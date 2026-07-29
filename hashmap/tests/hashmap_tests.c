#include <assert.h>
#include <stdio.h>

#include "../hashmap.h"

static void assert_key_value(struct hashmap *map, const char *key, int expected) {
    int *result = hashmap_get(map, key);

    assert(result != NULL);
    assert(*result == expected);
}

static void assert_missing(struct hashmap *map, const char *key) {
    assert(hashmap_get(map, key) == NULL);
}

static void test_create_destroy(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    hashmap_destroy(map);
}

static void test_put_get(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int value = 20;

    assert(hashmap_put(map, "key", &value));

    assert_key_value(map, "key", 20);

    hashmap_destroy(map);
}

static void test_update_existing_key(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int x = 20;
    int y = 40;

    assert(hashmap_put(map, "key", &x));
    assert(hashmap_put(map, "key", &y));

    assert_key_value(map, "key", 40);

    hashmap_destroy(map);
}

static void test_remove(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int value = 20;

    assert(hashmap_put(map, "key", &value));
    assert(hashmap_remove(map, "key"));
    assert_missing(map, "key");
    assert(!hashmap_remove(map, "key"));

    hashmap_destroy(map);
}

static void test_missing_key(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    assert_missing(map, "missing");
    assert(!hashmap_remove(map, "missing"));

    hashmap_destroy(map);
}

static void test_collisions(void) {
    struct hashmap *map = hashmap_create(1);
    assert(map != NULL);

    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;

    assert(hashmap_put(map, "A", &a));
    assert(hashmap_put(map, "B", &b));
    assert(hashmap_put(map, "C", &c));
    assert(hashmap_put(map, "D", &d));

    assert_key_value(map, "A", 10);
    assert_key_value(map, "B", 20);
    assert_key_value(map, "C", 30);
    assert_key_value(map, "D", 40);

    hashmap_destroy(map);
}

static void test_collision_remove(void) {
    struct hashmap *map = hashmap_create(1);
    assert(map != NULL);

    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;

    assert(hashmap_put(map, "A", &a));
    assert(hashmap_put(map, "B", &b));
    assert(hashmap_put(map, "C", &c));
    assert(hashmap_put(map, "D", &d));

    assert(hashmap_remove(map, "D"));

    assert_key_value(map, "A", 10);
    assert_key_value(map, "B", 20);
    assert_key_value(map, "C", 30);
    assert_missing(map, "D");

    assert(hashmap_remove(map, "B"));

    assert_key_value(map, "A", 10);
    assert_missing(map, "B");
    assert_key_value(map, "C", 30);
    assert_missing(map, "D");

    assert(hashmap_remove(map, "A"));

    assert_missing(map, "A");
    assert_missing(map, "B");
    assert_key_value(map, "C", 30);
    assert_missing(map, "D");

    hashmap_destroy(map);
}

static void test_resize(void) {
    enum { NUM_ENTRIES = 1000 };

    struct hashmap *map = hashmap_create(2);
    assert(map != NULL);

    int values[NUM_ENTRIES];

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];
        values[i] = i;
        
        snprintf(key, sizeof(key), "key%d", i);
        assert(hashmap_put(map, key, &values[i]));
    }

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];

        snprintf(key, sizeof(key), "key%d", i);

        assert_key_value(map, key, i);
    }

    hashmap_destroy(map);
}

static void test_stress_remove(void) {
    enum { NUM_ENTRIES = 1000 };

    struct hashmap *map = hashmap_create(2);
    assert(map != NULL);

    int values[NUM_ENTRIES];

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];
        values[i] = i;
        
        snprintf(key, sizeof(key), "key%d", i);
        assert(hashmap_put(map, key, &values[i]));
    }

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];

        snprintf(key, sizeof(key), "key%d", i);

        assert_key_value(map, key, i);
    }

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];

        snprintf(key, sizeof(key), "key%d", i);

        assert(hashmap_remove(map, key));
        assert(!hashmap_remove(map, key));
    }

    for (int i = 0; i < NUM_ENTRIES; i++) {
        char key[32];

        snprintf(key, sizeof(key), "key%d", i);

        assert_missing(map, key);
    }

    hashmap_destroy(map);
}

int main(void)
{
    test_create_destroy();
    puts("PASS: create_destroy");

    test_put_get();
    puts("PASS: put_get");

    test_update_existing_key();
    puts("PASS: update_existing_key");

    test_remove();
    puts("PASS: remove");

    test_missing_key();
    puts("PASS: missing_key");

    test_collisions();
    puts("PASS: collisions");

    test_collision_remove();
    puts("PASS: collision_remove");

    test_resize();
    puts("PASS: resize");

    test_stress_remove();
    puts("PASS: stress_remove");

    return 0;
}
