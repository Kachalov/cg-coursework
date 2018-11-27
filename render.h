#pragma once

#include <webassembly.h>

#include "scene.h"
#include "models.h"
#include "shaders.h"


typedef struct evertex_st {
    vertex_t v;
    rgba_t c;
    v3_t n;
} evertex_t;

export void frame(scene_t *s);
void draw_model(scene_t *s, const model_t *model);
void draw_model_face(scene_t *s, const model_t *model, const face_t *face);
void draw_fragment(scene_t *s, evertex_t *vs, const model_props_t *props);
void draw_triangle(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat);
void draw_triangle_row(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat, int y);
//void _draw_triangle_row_bitmask(scene_t *s, vertex_t va, vertex_t vb, shader_f_t shf, mat_t *mat, int y, bitmask_t *bmask);
