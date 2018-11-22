#include <webassembly.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>

#include "models.h"
#include "canvas.h"
#include "heap.h"

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

canvas_t *canvas_init(int w, int h)
{
    int size = sizeof(canvas_t) + w * h * sizeof(rgba_t);
    return heap_alloc(size);
}

bitmask_t *bitmask_init(int w, int h)
{
    int bw = w / 8 + sign(w % 8);
    int bh = h / 8 + sign(h % 8);
    bitmask_t *ptr = heap_alloc(sizeof(bitmask_t) + bw * bh);
    ptr->w = bw;
    ptr->h = bh;
    ptr->data = (char *)(ptr + 1);
    return ptr;
}

void bitmask_free(bitmask_t *ptr)
{
    heap_free(ptr);
}

void bitmask_invert(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = (uint8_t)((1 << (a_bit + 1)) - 1) & 0xff ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    //console_log("inv %u %u b%u e%u", a_mask, b_mask, a_byte, b_byte);

    char *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] ^= a_mask;
        it[b_byte] ^= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = ~it[i] & 0xff;
            //console_log("inv %u", it[i]);
        }
    }
}

void bitmask_set(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = (uint8_t)((1 << (a_bit + 1)) - 1) & 0xff ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    char *it = bmask->data + bmask->w * a.y;
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

void bitmask_unset(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = (uint8_t)((1 << (a_bit + 1)) - 1) & 0xff ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    char *it = bmask->data + bmask->w * a.y;
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

void log_bitmask_row(bitmask_t *bmask, int row)
{
    char *str = heap_alloc(bmask->w * 8 + 1);
    for (int byte = 0; byte < bmask->w; byte++)
        for (int bit = 0; bit < 8; bit++)
            str[byte * 8 + bit] = bmask->data[bmask->w * row + byte] & (1 << bit) ? '1' : '0';
    str[bmask->w * 8] = 0;
    //logs(str);
    //console_log("%s", str);
    heap_free(str);
}

rgba_t f_shader_inter(pixel_t a, pixel_t b, point_t p)
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
    canvas_t *canv, pixel_t pa, pixel_t pb,
    rgba_t (*f_shader)(pixel_t, pixel_t, point_t))
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
        color = f_shader(pa, pb, point);
        SET_PIXEL(canv, cx, cy, &color);
        err += derr;
        if (2 * err >= dx)
        {
            y += sy;
            err -= dx;
        }
    }
}

int find_line_x(point_t a, point_t b, int y)
{
    if ((a.y <= b.y && (a.y > y || b.y < y)) ||
        (a.y > b.y && (b.y > y || a.y < y)))
        return UINT16_MAX;

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

    return round(a.x + ((double)((y - a.y) * dx))/dy);
}

export
void draw_triangle(canvas_t *canv, vertex_t *vs)
{
    pixel_t a, b;

    rgba_t c1 = {255, 0, 0, 128}, c2 = {0, 255, 0, 128}, c3 = {0, 0, 255, 128};

    a.pos.y = vs[0].y;
    a.pos.x = vs[0].x;
    b.pos.y = vs[1].y;
    b.pos.x = vs[1].x;
    a.col = c1;
    b.col = c2;
    draw_line(canv, a, b, NULL);

    a.pos.y = vs[2].y;
    a.pos.x = vs[2].x;
    a.col = c3;
    draw_line(canv, a, b, NULL);

    b.pos.y = vs[0].y;
    b.pos.x = vs[0].x;
    b.col = c1;
    draw_line(canv, a, b, NULL);

    int min_x = min(min(vs[0].x, vs[1].x), vs[2].x);
    int min_y = min(min(vs[0].y, vs[1].y), vs[2].y);
    
    int max_x = max(max(vs[0].x, vs[1].x), vs[2].x);
    int max_y = max(max(vs[0].y, vs[1].y), vs[2].y);

    bitmask_t *bmask = bitmask_init(canv->w, 1);
    point_t ba = {0, 0}, bb = {bmask->w * 8, 1};
    point_t pa, pb;
    int x_int;


    for (int y = min_y; y < max_y; y++)
    //for (int y = min_y + 60; y < min_y + 65; y++)
    {
        ba.x = 0;
        bitmask_unset(bmask, ba, bb);
        int dirs[3] = {0};
        int xs[3] = {0};
        for (int i = 0; i < 3; i++)
        {
            pa.x = vs[i].x;
            pa.y = vs[i].y;
            pb.x = vs[(i + 1) % 3].x;
            pb.y = vs[(i + 1) % 3].y;

            x_int = find_line_x(pa, pb, y);
            dirs[i] = pb.y - pa.y;
            xs[i] = x_int;
            if (x_int == UINT16_MAX) continue;
            //SET_PIXEL(canv, x_int, y, styles[0]);
            //DOT(canv, x_int, y, styles[0]);
            ba.x = x_int;
            //console_log("Y %d X %d i %d", y, x_int, i);
            bitmask_invert(bmask, ba, bb);
        }

        if ((xs[0] == xs[1]) && (xs[0] != UINT16_MAX) && (dirs[0] * dirs[1] >= 0))
        {
            ba.x = xs[0];
            bitmask_invert(bmask, ba, bb);
            //console_log("(0 1) %d", dirs[0] * dirs[1]);
        }

        if ((xs[0] == xs[2]) && (xs[0] != UINT16_MAX) && (dirs[0] * dirs[2] >= 0))
        {
            ba.x = xs[0];
            bitmask_invert(bmask, ba, bb);
            //console_log("(0 2) %d", dirs[0] * dirs[2]);
        }

        if ((xs[2] == xs[1]) && (xs[2] != UINT16_MAX) && (dirs[2] * dirs[1] >= 0))
        {
            ba.x = xs[2];
            bitmask_invert(bmask, ba, bb);
            //console_log("(1 2) %d", dirs[2] * dirs[1]);
        }

        //console_log("%d %d %d", dirs[0], dirs[1], dirs[2]);


        //log_bitmask_row(bmask, 0);

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
                    a.col = /**(rgba_t *)black;*/*GET_PIXEL(canv, begin - 1, y);
                    b.col = *GET_PIXEL(canv, end - 1, y);
                    draw_line(canv, a, b, NULL);
                    begin = -1;
                    end = -1;
                }
                continue;
            }

            //if (!f)
            //    console_log("B %d", byte * 8);
            //f = 1;

            if (bmask->data[byte] == 0xff)
            {
                if (begin == -1)
                {
                    begin = byte * 8;
                    end = byte * 8 + 8;
                }

                for (int bit = 0; bit < 8; bit++)
                {
                    //SET_PIXEL(canv, byte * 8 + bit, y, styles[0]);
                }
            }
            else
            for (int bit = 0; bit < 8; bit++)
            {
                //if ((bmask->data[byte] & (1 << bit)) == 0) continue;
                //SET_PIXEL(canv, byte * 8 + bit, y, black);
                if ((bmask->data[byte] & (1 << bit)) == 0)
                {
                    if (begin != -1)
                    {
                        end = byte * 8 + bit;
                        a.pos.y = y;
                        a.pos.x = begin;
                        b.pos.y = y;
                        b.pos.x = end;
                        a.col = /**(rgba_t *)black;*/*GET_PIXEL(canv, begin - 1, y);
                        b.col = *GET_PIXEL(canv, end - 1, y);
                        draw_line(canv, a, b, NULL);
                        //DOT(canv, end, y, styles[0]);
                        begin = -1;
                        end = -1;
                    }
                }
                else
                {
                    //SET_PIXEL(canv, byte * 8 + bit, y, black);
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
            //console_log("%u", bmask->data[byte]);

            /*if (begin != -1 && end != -1 && byte == bmask->w)
            {
                a.pos.y = y;
                a.pos.x = begin;
                b.pos.y = y;
                b.pos.x = end;
                a.col = *GET_PIXEL(canv, begin - 1, y);
                b.col = *(rgba_t *)black;*GET_PIXEL(canv, end - 1, y);
                draw_line(canv, a, b, NULL);
                begin = -1;
                end = -1;
            }*/
        }
    }

    bitmask_free(bmask);
}


export
void fillit(int x1, int y1, int x2, int y2)
{
    for (int x = x1; x <= x2; x++)
        for (int y = y1; y <= y2; y++)
            if ((x + y) % 2 == 0) fillRect(x, y, 1, 1);
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
            it[3] = (x+y+off) % (2+y) == 0 ? 255: 0;
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

    draw_triangle(&canvas, vs);
    vs[1].x=800 - round(70*sin((-off) * 3.14/180)); vs[1].y=300 + round(15*cos((2*off) * 3.14/180)); vs[1].z=0;
    draw_triangle(&canvas, vs);
    vs[2].x=900 + round(20*cos((off) * 3.14/180)); vs[2].y=100 - round(30*cos((1.3*off) * 3.14/180)); vs[2].z=0;
    draw_triangle(&canvas, vs);
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