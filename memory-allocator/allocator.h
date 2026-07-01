#ifndef MINI_MALLOC_H
#define MINI_MALLOC_H

#include <stddef.h>

void *my_malloc(size_t size);
void my_free(void *ptr);
void *my_realloc(void *ptr, size_t size);
void *my_calloc(size_t count, size_t size);

void verify_heap(void);
void print_heap(void);

#endif