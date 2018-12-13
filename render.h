#pragma once

#include <webassembly.h>

#include "scene.h"
#include "models.h"
#include "shaders.h"


typedef struct evertex_st {
    vertex_t v;
    v3_t n;
    rgba_t c;
    rgba_t light;
} evertex_t;

typedef struct yield_evertex_st {
    evertex_t ev;
    evertex_t dev;
    struct {
        float r;
        float g;
        float b;
        float a;
    } evc;
    struct {
        float r;
        float g;
        float b;
        float a;
    } devc;
    float d;
    float t;
} yield_evertex_t;

export void frame(scene_t *s);
void draw_model(scene_t *s, const model_t *model);
void draw_model_face(scene_t *s, const model_t *model, const face_t *face);
void draw_fragment(scene_t *s, evertex_t *vs, const model_props_t *props);
export void draw_triangle(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat);
export void draw_triangle_row(scene_t *s, evertex_t *vs, shader_f_t shf,
    const mat_t *mat, int y, yield_evertex_t *ya, yield_evertex_t *yb);

#define YIELD_EVERTEX(_ev, _yv) \
(_yv)->ev.v.x += (_yv)->dev.v.x; \
(_yv)->ev.v.y += (_yv)->dev.v.y; \
(_yv)->ev.v.z += (_yv)->dev.v.z; \
(_ev)->v = (_yv)->ev.v; \
(_ev)->c.r = lroundf((_yv)->evc.r += (_yv)->devc.r); \
(_ev)->c.g = lroundf((_yv)->evc.g += (_yv)->devc.g); \
(_ev)->c.b = lroundf((_yv)->evc.b += (_yv)->devc.b); \
(_ev)->c.a = lroundf((_yv)->evc.a += (_yv)->devc.a); \
(_yv)->t += (_yv)->d;

yield_evertex_t yield_evertex_init(evertex_t a, evertex_t b, int steps);
void yield_evertex(evertex_t *ev, yield_evertex_t *yv);
