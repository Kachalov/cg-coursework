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
    return;

    s->models.d[0]->fs.d[0].l = 3;
    s->models.d[0]->fs.l = 3;

    s->models.d[0]->vs.d[0].x=200 + round(99*cos((1.3*off) * 3.14/180));
    s->models.d[0]->vs.d[0].y=100 + round(10*cos((4*off) * 3.14/180));
    s->models.d[0]->vs.d[0].z=50 + round(10*cos((3*off) * 3.14/180));

    s->models.d[0]->vs.d[1].x=400 + round(199*sin((-off) * 3.14/180));
    s->models.d[0]->vs.d[1].y=500 + round(15*cos((-2*off) * 3.14/180));
    s->models.d[0]->vs.d[1].z=500 - round(100*cos((1.3*off) * 3.14/180));

    s->models.d[0]->vs.d[2].x=700 + round(300*cos((off) * 3.14/180));
    s->models.d[0]->vs.d[2].y=450 - round(100*cos((1.3*off) * 3.14/180));
    s->models.d[0]->vs.d[2].z=50 + round(199*sin((-off) * 3.14/180));

    /*s->models.d[1]->fs.d[0].l = 3;

    s->models.d[1]->vs.d[0].x=200 + round(99*cos((1.3*off) * 3.14/180));
    s->models.d[1]->vs.d[0].y=100 + round(10*cos((4*off) * 3.14/180));
    s->models.d[1]->vs.d[0].z=50 + round(10*cos((3*off) * 3.14/180));

    s->models.d[1]->vs.d[1].x=400 + round(199*sin((-off) * 3.14/180));
    s->models.d[1]->vs.d[1].y=500 + round(15*cos((-2*off) * 3.14/180));
    s->models.d[0]->vs.d[1].z=500 - round(100*cos((1.3*off) * 3.14/180));

    s->models.d[1]->vs.d[2].x=700 + round(300*cos((off) * 3.14/180));
    s->models.d[1]->vs.d[2].y=450 - round(100*cos((1.3*off) * 3.14/180));
    s->models.d[1]->vs.d[2].z=50 + round(199*sin((-off) * 3.14/180));*/

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
            vs[i].v = model->vs.d[face->v[i]];
            /*console_log("%lf %lf %lf", model->vs.d[face->v[i]].x, model->vs.d[face->v[i]].y, model->vs.d[face->v[i]].z);
            console_log("%d", face->v[i]);*/
            if (face->n[i] != -1)
                vs[i].n = model->ns.d[face->n[i]];
        }

        for (int i = 0; i < 3; i++)
        {
            if (model->props.shaders.v)
                vs[i] = model->props.shaders.v(vs, i, s);
            console_log("%lf %lf %lf", vs[i].v.x, vs[i].v.y, vs[i].v.z);
            vs[i] = world2viewport(vs[i], s);
            console_log("%lf %lf %lf", vs[i].v.x, vs[i].v.y, vs[i].v.z);
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

/**
 * @param s Scene.
 * @param vs 3 y-sorted extended vertices.
 * @param shf Fragment shader function.
 * @param mat Matertial.
 */
void draw_triangle(scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat)
{
    evertex_t tmp;
    yield_evertex_t ya, yb, ytmp;
    console_log("X %lf %lf %lf", vs[0].v.x, vs[1].v.x, vs[2].v.x);
    console_log("Y %lf %lf %lf", vs[0].v.y, vs[1].v.y, vs[2].v.y);

    int min_y = min(s->canv->h, max(0, vs[0].v.y));
    int mid_y = min(s->canv->h, max(0, vs[1].v.y));
    int max_y = min(s->canv->h, max(0, vs[2].v.y));

    ya = yield_evertex_init(vs[0], vs[1], vs[0].v.y - vs[1].v.y);
    yb = yield_evertex_init(vs[0], vs[2], vs[0].v.y - vs[2].v.y);

    //ya.dev.v.y = 1;
    //yb.dev.v.y = 1;

    ya.evc.r = 255;
    ya.evc.g = 0;
    ya.evc.b = 0;
    ya.evc.a = 255;
    ya.devc.r = ya.devc.g = ya.devc.b = ya.devc.a = 0;
    yb.evc.r = 255;
    yb.evc.g = 0;
    yb.evc.b = 0;
    yb.evc.a = 255;
    yb.devc.r = yb.devc.g = yb.devc.b = yb.devc.a = 0;
    /*console_log("X %lf %lf %lf", vs[0].v.x, vs[1].v.x, vs[2].v.x);
    console_log("Y %lf %lf %lf", vs[0].v.y, vs[1].v.y, vs[2].v.y);*/

    for (int y = min_y; y < mid_y; y++)
        draw_triangle_row(s, vs, shf, mat, y, &ya, &yb);

    tmp = vs[2];
    vs[2] = vs[0];
    vs[0] = tmp;

    ytmp = ya;
    ya = yield_evertex_init(vs[0], vs[1], vs[1].v.y - vs[0].v.y);
    //ya.t = ytmp.t;
    //ya.d = ytmp.d;
    //ya.ev = ytmp.ev;
    ya.evc = ytmp.evc;

    ytmp = yb;
    yb = yield_evertex_init(vs[0], vs[2], vs[2].v.y - vs[0].v.y);
    //yb.t = ytmp.t;
    //yb.d = ytmp.d;
    //yb.ev = ytmp.ev;
    yb.evc = ytmp.evc;

    //ya.dev.v.y = -1;
    //yb.dev.v.y = -1;

    ya.evc.r = 0;
    ya.evc.g = 255;
    ya.evc.b = 0;
    ya.evc.a = 255;
    ya.devc.r = ya.devc.g = ya.devc.b = ya.devc.a = 0;
    yb.evc.r = 0;
    yb.evc.g = 255;
    yb.evc.b = 0;
    yb.evc.a = 255;
    yb.devc.r = yb.devc.g = yb.devc.b = yb.devc.a = 0;
    /*console_log("X %lf %lf %lf", ya.ev.v.x, ya.ev.v.x, ya.ev.v.z);
    console_log("Y %lf %lf %lf", yb.ev.v.y, yb.ev.v.y, yb.ev.v.z);*/

    //for (int y = max_y; y >= mid_y; y--)
    for (int y = mid_y; y < max_y; y++)
        draw_triangle_row(s, vs, shf, mat, y, &ya, &yb);

    /*console_log("X %lf %lf %lf", ya.ev.v.x, ya.ev.v.x, ya.ev.v.z);
    console_log("Y %lf %lf %lf", yb.ev.v.y, yb.ev.v.y, yb.ev.v.z);*/

    /*pixel_t ps[3];

    for (int i = 0; i < 3; i++)
    {
        ps[i].pos.x = vs[i].v.x;
        ps[i].pos.y = vs[i].v.y;
    }

    draw_line(s, ps[0].pos, ps[1].pos, shf, mat);
    draw_line(s, ps[0].pos, ps[2].pos, shf, mat);
    draw_line(s, ps[2].pos, ps[1].pos, shf, mat);*/
}

void draw_triangle_row(
    scene_t *s, evertex_t *vs, shader_f_t shf, const mat_t *mat, int y,
    yield_evertex_t *ya, yield_evertex_t *yb)
{
    evertex_t va, vb, tmpv;
    evertex_t *ev;
    yield_evertex_t *yv;

    ev = &va;
    YIELD_EVERTEX(ev, ya);

    ev = &vb;
    YIELD_EVERTEX(ev, yb);

    vb.v.y += 1;

    if (va.v.x > vb.v.x)
    {
        tmpv = va;
        va = vb;
        vb = tmpv;
    }

    evertex_t v;
    v.v.y = y;
    pixel_t a;
    float t;

    yield_evertex_t yi;
    evertex_t vi;
    yi = yield_evertex_init(va, vb, va.v.x - vb.v.x);
    yi.dev.v.x = 1;
    yi.dev.v.y = 0;

    ev = &vi;
    yv = &yi;

    for (int i = max(0, va.v.x); i < min(vb.v.x - 1, s->canv->w - 1); i++)
    {
        YIELD_EVERTEX(ev, yv);

        /*if (yi.ev.v.z < 0)
        {
            if (yi.dev.v.z < 0)
                break;
            else
                continue;
        }*/

        a = shf(vi, mat, s);
        SET_PIXEL(s->canv, a.pos.x, a.pos.y, &a.col);
    }
}

yield_evertex_t yield_evertex_init(evertex_t a, evertex_t b, int steps)
{
    // TODO(27.11.18): Evertex normal
    yield_evertex_t yv;
    
    yv.d = steps == 0 ? 0 : 1.0 / steps;
    yv.t = 0;
    yv.ev = a;
    yv.evc.r = a.c.r;
    yv.evc.g = a.c.g;
    yv.evc.b = a.c.b;
    yv.evc.a = a.c.a;

    yv.dev.v.x = (a.v.x - b.v.x) * yv.d;
    yv.dev.v.y = (a.v.y - b.v.y) * yv.d;
    yv.dev.v.z = (a.v.z - b.v.z) * yv.d;
    yv.devc.r = (a.c.r - b.c.r) * yv.d;
    yv.devc.g = (a.c.g - b.c.g) * yv.d;
    yv.devc.b = (a.c.b - b.c.b) * yv.d;
    yv.devc.a = (a.c.a - b.c.a) * yv.d;
    
    return yv;
}

void yield_evertex(evertex_t *ev, yield_evertex_t *yv)
{
    YIELD_EVERTEX(ev, yv);
}
