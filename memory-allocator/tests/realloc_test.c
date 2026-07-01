#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../allocator.h"

int main(void) {
    printf("realloc_test\n");
    
    char *p = my_malloc(20);
    strcpy(p, "Hello World");

    p = my_realloc(p, 80);

    assert(p != NULL);
    assert(strcmp(p, "Hello World") == 0);

    verify_heap();
    printf("grow:\n");
    print_heap();

    p = my_realloc(p, 20);

    assert(p != NULL);
    assert(strcmp(p, "Hello World") == 0);

    verify_heap();
    printf("shrink:\n");
    print_heap();

    return 0;
}