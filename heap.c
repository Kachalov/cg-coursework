#include <webassembly.h>

#include "heap.h"


export
void *heap_alloc(int size)
{
//    console_log("Size %d", size);
    void *r = malloc(size);
//    if (!r)
//        console_log("Nullptr! Size %d", size);
    return r;
}

export
void heap_free(void *ptr)
{
    free(ptr);
}

export
void *heap_realloc(void *ptr, int size)
{
    return realloc(ptr, size);
}
