#pragma once

#include "scene.h"


typedef struct {
    uint8_t *data;
    uint32_t w;
    uint32_t h;
} bitmask_t;

bitmask_t *bitmask_init(int w, int h);
void bitmask_free(bitmask_t *ptr);
void bitmask_invert(bitmask_t *bmask, point_t a, point_t b);
void bitmask_set(bitmask_t *bmask, point_t a, point_t b);
void bitmask_unset(bitmask_t *bmask, point_t a, point_t b);
void log_bitmask_row(bitmask_t *bmask, int row);
