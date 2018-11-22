#include <webassembly.h>

#include "shaders.h"


export
rgba_t f_shader_inter(pixel_t a, pixel_t b, point_t p, scene_t *s)
{
    int ra, rb;
    float t;
    if (abs(b.pos.x - a.pos.x) > abs(b.pos.y - a.pos.y))
    {
        ra = abs(a.pos.x - p.x);
        rb = abs(b.pos.x - p.x);
    }
    else
    {
        ra = abs(a.pos.y - p.y);
        rb = abs(b.pos.y - p.y);
    }

    t = 1. * ra / (ra + rb);
    rgba_t c = {
        int_inter(a.col.r, b.col.r, t),
        int_inter(a.col.g, b.col.g, t),
        int_inter(a.col.b, b.col.b, t),
        int_inter(a.col.a, b.col.a, t)
    };
    return c;
}
