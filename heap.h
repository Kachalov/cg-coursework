#pragma once

#include <webassembly.h>

export void *heap_alloc(int size);
export void heap_free(void *ptr);
