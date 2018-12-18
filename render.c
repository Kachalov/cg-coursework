#include <math.h>

#include "render.h"
#include "math.h"
#include "utils.h"
#include "shaders.h"


int render_mode;

void frame(scene_t *s, int render)
{
    model_t **model;
    render_mode = render;
    console_log("RENDER MODE: %d", render_mode);
    console_log("WIREFRAME: %d", render_mode & RENDER_WIREFRAME);

    if (render_mode & RENDER_GRID)
        draw_grid(s, 20, 26);
    draw_lights(s);

    model = s->models.d;
    for (int i = 0; i < s->models.l; i++, model++)
    {
        draw_model(s, *model);
    }

    if ((render_mode & RENDER_ZBUF) != 0)
        draw_zbuf(s);
}

void draw_grid(scene_t *s, int size, int count)
{
    evertex_t a, b;
    mat_t mat;

    a.v = (v3_t){0.0, 0, 0};
    a = world2viewport(a, s);

    mat.ambient = (rgba_t){0, 0, 255, 255};
    b.v = (v3_t){100, 0, 0};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, mat_shader_f, &mat);

    mat.ambient = (rgba_t){255, 0, 0, 255};
    b.v = (v3_t){0, 100, 0};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, mat_shader_f, &mat);

    mat.ambient = (rgba_t){255, 255, 0, 255};
    b.v = (v3_t){0, 0, 100};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, mat_shader_f, &mat);

    mat.ambient = (rgba_t){160, 160, 160, 255};
    for (int x = -count / 2; x <= count - (count / 2); x++)
    {
        a.v.z = b.v.z = 0;
        a.v.x = b.v.x = size * x;
        a.v.y = -size * (count / 2);
        b.v.y = size * (count - (count / 2));
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);
    }

    for (int y = -count / 2; y <= count - (count / 2); y++)
    {
        a.v.z = b.v.z = 0;
        a.v.y = b.v.y = size * y;
        a.v.x = -size * (count / 2);
        b.v.x = size * (count - (count / 2));
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);
    }

    mat.ambient = (rgba_t){255, 0, 0, 255};
    a.wv = a.v = (v3_t){0, 0, 200};
    a = world2viewport(a, s);
    a.v = v3_norm(v3_sub(a.wv, a.v));
    a = world2viewport(a, s);
    b.v.x = b.v.y = b.v.z = 0;
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, mat_shader_f, &mat);
}

void draw_lights(scene_t *s)
{
    evertex_t a, b;
    mat_t mat;

    const int r1 = 25, r2 = 10;

    for (int lid = 0; lid < s->ls.l; lid++)
    {

        mat.ambient = s->ls.d[lid].cols.ambient;
        a.v = b.v = s->ls.d[lid].pos;
        a.v.x -= r1;
        b.v.x += r1;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.y -= r1;
        b.v.y += r1;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.z -= r1;
        b.v.z += r1;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.x -= r2;
        b.v.x += r2;
        a.v.z -= r2;
        b.v.z += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.x -= r2;
        b.v.x += r2;
        a.v.y -= r2;
        b.v.y += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.z -= r2;
        b.v.z += r2;
        a.v.y -= r2;
        b.v.y += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, mat_shader_f, &mat);
    }
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

    rgba_t cols[] = {
        {255, 0, 0, 255}, {0, 255, 0, 255}, {0, 0, 255, 255},
        {99, 23, 162, 255}, {220, 226, 67, 255}, {61, 163, 45, 255}
    };
    int cols_len = 6;

    if (face->l == 3)
    {
        for (int i = 0; i < 3; i++)
        {
            vs[i] = vertex2evertex(
                model->vs.d[face->v[i]],
                face->n[i] != -1 ? model->ns.d[face->n[i]] : (v3_t){0, 0, 0},
                s
            );
            vs[i].c = model->props.mat.ambient;
            //vs[i].v = model->vs.d[face->v[i]];
            /*console_log("%lf %lf %lf", model->vs.d[face->v[i]].x, model->vs.d[face->v[i]].y, model->vs.d[face->v[i]].z);
            console_log("%d", face->v[i]);*/
            //if (face->n[i] != -1)
            //    vs[i].n = model->ns.d[face->n[i]];
        }

        for (int i = 0; i < 3; i++)
        {
            if (model->props.shaders.v)
                vs[i] = model->props.shaders.v(vs, i, s);
            //console_log("%lf %lf %lf", vs[i].wv.x, vs[i].wv.y, vs[i].wv.z);
            //console_log("%lf %lf %lf", vs[i].n.x, vs[i].n.y, vs[i].n.z);
            //vs[i] = world2viewport(vs[i], s);
            // TODO: TEST PART
            //memcpy(&vs[i].c, &cols[face->v[i] % cols_len], sizeof(rgba_t));
            // TODO: END OF TEST PART
            //console_log("%lf %lf %lf", vs[i].v.x, vs[i].v.y, vs[i].v.z);
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
    //console_log("X %lf %lf %lf", vs[0].v.x, vs[1].v.x, vs[2].v.x);
    //console_log("Y %lf %lf %lf", vs[0].v.y, vs[1].v.y, vs[2].v.y);

    if ((render_mode & RENDER_WIREFRAME) == 0) {
    int min_y = vs[0].v.y;//min(s->canv->h, max(0, vs[0].v.y));
    int mid_y = vs[1].v.y;//min(s->canv->h, max(0, vs[1].v.y));
    int max_y = vs[2].v.y;//min(s->canv->h, max(0, vs[2].v.y));

    ya = yield_evertex_init(vs[0], vs[1], vs[0].v.y - vs[1].v.y);
    yb = yield_evertex_init(vs[0], vs[2], vs[0].v.y - vs[2].v.y);

    ya.dev.v.y = 1;
    yb.dev.v.y = 1;

    /*ya.evc.r = 255;
    ya.evc.g = 0;
    ya.evc.b = 0;
    ya.evc.a = 255;
    ya.devc.r = ya.devc.g = ya.devc.b = ya.devc.a = 0;
    yb.evc.r = 255;
    yb.evc.g = 0;
    yb.evc.b = 0;
    yb.evc.a = 255;
    yb.devc.r = yb.devc.g = yb.devc.b = yb.devc.a = 0;*/
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
    //ya.evc = ytmp.evc;

    ytmp = yb;
    yb = yield_evertex_init(vs[0], vs[2], vs[2].v.y - vs[0].v.y);
    //yb.t = ytmp.t;
    //yb.d = ytmp.d;
    //yb.ev = ytmp.ev;
    //yb.evc = ytmp.evc;

    ya.dev.v.y = -1;
    yb.dev.v.y = -1;

    /*ya.evc.r = 0;
    ya.evc.g = 255;
    ya.evc.b = 0;
    ya.evc.a = 255;
    ya.devc.r = ya.devc.g = ya.devc.b = ya.devc.a = 0;
    yb.evc.r = 0;
    yb.evc.g = 255;
    yb.evc.b = 0;
    yb.evc.a = 255;
    yb.devc.r = yb.devc.g = yb.devc.b = yb.devc.a = 0;*/
    /*console_log("X %lf %lf %lf", ya.ev.v.x, ya.ev.v.x, ya.ev.v.z);
    console_log("Y %lf %lf %lf", yb.ev.v.y, yb.ev.v.y, yb.ev.v.z);*/

    //for (int y = max_y; y >= mid_y; y--)
    for (int y = mid_y; y < max_y; y++)
        draw_triangle_row(s, vs, shf, mat, y, &ya, &yb);

    /*console_log("X %lf %lf %lf", ya.ev.v.x, ya.ev.v.x, ya.ev.v.z);
    console_log("Y %lf %lf %lf", yb.ev.v.y, yb.ev.v.y, yb.ev.v.z);*/
    }

    pixel_t ps[3];

    if ((render_mode & RENDER_WIREFRAME) != 0) {
    for (int i = 0; i < 3; i++)
    {
        ps[i].pos.x = vs[i].v.x;
        ps[i].pos.y = vs[i].v.y;
    }

    draw_line(s, ps[0].pos, ps[1].pos, shf, mat);
    draw_line(s, ps[0].pos, ps[2].pos, shf, mat);
    draw_line(s, ps[2].pos, ps[1].pos, shf, mat);
    }
    else
    {
    /*draw_line_3d(s, vs[0], vs[1], shf, mat);
    draw_line_3d(s, vs[0], vs[2], shf, mat);
    draw_line_3d(s, vs[2], vs[1], shf, mat);*/
    }
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

    /*for (int i = va.v.x; i < 0; i++)
        YIELD_EVERTEX(ev, yv);*/

    for (int i = va.v.x; i <= min(vb.v.x - 1, s->canv->w - 1); i++)
    {
        YIELD_EVERTEX(ev, yv);

        /*if (yi.ev.v.z < 0)
        {
            if (yi.dev.v.z < 0)
                break;
            else
                continue;
        }*/

        if (vi.v.x < 0 || vi.v.x >= s->canv->w ||
        vi.v.y < 0 || vi.v.y >= s->canv->h)
            continue;

        a = shf(vi, mat, s);
        if (a.pos.x >= 0 && a.pos.x < s->canv->w && a.pos.y >= 0 && a.pos.y < s->canv->h && (vi.v.z >= s->perspective_props.near && vi.v.z <= s->perspective_props.far))
            SET_PIXEL_Z(s->canv, s->zbuf, a.pos.x, a.pos.y, ((vi.v.z)/(s->perspective_props.far - s->perspective_props.near)*ZBUF_DEPTH), &a.col);
        //if (a.pos.x >= 0 && a.pos.x < s->canv->w &&
        //    a.pos.y >= 0 && a.pos.y < s->canv->h &&
        //    (vi.v.z >= 0 && vi.v.z <= ZBUF_DEPTH))
        //    SET_PIXEL_Z(s->canv, s->zbuf, a.pos.x, a.pos.y, vi.v.z, &a.col);

        //YIELD_EVERTEX(ev, yv);
    }
}

yield_evertex_t yield_evertex_init(evertex_t a, evertex_t b, int steps)
{
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
    
    yv.dev.wv.x = (a.wv.x - b.wv.x) * yv.d;
    yv.dev.wv.y = (a.wv.y - b.wv.y) * yv.d;
    yv.dev.wv.z = (a.wv.z - b.wv.z) * yv.d;
    
    yv.dev.n.x = (a.n.x - b.n.x) * yv.d;
    yv.dev.n.y = (a.n.y - b.n.y) * yv.d;
    yv.dev.n.z = (a.n.z - b.n.z) * yv.d;
    
    yv.dev.wn.x = (a.wn.x - b.wn.x) * yv.d;
    yv.dev.wn.y = (a.wn.y - b.wn.y) * yv.d;
    yv.dev.wn.z = (a.wn.z - b.wn.z) * yv.d;
    
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

void draw_zbuf(scene_t *s)
{
    for (int y = 0; y < s->canv->h; y++)
        for (int x = 0; x < s->canv->w; x++)
        {
            int z = s->zbuf->data[y * s->canv->w + x] * 1.0 / 255;
            SET_PIXEL(s->canv, x, y, &((rgba_t){z, z, z, 255}));
        }
}
