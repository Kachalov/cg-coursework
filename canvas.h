#pragma once

#include <inttypes.h>

typedef struct scene_st scene_t;
typedef struct v3_st vertex_t;


typedef struct rgba_st {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} rgba_t;

typedef struct canvas_st {
    rgba_t *data;
    uint32_t w;
    uint32_t h;
} canvas_t;

typedef struct point_st {
    uint16_t x;
    uint16_t y;
} point_t;

typedef struct pixel_st {
    rgba_t col;
    point_t pos;
} pixel_t;

typedef rgba_t (*v_shader_t)(pixel_t a, scene_t *s);
typedef rgba_t (*f_shader_t)(pixel_t a, pixel_t b, point_t x, scene_t *s);

#define SET_PIXEL_PTR(__ptr, __color) memcpy((__ptr), (__color), 4)
#define SET_PIXEL(__canv, __x, __y, __color) SET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)), __color)

#define GET_PIXEL_PTR(__ptr) ((rgba_t *)(__ptr))
#define GET_PIXEL(__canv, __x, __y) GET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)))

#define DOT(c, x, y, p) {\
SET_PIXEL(c, x, y, p); \
SET_PIXEL(c, x+1, y, p); \
SET_PIXEL(c, x, y+1, p); \
SET_PIXEL(c, x+1, y+1, p); }

canvas_t *canvas_init(int w, int h, rgba_t *data);
void draw_line(scene_t *s, pixel_t pa, pixel_t pb, f_shader_t f_shader);
void draw_fragment(scene_t *s, vertex_t *vs);
void draw_triangle(scene_t *s, pixel_t *ps);
void set_pixel(canvas_t *canv, uint32_t x, uint32_t y, rgba_t *color);
