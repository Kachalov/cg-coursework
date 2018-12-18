#pragma once

#include <inttypes.h>

typedef struct scene_st scene_t;
typedef struct mat_st mat_t;
typedef struct zbuf_st zbuf_t;


typedef struct rgba_st {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} rgba_t;

typedef struct canvas_st {
    rgba_t *data;
    uint16_t w;
    uint16_t h;
} canvas_t;

typedef struct point_st {
    int16_t x;
    int16_t y;
} point_t;

typedef struct pixel_st {
    rgba_t col;
    point_t pos;
} pixel_t;

typedef rgba_t (*v_shader_t)(pixel_t a, scene_t *s);
typedef rgba_t (*f_shader_t)(pixel_t a, pixel_t b, point_t x, scene_t *s);

#define SET_PIXEL_PTR(__ptr, __color) memcpy((__ptr), (__color), 4)
#define SET_PIXEL(__canv, __x, __y, __color) SET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)), __color)

#define SET_PIXEL_Z(__canv, __zbuf, __x, __y, __z, __color) {\
    if (__zbuf->data[(__zbuf)->w * (__y) + (__x)] > __z)\
    {\
        SET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)), __color);\
        __zbuf->data[(__zbuf)->w * (__y) + (__x)] = __z;\
    }}

#define GET_PIXEL_PTR(__ptr) ((rgba_t *)(__ptr))
#define GET_PIXEL(__canv, __x, __y) GET_PIXEL_PTR((__canv)->data + ((__canv)->w * (__y) + (__x)))

#define DOT(c, x, y, p) {\
SET_PIXEL(c, x, y, p); \
SET_PIXEL(c, x+1, y, p); \
SET_PIXEL(c, x, y+1, p); \
SET_PIXEL(c, x+1, y+1, p); }

export
canvas_t *canvas_init(int w, int h, rgba_t *data);
export
void draw_line(scene_t *s, point_t a, point_t b, void *shf, mat_t *mat);
export
void set_pixel(canvas_t *canv, int16_t x, int16_t y, rgba_t *color);
void set_pixel_z(canvas_t *canv, zbuf_t *zbuf, int16_t x, int16_t y, int16_t z, rgba_t *color);
rgba_t rgba_scale3(rgba_t a, float k);
rgba_t int2rgba(int rgba);
int rgba2int(rgba_t rgba);
