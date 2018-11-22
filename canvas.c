#include <webassembly.h>

#include "canvas.h"


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
void draw_line(
    scene_t *s, pixel_t pa, pixel_t pb,
    f_shader_t f_shader)
{
    point_t a = pa.pos;
    point_t b = pb.pos;

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

        pixel_t tmpp = pa;
        pa = pb;
        pb = tmpp;

        dx = abs(a.x - b.x);
        dy = abs(a.y - b.y);
    }

    int err = 0;
    int derr = dy;
    int y = a.y;
    int sy = sign(b.y - a.y);

    if (!f_shader)
        f_shader = f_shader_inter;

    int cx, cy;
    rgba_t color;
    point_t point;
    for (int x = a.x; x <= b.x; x++)
    {
        cx = steep ? y : x;
        cy = steep ? x : y;
        point.x = cx;
        point.y = cy;
        color = f_shader(pa, pb, point, s);
        SET_PIXEL(s->canv, cx, cy, &color);
        err += derr;
        if (2 * err >= dx)
        {
            y += sy;
            err -= dx;
        }
    }
}

export
void draw_fragment(scene_t *s, vertex_t *vs)
{
    static int off = 0;
    pixel_t p[3];
    rgba_t c[3] = {
        {255, (0+125+(int)round(100*cos(1.*off/10)))&0xff, 0, 255-128-(int)round(127*cos(1.*off/50))},
        {0, 255, (0+125+(int)round(100*cos(1.*off/15 + 10)))&0xff, 255},
        {(0+125+(int)round(125*sin(1.*off/20)))&0xff, 0, 255, 255}
    };

    for (int i = 0; i < 3; i++)
    {
        p[i].pos.x = vs[i].x;
        p[i].pos.y = vs[i].y;
        p[i].col = c[i];
    }

    draw_triangle(s, p);
    off++;
}

export
void draw_triangle(scene_t *s, pixel_t *ps)
{
    draw_line(s, ps[0], ps[1], NULL);
    draw_line(s, ps[0], ps[2], NULL);
    draw_line(s, ps[2], ps[1], NULL);

    int min_x = min(min(ps[0].pos.x, ps[1].pos.x), ps[2].pos.x);
    int min_y = min(min(ps[0].pos.y, ps[1].pos.y), ps[2].pos.y);

    int max_x = max(max(ps[0].pos.x, ps[1].pos.x), ps[2].pos.x);
    int max_y = max(max(ps[0].pos.y, ps[1].pos.y), ps[2].pos.y);

    bitmask_t *bmask = bitmask_init(s->canv->w, 1);
    point_t ba = {0, 0}, bb = {bmask->w * 8, 1};
    point_t pa, pb;
    pixel_t a, b;
    int x_int;


    for (int y = min_y; y < max_y; y++)
    {
        ba.x = 0;
        bitmask_unset(bmask, ba, bb);
        int dirs[3] = {0};
        int xs[3] = {0};
        for (int i = 0; i < 3; i++)
        {
            pa.x = ps[i].pos.x;
            pa.y = ps[i].pos.y;
            pb.x = ps[(i + 1) % 3].pos.x;
            pb.y = ps[(i + 1) % 3].pos.y;

            x_int = find_line_x(pa, pb, y);
            dirs[i] = pb.y - pa.y;
            xs[i] = x_int;
            if (x_int == -1) continue;
            ba.x = x_int;
            bitmask_invert(bmask, ba, bb);
        }

        if ((xs[0] == xs[1]) && (xs[0] != -1) && (dirs[0] * dirs[1] >= 0))
        {
            ba.x = xs[0];
            bitmask_invert(bmask, ba, bb);
        }

        if ((xs[0] == xs[2]) && (xs[0] != -1) && (dirs[0] * dirs[2] >= 0))
        {
            ba.x = xs[0];
            bitmask_invert(bmask, ba, bb);
        }

        if ((xs[2] == xs[1]) && (xs[2] != -1) && (dirs[2] * dirs[1] >= 0))
        {
            ba.x = xs[2];
            bitmask_invert(bmask, ba, bb);
        }


        int begin = -1;
        int end = -1;
        for (int byte = 0; byte < bmask->w; byte++)
        {
            if (bmask->data[byte] == 0)
            {
                if (begin != -1)
                {
                    end = byte * 8;
                    a.pos.y = y;
                    a.pos.x = begin;
                    b.pos.y = y;
                    b.pos.x = end;
                    a.col = *GET_PIXEL(s->canv, begin - 1, y);
                    b.col = *GET_PIXEL(s->canv, end - 1, y);
                    draw_line(s, a, b, NULL);
                    begin = -1;
                    end = -1;
                }
                continue;
            }

            if (bmask->data[byte] == 0xff)
            {
                if (begin == -1)
                {
                    begin = byte * 8;
                    end = byte * 8 + 8;
                }
            }
            else
            for (int bit = 0; bit < 8; bit++)
            {
                if ((bmask->data[byte] & (1 << bit)) == 0)
                {
                    if (begin != -1)
                    {
                        end = byte * 8 + bit;
                        a.pos.y = y;
                        a.pos.x = begin;
                        b.pos.y = y;
                        b.pos.x = end;
                        a.col = *GET_PIXEL(s->canv, begin - 1, y);
                        b.col = *GET_PIXEL(s->canv, end - 1, y);
                        draw_line(s, a, b, NULL);
                        begin = -1;
                        end = -1;
                    }
                }
                else
                {
                    if (begin == -1)
                    {
                        begin = byte * 8 + bit;
                        end = byte * 8 + bit;
                    }
                    else
                    {
                        end = byte * 8 + bit;
                    }
                }
            }
        }
    }

    bitmask_free(bmask);
}
