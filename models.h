#pragma once

#include <inttypes.h>

#include "canvas.h"
#include "scene.h"
#include "shaders.h"

typedef struct v3_st {
    float x;
    float y;
    float z;
} v3_t;

typedef struct v4_st {
    float x;
    float y;
    float z;
    float w;
} v4_t;

typedef struct m3x3_st {
    v3_t d[3];
} m3x3_t;

typedef struct m3x4_st {
    v4_t d[3];
} m3x4_t;

typedef struct m4x3_st {
    v3_t d[4];
} m4x3_t;

typedef struct m4x4_st {
    v4_t d[4];
} m4x4_t;

typedef v3_t vertex_t;
typedef v3_t normal_t;

typedef struct {
    vertex_t *d;
    uint32_t l;
} vertices_t;

typedef struct face_st {
    uint32_t *d; // IDs
    uint32_t l;
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
    } ns;
    struct {
        face_t *d;
        uint32_t l;
    } fs;
    // TODO: lines
    model_props_t props;
} model_t;

model_t *model_init(int v, int n, int f);
