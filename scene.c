#include <webassembly.h>

#include "scene.h"


export
scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint8_t *zbuf)
{
    int canv_s = canv ? 0 : sizeof(canvas_t);
    int zbuf_s = zbuf ? 0 : sizeof(zbuf_t);

    scene_t *s = heap_alloc(sizeof(scene_t));
    char *after = (char *)(s + 1);

    s->ls = NULL;
    s->ll = 0;

    s->canv = canvas_init(w, h, canv ? canv : (rgba_t *)(after + canv_s));
    s->zbuf = zbuf_init(w, h, zbuf ? zbuf : (uint8_t *)(after + canv_s + zbuf_s));

    s->bmask_row = bitmask_init(w, 1);

    return s;
}

export
void scene_free(scene_t *s)
{
    bitmask_free(s->bmask_row);
    heap_free(s);
}

export
zbuf_t *zbuf_init(int w, int h, uint8_t *data)
{
    int size_data = data ? 0 : w * h * sizeof(uint8_t);
    int size = sizeof(zbuf_t) + size_data;
    zbuf_t *r = (zbuf_t *)heap_alloc(size);

    r->data = data ? data : (uint8_t *)(r + 1);
    r->w = w;
    r->h = h;

    return r;
}

export void clear(scene_t *s)
{
    rgba_t *it = (rgba_t *)s->canv->data;

    for (int i = 0; i < s->canv->h * s->canv->w; i++, it++)
    {
        it->a = 0;
    }
}

export
void frame(scene_t *s)
{
    static int off = 0;
    vertex_t vs[3];
    vs[0].x=200 + round(99*cos((1.3*off) * 3.14/180)); vs[0].y=100 + round(10*cos((4*off) * 3.14/180)); vs[0].z=0;
    vs[1].x=400 + round(199*sin((-off) * 3.14/180)); vs[1].y=500 + round(15*cos((-2*off) * 3.14/180)); vs[1].z=0;
    vs[2].x=700 + round(300*cos((off) * 3.14/180)); vs[2].y=450 - round(100*cos((1.3*off) * 3.14/180)); vs[2].z=0;

    draw_fragment(s, vs);
    off++;
}

export
int16_t find_line_x(point_t a, point_t b, int16_t y)
{
    if ((a.y <= b.y && (a.y > y || b.y < y)) ||
        (a.y > b.y && (b.y > y || a.y < y)))
        return INT16_MIN;

    if (a.y == y)
        return a.x;

    if (b.y == y)
        return b.x;

    int dx = b.x - a.x;
    int dy = b.y - a.y;

    if (dx == 0)
        return min(a.x, b.x);

    dx = abs(a.x - b.x);
    dy = abs(a.y - b.y);
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
    int yi = a.y;
    int sy = sign(b.y - a.y);

    for (int x = a.x; x <= b.x; x++)
    {
        if ((steep && x == y) || (!steep && yi == y)) return steep ? yi : x;
        err += derr;
        if (2 * err >= dx)
        {
            yi += sy;
            err -= dx;
        }
    }

    return (int)round(a.x + ((double)((y - a.y) * dx))/dy);
}
