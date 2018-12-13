#pragma once

#include <webassembly.h>

#include "scene.h"

typedef struct point_st point_t;


#define BITMASK_CHUNK_SIZE 8
#define BITMASK_CHUNK_MASK 0xff

typedef uint8_t bitmask_data_t;

typedef struct bitmask_st {
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
