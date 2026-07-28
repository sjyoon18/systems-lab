#include <assert.h>
#include <stdio.h>

#include "../hashmap.h"

static void test_create_destroy(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    hashmap_destroy(map);
}

static void test_put_get(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int value = 20;

    assert(hashmap_put(map, "age", &value));

    int *result = hashmap_get(map, "age");

    assert(result != NULL);
    assert(*result == 20);

    hashmap_destroy(map);
}

static void test_update_existing_key(void) {
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int x = 20;
    int y = 40;

    assert(hashmap_put(map, "age", &x));
    assert(hashmap_put(map, "age", &y));

    int *result = hashmap_get(map, "age");

    assert(result != NULL);
    assert(*result == 40);

    hashmap_destroy(map);
}

static void test_remove(void)
{
    struct hashmap *map = hashmap_create(8);
    assert(map != NULL);

    int value = 20;

    assert(hashmap_put(map, "age", &value));
    assert(hashmap_remove(map, "age"));
    assert(hashmap_get(map, "age") == NULL);
    assert(!hashmap_remove(map, "age"));

    hashmap_destroy(map);
}

static void test_missing_key(void)
{
    struct hashmap *map = hashmap_create(8);

    assert(map != NULL);

    assert(hashmap_get(map, "missing") == NULL);
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

    int *result_a = hashmap_get(map, "A");
    int *result_b = hashmap_get(map, "B");
    int *result_c = hashmap_get(map, "C");
    int *result_d = hashmap_get(map, "D");

    assert(result_a != NULL);
    assert(result_b != NULL);
    assert(result_c != NULL);
    assert(result_d != NULL);

    assert(*(int *)hashmap_get(map, "A") == 10);
    assert(*(int *)hashmap_get(map, "B") == 20);
    assert(*(int *)hashmap_get(map, "C") == 30);
    assert(*(int *)hashmap_get(map, "D") == 40);

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

    return 0;
}
