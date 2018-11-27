#pragma once

#include <inttypes.h>

#include "bitmask.h"

struct canvas_st;
typedef struct canvas_st canvas_t;
typedef struct model_st model_t;
typedef struct v3_st v3_t;
typedef struct rgba_st rgba_t;


typedef struct {
    uint8_t *data;
    uint32_t w;
    uint32_t h;
} zbuf_t;

typedef v3_t light_t;

typedef struct scene_st {
    canvas_t *canv;
    bitmask_t *bmask_row;
    zbuf_t *zbuf;
    struct {
        light_t *d;
        uint32_t l;
    } ls;
    struct {
        void *d;
        uint8_t l;
        uint8_t cur;
    } cams;
    struct {
        model_t **d;
        uint8_t l;
    } models;
} scene_t;

scene_t *scene;

scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint8_t *zbuf);
void scene_free(scene_t *s);
int scene_add_model(scene_t *s, model_t *model);
zbuf_t *zbuf_init(int w, int h, uint8_t *data);
void clear(scene_t *s);
