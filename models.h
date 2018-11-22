#pragma once

#include <inttypes.h>

#include "canvas.h"
#include "scene.h"

typedef struct v3_st {
    float x;
    float y;
    float z;
} v3_t;

typedef v3_t vertex_t;
typedef v3_t normal_t;

typedef struct {
    vertex_t *vs;
    uint32_t vl;
} vertices_t;

typedef vertices_t face_t;
typedef vertices_t line_t;

typedef struct {
    normal_t *ns;
    vertex_t *vs;
    face_t *fs;
    uint32_t nl;
    uint32_t vl;
    uint32_t fl;
    rgba_t color;
} obj_t;


obj_t obj_init();
