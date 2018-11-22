#pragma once

#include <inttypes.h>

#include "models.h"
#include "bitmask.h"

struct canvas_st;
typedef struct canvas_st canvas_t;


typedef struct {
    uint8_t *data;
    uint32_t w;
    uint32_t h;
} zbuf_t;

typedef v3_t light_t;

typedef struct scene_st {
    canvas_t *canv;
    zbuf_t *zbuf;
    light_t *ls;
    uint32_t ll;
} scene_t;

scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint8_t *zbuf);
void scene_free(scene_t *s);
zbuf_t *zbuf_init(int w, int h, uint8_t *data);void clear(scene_t *s);
void frame(scene_t *s);
int16_t find_line_x(point_t a, point_t b, int16_t y);
