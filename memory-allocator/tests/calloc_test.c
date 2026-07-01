#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../allocator.h"

int main(void) {
    printf("calloc_test\n");

    int *arr = my_calloc(5, sizeof(int));
    assert(arr != NULL);

    for (int i = 0; i < 5; i++) {
        assert(arr[i] == 0);
    }

    verify_heap();
    print_heap();

    return 0;
}