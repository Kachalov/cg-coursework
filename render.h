#pragma once

#include <webassembly.h>

#include "scene.h"
#include "models.h"
#include "shaders.h"


int render_mode;
#define RENDER_FACE 1
#define RENDER_WIREFRAME 2
#define RENDER_ZBUF 4
#define RENDER_GRID 8
#define RENDER_XYZ 16
#define RENDER_NORMS 32
#define RENDER_VERTS 64
#define RENDER_LIGHTS 128
#define RENDER_COLORED 256

typedef struct evertex_st {
    vertex_t v;
    vertex_t wv;
    v3_t n;
    v3_t wn;
    rgba_t c;
    frgba_t light;
} evertex_t;

typedef struct yield_evertex_st {
    evertex_t ev;
    evertex_t dev;
    frgba_t evc;
    frgba_t devc;
    double d;
    double t;
} yield_evertex_t;

export void frame(scene_t *s, int render_mode);
void draw_grid(scene_t *s, int size, int count);
void draw_xyz(scene_t *s);
void draw_lights(scene_t *s);
void draw_model(scene_t *s, const model_t *model);
void draw_model_face(scene_t *s, const model_t *model, const face_t *face);
void draw_fragment(scene_t *s, evertex_t *vs, const model_props_t *props);
export void draw_triangle(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat);
export void draw_triangle_row(scene_t *s, evertex_t *vs, shader_f_t shf,
    const mat_t *mat, int y, yield_evertex_t *ya, yield_evertex_t *yb);
yield_evertex_t yield_evertex_init(evertex_t a, evertex_t b, int steps);
void yield_evertex(evertex_t *ev, yield_evertex_t *yv);
void draw_zbuf(scene_t *s);
void draw_norm(scene_t *s, evertex_t v);

#define YIELD_EVERTEX(_ev, _yv) \
(_yv)->ev.v.x += (_yv)->dev.v.x; \
(_yv)->ev.v.y += (_yv)->dev.v.y; \
(_yv)->ev.v.z += (_yv)->dev.v.z; \
(_yv)->ev.wv.x += (_yv)->dev.wv.x; \
(_yv)->ev.wv.y += (_yv)->dev.wv.y; \
(_yv)->ev.wv.z += (_yv)->dev.wv.z; \
(_yv)->ev.n.x += (_yv)->dev.n.x; \
(_yv)->ev.n.y += (_yv)->dev.n.y; \
(_yv)->ev.n.z += (_yv)->dev.n.z; \
(_yv)->ev.wn.x += (_yv)->dev.wn.x; \
(_yv)->ev.wn.y += (_yv)->dev.wn.y; \
(_yv)->ev.wn.z += (_yv)->dev.wn.z; \
(_ev)->v = (_yv)->ev.v; \
(_ev)->wv = (_yv)->ev.wv; \
(_ev)->n = (_yv)->ev.n; \
(_ev)->wn = (_yv)->ev.wn; \
(_ev)->c.r = clamp(lroundf((_yv)->evc.r += (_yv)->devc.r), 0, 255); \
(_ev)->c.g = clamp(lroundf((_yv)->evc.g += (_yv)->devc.g), 0, 255); \
(_ev)->c.b = clamp(lroundf((_yv)->evc.b += (_yv)->devc.b), 0, 255); \
(_ev)->c.a = clamp(lroundf((_yv)->evc.a += (_yv)->devc.a), 0, 255); \
(_ev)->light.r = (_yv)->ev.light.r += (_yv)->dev.light.r; \
(_ev)->light.g = (_yv)->ev.light.g += (_yv)->dev.light.g; \
(_ev)->light.b = (_yv)->ev.light.b += (_yv)->dev.light.b; \
(_ev)->light.a = (_yv)->ev.light.a += (_yv)->dev.light.a; \
(_yv)->t += (_yv)->d;
