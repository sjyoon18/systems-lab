#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../allocator.h"

int main(void) {
    printf("split_test\n");
    
    char *p1 = my_malloc(100);
    my_free(p1);

    char *p2 = my_malloc(20);
    assert(p2 == p1);

    verify_heap();
    print_heap();

    return 0;
}