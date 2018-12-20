#pragma once

#include <inttypes.h>

#include "canvas.h"
#include "scene.h"
#include "shaders.h"
#include "math.h"


typedef v3_t vertex_t;
typedef v3_t normal_t;

typedef uint32_t vertexid_t;
typedef uint32_t normalid_t;

typedef struct {
    vertex_t *d;
    uint32_t l;
    uint32_t alloc;
} vertices_t;

typedef struct face_st {
    vertexid_t *v;
    normalid_t *n;
    uint32_t l;
    uint32_t alloc;
} face_t;

typedef vertices_t line_t;

typedef struct mat_st {
    rgba_t ambient;
    rgba_t diffuse;
    rgba_t specular;
} mat_t;

typedef struct model_props_st {
    shaders_t shaders;
    mat_t mat;
} model_props_t;

typedef struct model_st {
    vertices_t vs;
    struct {
        normal_t *d;
        uint32_t l;
        uint32_t alloc;
    } ns;
    struct {
        face_t *d;
        uint32_t l;
        uint32_t alloc;
    } fs;
    model_props_t props;
} model_t;

model_t *model_init(model_t *m, int v, int n, int f);
model_t *model_add_vertices_arr(model_t *m, vertex_t *vs, int n);
model_t *model_add_normals_arr(model_t *m, v3_t *ns, int n);
model_t *model_add_face_arr(model_t *m, vertexid_t *vs, normalid_t *ns, int n);
face_t model_face_static_init(int n);
