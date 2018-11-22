#pragma once

#include <inttypes.h>

#include "canvas.h"
#include "scene.h"

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

typedef vertices_t face_t;
typedef vertices_t line_t;

typedef struct {
    normal_t *ns;
    vertices_t *vs;
    face_t *fs;
    uint32_t nl;
    uint32_t vl;
    uint32_t fl;
    rgba_t color;
} obj_t;


obj_t obj_init();
