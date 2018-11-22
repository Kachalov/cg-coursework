#pragma once

#include <inttypes.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} rgba_t;

typedef struct {
    rgba_t *data;
    uint32_t w;
    uint32_t h;
} canvas_t;

typedef struct {
    uint16_t x;
    uint16_t y;
} point_t;

typedef struct {
    uint8_t *data;
    uint32_t w;
    uint32_t h;
} zbuf_t;

typedef struct {
    rgba_t col;
    point_t pos;
} pixel_t;

typedef struct {
    uint8_t *data;
    uint32_t w;
    uint32_t h;
} bitmask_t;

#define SET_PIXEL_PTR(__ptr, __color) memcpy((__ptr), (__color), 4)
#define SET_PIXEL(__canv, __x, __y, __color) SET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)), __color)

#define GET_PIXEL_PTR(__ptr) ((rgba_t *)(__ptr))
#define GET_PIXEL(__canv, __x, __y) GET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)))
