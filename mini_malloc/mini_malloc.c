#include <stdio.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>

struct block {
    size_t size;
    int free;
    struct block *next;
    struct block *prev;
};

struct block *head = NULL;
struct block *tail = NULL;

size_t align8(size_t size) {
    return (size + 7) & ~7;
}

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

void split_block(struct block *block, size_t size) {
    if (block->size <= size + sizeof(struct block)) {
        return;
    }

    size_t new_block_size = block->size - size - sizeof(struct block);

    struct block *new_block = (struct block *)((char *)(block + 1) + size);

    new_block->size = new_block_size;
    new_block->free = 1;
    new_block->next = block->next;
    new_block->prev = block;

    if (new_block->next != NULL) {
        new_block->next->prev = new_block;
    } else {
        tail = new_block;
    }

    block->size = size;
    block->next = new_block;
}

void *my_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }

    size = align8(size);

    struct block *free_block = find_free_block(size);

    if (free_block != NULL) {
        split_block(free_block,size);
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

void *my_calloc(size_t count, size_t size) {
    size_t total_size = count * size;

    void *ptr = my_malloc(total_size);

    if (ptr == NULL) {
        return NULL;
    }

    memset(ptr, 0, total_size);

    return ptr;
}

void coalesce_next(struct block *block) {
    if (block == NULL || block->next == NULL) {
        return;
    }

    struct block *next = block->next;

    if (block->free == 1 && next->free == 1) {
        block->size = block->size + sizeof(struct block) + next->size;

        block->next = next->next;

        if (next->next != NULL) {
            next->next->prev = block;
        } else {
            tail = block;
        }
    }
}

void my_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    struct block *metadata = (struct block *)ptr - 1;
    metadata->free = 1;

    coalesce_next(metadata);

    if (metadata->prev != NULL) {
        coalesce_next(metadata->prev);
    }
}

void *my_realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
        return my_malloc(size);
    }

    if (size == 0) {
        my_free(ptr);
        return NULL;
    }

    size = align8(size);

    struct block *metadata = (struct block *)ptr - 1;

    if (metadata->size >= size) {
        split_block(metadata, size);

        if (metadata->next != NULL) {
            coalesce_next(metadata->next);
        }

        return ptr;
    }

    struct block *next = metadata->next;

    if (
        next != NULL &&
        next->free == 1 &&
        metadata->size + sizeof(struct block) + next->size >= size
    ) {
        metadata->size += sizeof(struct block) + next->size;
        metadata->next = next->next;

        if (metadata->next != NULL) {
            metadata->next->prev = metadata;
        } else {
            tail = metadata;
        }

        split_block(metadata, size);

        return ptr;
    }

    void *new_ptr = my_malloc(size);

    if (new_ptr == NULL) {
        return NULL;
    }

    size_t copy_size = metadata->size;
    if (copy_size > size) {
        copy_size = size;
    }

    memcpy(new_ptr, ptr, copy_size);
    my_free(ptr);

    return new_ptr;
}

void print_heap(void) {
    struct block *current = head;
    int i = 0;

    while (current != NULL) {

        printf("+------------------------------+\n");
        printf("| Block %-2d %-10s          |\n", i, current->free ? "(FREE)" : "(USED)");
        printf("+------------------------------+\n");
        printf("| Addr : %-21p |\n", (void *)current);
        printf("| Size : %-21zu |\n", current->size);
        printf("+------------------------------+\n");

        current = current->next;
        i++;
    }

    printf("\n");
}

int main() {
    char *p1 = my_malloc(100);
    char *p2 = my_malloc(40);
    char *p3 = my_malloc(60);

    printf("Initial:\n");
    print_heap();

    my_free(p2);
    printf("After free p2:\n");
    print_heap();

    my_free(p1);
    printf("After free p1:\n");
    print_heap();

    char *p4 = my_malloc(60);
    printf("After malloc p4:\n");
    print_heap();

    my_realloc(p4, 10);
    printf("After realloc p4 (shrink):\n");
    print_heap();
}