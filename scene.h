#pragma once

#include <inttypes.h>

#include "canvas.h"
#include "models.h"



typedef v3_t light_t;

typedef struct {
    canvas_t *canv;
    zbuf_t *zbuf;
    light_t *ls;
    uint32_t ll;
} scene_t;

typedef rgba_t (*f_shader_t)(pixel_t a, pixel_t b, point_t x, scene_t *s);
