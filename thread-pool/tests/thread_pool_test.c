#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../thread_pool.h"

static void print_number(void *arg) {
    int number = *(int *)arg;

    printf("Processing %d\n", number);
    sleep(1);
    free(arg);
}

int main(void) {
    struct thread_pool pool;

    if (thread_pool_init(&pool, 4) != 0) {
        fprintf(stderr, "Failed to initialize thread pool\n");
        return 1;
    }

    for (int i = 0; i < 8; i++) {
        int *number = malloc(sizeof(int));

        if (number == NULL) {
            fprintf(stderr, "Failed to allocate job argument\n");
            return 1;
        }

        *number = i;

        if (thread_pool_submit(&pool, print_number, number) != 0) {
            fprintf(stderr, "Failed to submit job\n");
            free(number);
            return 1;
        }
    }

    thread_pool_destroy(&pool);
    printf("Test finished\n");

    return 0;
}