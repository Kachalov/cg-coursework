#include <webassembly.h>

#include "heap.h"


void *heap_alloc(int size)
{
    void *r = malloc(size);
    return r;
}

void heap_free(void *ptr)
{
    free(ptr);
}

void *heap_realloc(void *ptr, int size)
{
    return realloc(ptr, size);
}
