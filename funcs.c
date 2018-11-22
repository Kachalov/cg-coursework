#include <webassembly.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>

#include "models.h"
#include "canvas.h"
#include "heap.h"
#include "scene.h"

#define DOT(c, x, y, p) {\
SET_PIXEL(c, x, y, p); \
SET_PIXEL(c, x+1, y, p); \
SET_PIXEL(c, x, y+1, p); \
SET_PIXEL(c, x+1, y+1, p); }

char styles[][4] = {{179, 35, 35, 255}, {0, 100, 80, 255}, {11, 175, 13, 255}, {11, 35, 176, 255}, {255, 128, 255, 255}};


char black[] = {0, 0, 0, 255};

// ====================== math.c  ====================== //

int sign (int x)
{
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

double pow2 (float x, int e)
{
    return x * x;
}

int int_inter(int a, int b, float t)
{
    return (b - a) * t + a;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

// ====================== models.c  ====================== //

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

export
bitmask_t *bitmask_init(int w, int h)
{
    int bw = w / 8 + sign(w % 8);
    int bh = h / 8 + sign(h % 8);
    bitmask_t *ptr = heap_alloc(sizeof(bitmask_t) + bw * bh);
    ptr->w = bw;
    ptr->h = bh;
    ptr->data = (uint8_t *)(ptr + 1);
    return ptr;
}

export
void bitmask_free(bitmask_t *ptr)
{
    heap_free(ptr);
}

export
void bitmask_invert(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] ^= a_mask;
        it[b_byte] ^= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = ~it[i] & 0xff;
        }
    }
}

export
void bitmask_set(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] |= a_mask;
        it[b_byte] |= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = 0xff;
        }
    }
}

export
void bitmask_unset(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] &= a_mask;
        it[b_byte] &= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = 0;
        }
    }
}

export
void log_bitmask_row(bitmask_t *bmask, int row)
{
    char *str = heap_alloc(bmask->w * 8 + 1);
    for (int byte = 0; byte < bmask->w; byte++)
        for (int bit = 0; bit < 8; bit++)
            str[byte * 8 + bit] = bmask->data[bmask->w * row + byte] & (1 << bit) ? '1' : '0';
    str[bmask->w * 8] = 0;
    heap_free(str);
}

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

    return s;
}

export
void scene_free(scene_t *s)
{
    heap_free(s);
}

export
rgba_t f_shader_inter(pixel_t a, pixel_t b, point_t p, scene_t *s)
{
    //float ra, rb, t;
    //ra = sqrt(pow2(p.x - a.pos.x, 2) + pow2(p.y - a.pos.y, 2));
    //rb = sqrt(pow2(p.x - b.pos.x, 2) + pow2(p.y - b.pos.y, 2));

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
int find_line_x(point_t a, point_t b, int y)
{
    if ((a.y <= b.y && (a.y > y || b.y < y)) ||
        (a.y > b.y && (b.y > y || a.y < y)))
        return -1;

    if (a.y == y)
        return a.x;

    if (b.y == y)
        return b.x;

    int dx = b.x - a.x;
    int dy = b.y - a.y;

    if (dx == 0)
        return min(a.x, b.x);


    //goto skip;
    ///////////
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
    ///////////
    skip:

    return (int)round(a.x + ((double)((y - a.y) * dx))/dy);
}

export void draw_triangle(scene_t *s, pixel_t *ps);
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
void bufdraw(char *buf, int w, int h)
{
    static int off = 0;
    canvas_t canvas = {.data = (rgba_t *)buf, .w = w, .h = h};
    // XXX (15.11.18): memset
    vertex_t vs[3];
    vs[0].x=200 + round(99*cos((1.3*off) * 3.14/180)); vs[0].y=100 + round(10*cos((4*off) * 3.14/180)); vs[0].z=0;
    vs[1].x=400 + round(199*sin((-off) * 3.14/180)); vs[1].y=500 + round(15*cos((-2*off) * 3.14/180)); vs[1].z=0;
    vs[2].x=700 + round(300*cos((off) * 3.14/180)); vs[2].y=450 - round(100*cos((1.3*off) * 3.14/180)); vs[2].z=0;

    char pix[] = {0, 0, 0, 255};
    SET_PIXEL(&canvas, 10, 10, pix);
    char *it = buf;
    for (int y = 0; y < h; y++)
        for (int x = 0; x < w; x++, it+=4) {
            //if (x == y + off)SET_PIXEL_PTR(it, styles[0]);
            SET_PIXEL_PTR(it, styles[(x+y+off) % (sizeof(styles)/sizeof(styles)[0])]);
            //for (int k = 0; k < 3; k++)

            //    it[k] = styles[(i+j+off) % (sizeof(styles)/sizeof(styles)[0])][k];
            it[3] = (x+y+off) % (2+y) == 0 ? 0xff: 0;
        }
    off++;

    int y0 = 300;
    int a = 40;
    int y = 0, y2 = 0, y3= 0;
    for (int x = 0; x < w-1; x++)
    {

        y = y0 + round(a*cos((x+1.3*off) * 3.14/180));
        y2 = y0+10 + round(a*cos((x+1.15*off) * 3.14/180));
        y3 = y0+20 + round(a*cos((x+off) * 3.14/180));
        DOT(&canvas, x, y, styles[0]);
        DOT(&canvas, x, y2, styles[3]);
        DOT(&canvas, x, y3, styles[2]);
    }

    //draw_fragment(&canvas, vs);
    vs[1].x=800 - round(70*sin((-off) * 3.14/180)); vs[1].y=300 + round(15*cos((2*off) * 3.14/180)); vs[1].z=0;
    //draw_fragment(&canvas, vs);
    vs[2].x=900 + round(20*cos((off) * 3.14/180)); vs[2].y=100 - round(30*cos((1.3*off) * 3.14/180)); vs[2].z=0;
    //draw_fragment(&canvas, vs);
}

// ====================== heap.c  ====================== //

export
void *heap_alloc(int size)
{
    return malloc(size);
}

export
void heap_free(void *ptr)
{
    free(ptr);
}

// ====================== models.c  ====================== //

obj_t obj_init()
{
    obj_t obj;

    obj.vl = 0;
    obj.vs = NULL;

    obj.fl = 0;
    obj.fs = NULL;

    return obj;
}