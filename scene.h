#pragma once

#include <inttypes.h>

#include "math.h"
#include "canvas.h"
#include "bitmask.h"

struct canvas_st;
typedef struct model_st model_t;


#define ZBUF_DEPTH 65535

typedef struct zbuf_st {
    uint16_t *data;
    uint32_t w;
    uint32_t h;
} zbuf_t;

typedef struct light_attens_st {
    float c;
    float l;
    float q;
} light_attens_t;

typedef struct light_st {
    v3_t pos;
    light_attens_t attens;
    struct {
        rgba_t ambient;
        rgba_t diffuse;
        rgba_t specular;
    } cols;
} light_t;

typedef struct scene_st {
    canvas_t *canv;
    bitmask_t *bmask_row;
    zbuf_t *zbuf;
    struct {
        light_t *d;
        uint32_t l;
    } ls;
    /*struct {
        void *d;
        uint8_t l;
        uint8_t cur;
    } viewports;*/
    m4_t viewport;
    struct {
        v3_t eye;
        v3_t center;
        v3_t up;
    } viewport_props;
    struct {
        float angle;
        float ratio;
        float near;
        float far;
    } perspective_props;
    struct {
        model_t **d;
        uint8_t l;
    } models;
    m4_t view_mtrx;
    m4_t viewport_mtrx;
    m4_t proj_mtrx;
    m4_t mvp_mtrx;
} scene_t;

scene_t *scene;

scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint16_t *zbuf);
void scene_free(scene_t *s);
export int scene_add_model(scene_t *s, model_t *model);
zbuf_t *zbuf_init(int w, int h, uint16_t *data);
void clear(scene_t *s);
export void calculate_mtrx(scene_t *s);
export void move_viewport(scene_t *s, float hor, float vert, float tang, float norm);
export int scene_add_light(scene_t *s, light_t *light);
export int scene_create_light(scene_t *s, int x, int y, int z, int ambient);
export int scene_create_cube(scene_t *s, int x, int y, int z,
    int shader, int color, int w, int h, int d);
export int scene_create_sphere(scene_t *s, int x, int y, int z,
    int shader, int color, int segs, int r);
void *scene_select_shder_v(int n);
void *scene_select_shder_f(int n);
