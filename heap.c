#include <webassembly.h>

#include "heap.h"


export
void *heap_alloc(int size)
{
    return malloc(size);
}

export
void heap_free(void *ptr)
{
    free(ptr);
}
