#include <stdio.h>
#include <unistd.h>
#include <stddef.h>

struct block {
    size_t size;
    int free;
    struct block *next;
    struct block *prev;
};

struct block *head = NULL;
struct block *tail = NULL;

void append_block(struct block *block) {
    block->next = NULL;
    block->prev = tail;

    if (tail != NULL) {
        tail->next = block;
    } else {
        head = block;
    }

    tail = block;
}

struct block* find_free_block(size_t size) {
    struct block *current = tail;

    while (current != NULL) {
        if (current->free == 1 && current->size >= size) {
            return current;
        }
        current = current->prev;
    }

    return NULL;
}

void *my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    struct block *free_block = find_free_block(size);

    if (free_block != NULL) {
        free_block->free = 0;

        return free_block + 1;
    }

    void *raw = sbrk(sizeof(struct block) + size);

    if (raw == (void *)-1) {
        return NULL;
    }

    struct block *metadata = raw;

    metadata->size = size;
    metadata->free = 0;
    append_block(metadata);

    return metadata + 1;
}

void my_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    struct block *metadata = (struct block *)ptr - 1;
    metadata->free = 1;
}

int main() {
    char *p1 = my_malloc(10);
    char *p2 = my_malloc(20);

    printf("p1 = %p\n", p1);
    printf("p2 = %p\n", p2);

    my_free(p1);

    char *p3 = my_malloc(5);

    printf("p3 = %p\n", p3);

    return 0;
}