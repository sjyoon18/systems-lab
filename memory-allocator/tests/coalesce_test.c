#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../allocator.h"

int main(void) {
    printf("coalesce_test\n");
    char *p1 = my_malloc(20);
    char *p2 = my_malloc(30);

    my_free(p1);
    my_free(p2);

    char *p3 = my_malloc(60);

    assert(p3 == p1);

    verify_heap();
    print_heap();

    return 0;
}