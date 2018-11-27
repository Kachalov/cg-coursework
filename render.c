#include <math.h>

#include "render.h"
#include "math.h"
#include "bitmask.h"
#include "utils.h"


void frame(scene_t *s)
{
    model_t **model;

    model = s->models.d;
    for (int i = 0; i < s->models.l; i++, model++)
    {
        draw_model(s, *model);
    }

    // TODO (27.11.18): Remove test part
    static int off = 0;

    s->models.d[0]->fs.d[0].l = 3;

    s->models.d[0]->vs.d[0].x=200 + round(99*cos((1.3*off) * 3.14/180));
    s->models.d[0]->vs.d[0].y=100 + round(10*cos((4*off) * 3.14/180));
    s->models.d[0]->vs.d[0].z=50 + round(10*cos((3*off) * 3.14/180));

    s->models.d[0]->vs.d[1].x=400 + round(199*sin((-off) * 3.14/180));
    s->models.d[0]->vs.d[1].y=500 + round(15*cos((-2*off) * 3.14/180));
    s->models.d[0]->vs.d[1].z=100 - round(100*cos((1.3*off) * 3.14/180));

    s->models.d[0]->vs.d[2].x=700 + round(300*cos((off) * 3.14/180));
    s->models.d[0]->vs.d[2].y=450 - round(100*cos((1.3*off) * 3.14/180));
    s->models.d[0]->vs.d[2].z=50 + round(199*sin((-off) * 3.14/180));

    off++;
    // End of test part
}

void draw_model(scene_t *s, const model_t *model)
{
    face_t *face;

    face = model->fs.d;
    for (int i = 0; i < model->fs.l; i++, face++)
    {
        draw_model_face(s, model, face);
    }

    // TODO (15.11.2018): draw_model_line
}

void draw_model_face(scene_t *s, const model_t *model, const face_t *face)
{
    evertex_t vs[3];

    if (face->l == 3)
    {
        for (int i = 0; i < 3; i++)
        {
            vs[i].c = (rgba_t){255 * (i == 0), 255 * (i == 1), 255 * (i == 2), 255};
            vs[i].v = model->vs.d[face->d[i]];
            if (model->props.shaders.v)
                vs[i] = model->props.shaders.v(vs[i], s);
            vs[i] = world2viewport(vs[i], s);
        }

        draw_fragment(s, vs, &model->props);
    }
    else
    {
        // TODO (15.11.2018): yield_face_triangle
    }
}

void draw_fragment(scene_t *s, evertex_t *vs, const model_props_t *props)
{
    evertex_t vss[3], tmp;
    for (int i = 0; i < 3; i++)
        vss[i] = vs[i];

    if (vss[0].v.y > vss[1].v.y)
    {
        tmp = vss[0];
        vss[0] = vss[1];
        vss[1] = tmp;
    }

    if (vss[0].v.y > vss[2].v.y)
    {
        tmp = vss[0];
        vss[0] = vss[2];
        vss[2] = tmp;
    }

    if (vss[1].v.y > vss[2].v.y)
    {
        tmp = vss[1];
        vss[1] = vss[2];
        vss[2] = tmp;
    }

    draw_triangle(s, vss, props->shaders.f, &props->mat);
}

void draw_triangle(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat)
{
    evertex_t tmp;
    pixel_t ps[3];

    for (int i = 0; i < 3; i++)
    {
        ps[i].pos.x = vs[i].v.x;
        ps[i].pos.y = vs[i].v.y;
    }

    int min_y = min(s->canv->h, max(0, ps[0].pos.y));
    int mid_y = min(s->canv->h, max(0, ps[1].pos.y));
    int max_y = min(s->canv->h, max(0, ps[2].pos.y));

    for (int y = min_y; y < mid_y; y++)
    {
        draw_triangle_row(s, vs, shf, mat, y);
    }

    tmp = vs[2];
    vs[2] = vs[0];
    vs[0] = tmp;

    for (int y = mid_y; y < max_y; y++)
    {
        draw_triangle_row(s, vs, shf, mat, y);
    }

    /*draw_line(s, ps[0].pos, ps[1].pos, shf, mat);
    draw_line(s, ps[0].pos, ps[2].pos, shf, mat);
    draw_line(s, ps[2].pos, ps[1].pos, shf, mat);*/
}

void draw_triangle_row(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat, int y)
{
    pixel_t ps[3];
    evertex_t va, vb, tmpv;

    for (int i = 0; i < 3; i++)
    {
        ps[i].pos.x = vs[i].v.x;
        ps[i].pos.y = vs[i].v.y;
    }

    bitmask_t *bmask = s->bmask_row;
    point_t ba = {0, 0}, bb = {bmask->w * BITMASK_CHUNK_SIZE, 1};
    point_t pa, pb;
    int x_int;
    int dirs[3] = {0};
    int xs[3] = {0};
    float zs[3] = {0};

    /*bitmask_unset(bmask, ba, bb);

    for (int i = 0; i < 3; i++)
    {
        pa.x = ps[i].pos.x;
        pa.y = ps[i].pos.y;
        pb.x = ps[(i + 1) % 3].pos.x;
        pb.y = ps[(i + 1) % 3].pos.y;

        x_int = find_line_x(pa, pb, y);
        dirs[i] = pb.y - pa.y;
        xs[i] = x_int;
        if (x_int == INT16_MIN)
            continue;

        float t = 0;
        if (dirs[i] != 0)
            t = y / dirs[i];
        else if (pb.x - pa.x != 0)
            t = x_int / (pb.x - pa.x);
        else
        {
            return;
            console_log("(%lf %lf %lf) (%lf %lf %lf) (%lf %lf %lf)", vs[0].x, vs[0].y, vs[0].z,
            vs[1].x, vs[1].y, vs[1].z, vs[2].x, vs[2].y, vs[2].z);
            abort();
        }
        zs[i] = float_inter(vs[i].z, vs[(i + 1) % 3].z, t);

        ba.x = x_int >= 0 ? x_int : 0;
        bitmask_invert(bmask, ba, bb);
    }

    #define TRI_CHECK_EXTREMUM(m, n) \
    if ((xs[m] == xs[n]) && (xs[m] != INT16_MIN) && (dirs[m] * dirs[n] >= 0)) \
    { \
        ba.x = xs[m]; \
        bitmask_invert(bmask, ba, bb); \
    }

    TRI_CHECK_EXTREMUM(0, 1);
    TRI_CHECK_EXTREMUM(0, 2);
    TRI_CHECK_EXTREMUM(1, 2);
    #undef TRI_CHECK_EXTREMUM*/

    /*int x_int_a, x_int_b;

    pa.x = ps[0].pos.x;
    pa.y = ps[0].pos.y;
    pb.x = ps[1].pos.x;
    pb.y = ps[1].pos.y;
    x_int_a = find_line_x(pa, pb, y);
    if (x_int == INT16_MIN)
    {
        console_log("Unexpected intersection!");
        return;
    }

    pb.x = ps[2].pos.x;
    pb.y = ps[2].pos.y;
    x_int_b = find_line_x(pa, pb, y);
    if (x_int == INT16_MIN)
    {
        console_log("Unexpected intersection!");
        return;
    }*/

    /*if (x_int_a > x_int_b)
    {
        int tmp = x_int_a;
        x_int_a = x_int_b;
        x_int_b = tmp;
    }*/

    float d = vs[2].v.y - vs[0].v.y;
    if (fabsf(d) <= 1e-6)
    {
        console_log("Line!");
        return;
    }

    float t = (vs[2].v.y - y) / d;
    float t_delta = fabs(1. / d);

    vb.v.x = float_inter(vs[2].v.x, vs[0].v.x, t);
    vb.v.y = y;
    vb.v.z = float_inter(vs[2].v.z, vs[0].v.z, t);

    vb.c.r = float_inter(vs[2].c.r, vs[0].c.r, t);
    vb.c.g = float_inter(vs[2].c.g, vs[0].c.g, t);
    vb.c.b = float_inter(vs[2].c.b, vs[0].c.b, t);
    vb.c.a = float_inter(vs[2].c.a, vs[0].c.a, t);

    d = vs[1].v.y - vs[0].v.y;
    if (fabsf(d) <= 1e-6)
    {
        console_log("Line!");
        return;
    }

    t = (vs[1].v.y - y) / d;
    t_delta = fabs(1. / d);

    va.v.x = float_inter(vs[1].v.x, vs[0].v.x, t);
    va.v.y = y;
    va.v.z = float_inter(vs[1].v.z, vs[0].v.z, t);
    
    va.c.r = float_inter(vs[1].c.r, vs[0].c.r, t);
    va.c.g = float_inter(vs[1].c.g, vs[0].c.g, t);
    va.c.b = float_inter(vs[1].c.b, vs[0].c.b, t);
    va.c.a = float_inter(vs[1].c.a, vs[0].c.a, t);

    if (va.v.x > vb.v.x)
    {
        tmpv = va;
        va = vb;
        vb = tmpv;
    }

    /*ba.x = x_int_a;
    bb.x = x_int_b;
    bitmask_invert(bmask, ba, bb);*/

    //_draw_triangle_row_bitmask(s, va, vb, shf, mat, y, bmask);

    evertex_t v;
    v.v.y = y;
    pixel_t a;

    if (va.v.x != vb.v.x)
        for (int i = max(0, va.v.x); i <= min(vb.v.x, s->canv->w - 1); i++)
        {
            v.v.x = i;
            t = (va.v.x - v.v.x) / (va.v.x - vb.v.x);
            t = t > 1 ? 1 : t < 0 ? 0 : t;

            v.v.z = float_inter(va.v.z, vb.v.z, t);

            v.c.r = float_inter(va.c.r, vb.c.r, t);
            v.c.g = float_inter(va.c.g, vb.c.g, t);
            v.c.b = float_inter(va.c.b, vb.c.b, t);
            v.c.a = float_inter(va.c.a, vb.c.a, t);

            if (fpclassify(v.v.z) == FP_NAN)
            {
                console_log("t: %lf", t);
                console_log("(%lf %lf %lf) (%lf %lf %lf)",
                va.v.x, va.v.y, va.v.z, vb.v.x, vb.v.y, vb.v.z);
                abort();
            }

            a = shf(v, mat, s);
            SET_PIXEL(s->canv, a.pos.x, a.pos.y, &a.col);
        }
}

/*
void _draw_triangle_row_bitmask(scene_t *s, evertex_t va, vertex_t vb, shader_f_t shf, mat_t *mat, int y, bitmask_t *bmask)
{
    pixel_t a, b;
    int begin = -1;
    int end = -1;
    float t;

    a.pos.y = b.pos.y = y;

    #define TRI_BMASK_DRAW \
    a.pos.x = begin; \
    b.pos.x = end - 1; \
    draw_line(s, a.pos, b.pos, shf, mat);

    for (int byte = 0; byte < bmask->w; byte++)
    {
        if (bmask->data[byte] == 0)
        {
            if (begin != -1)
            {
                end = byte * BITMASK_CHUNK_SIZE;
                //TRI_BMASK_DRAW;
                return;
            }
        }
        else
        if (bmask->data[byte] == BITMASK_CHUNK_MASK)
        {
            if (begin == -1)
            {
                begin = byte * BITMASK_CHUNK_SIZE;
                end = (byte + 1) * BITMASK_CHUNK_SIZE;
            }

            for (int i = 0; i < BITMASK_CHUNK_SIZE; i++)
            {
                vertex_t v;
                v.x = byte * BITMASK_CHUNK_SIZE + i;
                v.y = y;
                t = fabsf(va.y - vb.y) < 1e-6 ? y / (va.y - vb.y) : v.x / (va.x - vb.x);
                v.z = float_inter(va.z, vb.z, t);
                a = shf(v, mat, s);
                SET_PIXEL(s->canv, a.pos.x, a.pos.y, &a.col);
            }
        }
        else
        for (int bit = 0; bit < BITMASK_CHUNK_SIZE; bit++)
        {
            if ((bmask->data[byte] & (1 << bit)) == 0)
            {
                if (begin != -1)
                {
                    end = byte * BITMASK_CHUNK_SIZE + bit;
                    //TRI_BMASK_DRAW;
                    return;
                }
            }
            else
            {
                if (begin == -1)
                    begin = byte * BITMASK_CHUNK_SIZE + bit;
                end = byte * BITMASK_CHUNK_SIZE + bit;

                vertex_t v;
                v.x = end;
                v.y = y;
                t = fabsf(va.y - vb.y) < 1e-6 ? y / (va.y - vb.y) : v.x / (va.x - vb.x);
                v.z = float_inter(va.z, vb.z, t);
                a = shf(v, mat, s);
                SET_PIXEL(s->canv, a.pos.x, a.pos.y, &a.col);
            }
        }
    }

    #undef TRI_BMASK_DRAW
}*/
