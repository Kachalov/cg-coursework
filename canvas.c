#include <webassembly.h>

#include "canvas.h"
#include "math.h"
#include "shaders.h"
#include "heap.h"
#include "render.h"


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

void draw_line_3d(scene_t *s, evertex_t a, evertex_t b, void *shf, const mat_t *mat)
{
    int dx = fabsf(a.v.x - b.v.x);
    int dy = fabsf(a.v.y - b.v.y);
    int steep = dy > dx;

    if (dx == dy && dy == 0)
        return;

    if (steep)
    {
        float tmp = a.v.x;
        a.v.x = a.v.y;
        a.v.y = tmp;

        tmp = b.v.x;
        b.v.x = b.v.y;
        b.v.y = tmp;

        dx = fabsf(a.v.x - b.v.x);
        dy = fabsf(a.v.y - b.v.y);
    }

    if (a.v.x > b.v.x)
    {
        evertex_t tmp = a;
        a = b;
        b = tmp;

        dx = fabsf(a.v.x - b.v.x);
        dy = fabsf(a.v.y - b.v.y);
    }

    int err = 0;
    int derr = dy;
    int y = a.v.y;
    int sy = sign(b.v.y - a.v.y);

    int cx, cy;
    pixel_t pix;
    evertex_t v;
    float t;

    for (
        int x = a.v.x;
        x <= min(b.v.x, (steep ? s->canv->h : s->canv->w) - 1) &&
        y < (steep ? s->canv->w : s->canv->h);
        x++)
    {
        t = (x - a.v.x) * 1.0 / (b.v.x - a.v.x);
        cx = steep ? y : x;
        cy = steep ? x : y;
        v.v.x = cx;
        v.v.y = cy;
        v.v.z = float_inter(a.v.z, b.v.z, t);
        v.n.x = float_inter(a.n.x, b.n.x, t);
        v.n.y = float_inter(a.n.y, b.n.y, t);
        v.n.z = float_inter(a.n.z, b.n.z, t);
        v.c.r = clamp(float_inter(a.c.r, b.c.r, t), 0, 255);
        v.c.g = clamp(float_inter(a.c.g, b.c.g, t), 0, 255);
        v.c.b = clamp(float_inter(a.c.b, b.c.b, t), 0, 255);
        v.c.a = clamp(float_inter(a.c.a, b.c.a, t), 0, 255);

        if (cx >= 0 && cy >= 0)
        {
            pix = ((shader_f_t)shf)(v, mat, s);
            if (pix.pos.x >= 0 && pix.pos.x < s->canv->w && pix.pos.y >= 0 && pix.pos.y < s->canv->h &&
                (v.v.z >= s->perspective_props.near && v.v.z <= s->perspective_props.far))
                SET_PIXEL_Z(s->canv, s->zbuf, pix.pos.x, pix.pos.y, ((v.v.z)/(s->perspective_props.far - s->perspective_props.near)*ZBUF_DEPTH), &pix.col);
        }

        err += derr;
        if (2 * err >= dx)
        {
            y += sy;
            err -= dx;
        }
    }
}

void draw_line(scene_t *s, point_t a, point_t b, void *shf, const mat_t *mat)
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
        clamp(k * a.r, 0, 255),
        clamp(k * a.g, 0, 255),
        clamp(k * a.b, 0, 255),
        a.a
    };
}

rgba_t int2rgba(int rgba)
{
    return (rgba_t){
        (rgba >> 24) & 255,
        (rgba >> 16) & 255,
        (rgba >> 8) & 255,
        (rgba) & 255
    };
}

int rgba2int(rgba_t rgba)
{
    return (rgba.r << 24) + (rgba.g << 16) + (rgba.b << 8) + (rgba.a);
}
