#pragma once

#include <webassembly.h>

#include "scene.h"


#define BITMASK_CHUNK_SIZE 16
#define BITMASK_CHUNK_MASK 0xffff

typedef uint16_t bitmask_data_t;

typedef struct {
    bitmask_data_t *data;
    uint32_t w;
    uint32_t h;
} bitmask_t;

export bitmask_t *bitmask_init(int w, int h);
export void bitmask_free(bitmask_t *ptr);
export void bitmask_invert(bitmask_t *bmask, point_t a, point_t b);
export void bitmask_set(bitmask_t *bmask, point_t a, point_t b);
export void bitmask_unset(bitmask_t *bmask, point_t a, point_t b);
export void log_bitmask_row(bitmask_t *bmask, int row);
