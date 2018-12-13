#include <webassembly.h>

#include "canvas.h"
#include "math.h"
#include "shaders.h"
#include "heap.h"
#include "render.h"


export
canvas_t *canvas_init(int w, int h, rgba_t *data)
{
    int size_data = data ? 0 : w * h * sizeof(rgba_t);
    int size = sizeof(canvas_t) + size_data;
    canvas_t *r = (canvas_t *)heap_alloc(size);

    r->data = data ? data : (rgba_t *)(r + 1);
    r->w = w;
    r->h = h;

    return r;
}

export
void draw_line(scene_t *s, point_t a, point_t b, void *shf, mat_t *mat)
{
    int dx = abs(a.x - b.x);
    int dy = abs(a.y - b.y);
    int steep = dy > dx;

    if (steep)
    {
        int tmp = a.x;
        a.x = a.y;
        a.y = tmp;

        tmp = b.x;
        b.x = b.y;
        b.y = tmp;

        dx = abs(a.x - b.x);
        dy = abs(a.y - b.y);
    }

    if (a.x > b.x)
    {
        point_t tmp = a;
        a = b;
        b = tmp;

        dx = abs(a.x - b.x);
        dy = abs(a.y - b.y);
    }

    int err = 0;
    int derr = dy;
    int y = a.y;
    int sy = sign(b.y - a.y);

    int cx, cy;
    pixel_t pix;
    evertex_t v;

    for (
        int x = a.x;
        x <= min(b.x, (steep ? s->canv->h : s->canv->w) - 1) &&
        y < (steep ? s->canv->w : s->canv->h);
        x++)
    {
        cx = steep ? y : x;
        cy = steep ? x : y;
        v.v.x = cx;
        v.v.y = cy;
        v.v.z = 0;
        v.n.x = 0;
        v.n.y = 0;
        v.n.z = 0;
        v.c = mat->ambient;

        if (cx >= 0 && cy >= 0)
        {
            pix = ((shader_f_t)shf)(v, mat, s);
            SET_PIXEL(s->canv, pix.pos.x, pix.pos.y, &pix.col);
        }

        err += derr;
        if (2 * err >= dx)
        {
            y += sy;
            err -= dx;
        }
    }
}

void set_pixel(canvas_t *canv, int16_t x, int16_t y, rgba_t *color)
{
    SET_PIXEL(canv, x, y, color);
}

void set_pixel_z(canvas_t *canv, zbuf_t *zbuf, int16_t x, int16_t y, int16_t z, rgba_t *color)
{
    SET_PIXEL_Z(canv, zbuf, x, y, z, color);
}

rgba_t rgba_scale3(rgba_t a, float k)
{
    return (rgba_t){
        a.r * k,
        a.g * k,
        a.b * k,
        a.a
    };
}
