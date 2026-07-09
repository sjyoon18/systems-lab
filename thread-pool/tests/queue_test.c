#include <stdio.h>
#include <assert.h>
#include "../queue.h"

void print_number(void *arg) {
    int *num = arg;
    printf("%d\n", *num);
}

int main(void) {
    struct job_queue queue;

    queue_init(&queue);

    assert(queue_empty(&queue));
    assert(dequeue(&queue) == NULL);

    int a = 1;
    int b = 2;
    int c = 3;

    struct job job1 = { print_number, &a, NULL };
    struct job job2 = { print_number, &b, NULL };
    struct job job3 = { print_number, &c, NULL };

    enqueue(&queue, &job1);
    enqueue(&queue, &job2);
    enqueue(&queue, &job3);

    assert(!queue_empty(&queue));

    struct job *first = dequeue(&queue);
    struct job *second = dequeue(&queue);
    struct job *third = dequeue(&queue);

    assert(first == &job1);
    assert(second == &job2);
    assert(third == &job3);

    first->function(first->arg);
    second->function(second->arg);
    third->function(third->arg);

    assert(queue_empty(&queue));
    assert(dequeue(&queue) == NULL);

    return 0;
}