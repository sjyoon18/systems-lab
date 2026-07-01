#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../allocator.h"

int main(void) {
    printf("malloc_test\n");
    
    char *p = my_malloc(20);
    assert(p != NULL);

    strcpy(p, "Hello World");
    assert(strcmp(p, "Hello World") == 0);

    verify_heap();
    print_heap();

    return 0;
}