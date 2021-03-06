#include <math.h>

#include "render.h"
#include "math.h"
#include "utils.h"
#include "shaders.h"


void frame(scene_t *s, int render)
{
    model_t **model;
    render_mode = render;

    if (render_mode & RENDER_GRID)
        draw_grid(s, 20, 26);

    if (render_mode & RENDER_XYZ)
        draw_xyz(s);

    if (render_mode & RENDER_LIGHTS)
        draw_lights(s);

    model = s->models.d;
    for (int i = 0; i < s->models.l; i++, model++)
        draw_model(s, *model);

    if (render_mode & RENDER_ZBUF)
        draw_zbuf(s);
}

void draw_grid(scene_t *s, int size, int count)
{
    evertex_t a, b;
    mat_t mat;

    a.v = (v3_t){0.0, 0, 0};
    a = world2viewport(a, s);

    mat.ambient = (rgba_t){160, 160, 160, 255};
    for (int x = -count / 2; x <= count - (count / 2); x++)
    {
        a.v.z = b.v.z = 0;
        a.v.x = b.v.x = size * x;
        a.v.y = -size * (count / 2);
        b.v.y = size * (count - (count / 2));
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);
    }

    for (int y = -count / 2; y <= count - (count / 2); y++)
    {
        a.v.z = b.v.z = 0;
        a.v.y = b.v.y = size * y;
        a.v.x = -size * (count / 2);
        b.v.x = size * (count - (count / 2));
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);
    }
}

void draw_xyz(scene_t *s)
{
    evertex_t a, b;
    mat_t mat;

    a.v = (v3_t){0.0, 0, 0};
    a = world2viewport(a, s);

    mat.ambient = (rgba_t){0, 0, 255, 255};
    b.v = (v3_t){100, 0, 0};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, none_shader_f, &mat);

    mat.ambient = (rgba_t){255, 0, 0, 255};
    b.v = (v3_t){0, 100, 0};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, none_shader_f, &mat);

    mat.ambient = (rgba_t){255, 255, 0, 255};
    b.v = (v3_t){0, 0, 100};
    b = world2viewport(b, s);
    draw_line_3d(s, a, b, none_shader_f, &mat);
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
        draw_line_3d(s, a, b, none_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.y -= r1;
        b.v.y += r1;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.z -= r1;
        b.v.z += r1;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.x -= r2;
        b.v.x += r2;
        a.v.z -= r2;
        b.v.z += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.x -= r2;
        b.v.x += r2;
        a.v.y -= r2;
        b.v.y += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);

        a.v = b.v = s->ls.d[lid].pos;
        a.v.z -= r2;
        b.v.z += r2;
        a.v.y -= r2;
        b.v.y += r2;
        a = world2viewport(a, s);
        b = world2viewport(b, s);
        draw_line_3d(s, a, b, none_shader_f, &mat);
    }
}

void draw_model(scene_t *s, const model_t *model)
{
    face_t *face;

    face = model->fs.d;
    for (int i = 0; i < model->fs.l; i++, face++)
        draw_model_face(s, model, face);

    if (render_mode & RENDER_VERTS)
        for (int i = 0; i < model->vs.l; i++)
        {
            rgba_t c = (rgba_t){0, 255, 0, 255};
            evertex_t v;
            v.v =  model->vs.d[i];
            v = world2viewport(v, s);
            if (v.v.x >= 0 && v.v.x < s->canv->w && v.v.y >= 0 && v.v.y < s->canv->h)
                DOT(s->canv, lroundf(v.v.x), lroundf(v.v.y), &c);
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
        }

        for (int i = 0; i < 3; i++)
        {
            if (model->props.shaders.v)
                vs[i] = model->props.shaders.v(vs, i, s);

            if (render_mode & RENDER_COLORED)
                memcpy(&vs[i].c, &cols[face->v[i] % cols_len], sizeof(rgba_t));
        }

        draw_fragment(s, vs, &model->props);

        if (render_mode & RENDER_NORMS)
            for (int i = 0; i < 3; i++)
                draw_norm(s, vs[i]);
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

    if (render_mode & RENDER_FACE)
    {
        int min_y = vs[0].v.y;
        int mid_y = vs[1].v.y;
        int max_y = vs[2].v.y;

        ya = yield_evertex_init(vs[0], vs[1], min_y - mid_y);
        yb = yield_evertex_init(vs[0], vs[2], min_y - max_y);

        ya.dev.v.y = 1;
        yb.dev.v.y = 1;

        if (min_y != mid_y)
            for (int y = min_y; y <= mid_y; y++)
                draw_triangle_row(s, vs, shf, mat, y, &ya, &yb);

        tmp = vs[2];
        vs[2] = vs[0];
        vs[0] = tmp;

        ytmp = ya;
        ya = yield_evertex_init(vs[0], vs[1], mid_y - max_y);
        yb = yield_evertex_init(vs[0], vs[2], min_y - max_y);

        ya.dev.v.y = -1;
        yb.dev.v.y = -1;

        if (mid_y != max_y)
            for (int y = mid_y; y <= max_y; y++)
                draw_triangle_row(s, vs, shf, mat, y, &ya, &yb);
    }

    pixel_t ps[3];
    if (render_mode & RENDER_WIREFRAME)
    {
        for (int i = 0; i < 3; i++)
        {
            ps[i].pos.x = vs[i].v.x;
            ps[i].pos.y = vs[i].v.y;
        }

        draw_line(s, ps[0].pos, ps[1].pos, none_shader_f, mat);
        draw_line(s, ps[0].pos, ps[2].pos, none_shader_f, mat);
        draw_line(s, ps[2].pos, ps[1].pos, none_shader_f, mat);
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

    va = ya->ev;
    va.c = (rgba_t){
        clamp(ya->evc.r, 0, 255),
        clamp(ya->evc.g, 0, 255),
        clamp(ya->evc.b, 0, 255),
        clamp(ya->evc.a, 0, 255),
    };

    vb = yb->ev;
    vb.c = (rgba_t){
        clamp(yb->evc.r, 0, 255),
        clamp(yb->evc.g, 0, 255),
        clamp(yb->evc.b, 0, 255),
        clamp(yb->evc.a, 0, 255),
    };

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
    evertex_t vi = va;
    yi = yield_evertex_init(va, vb, va.v.x - vb.v.x);
    yi.dev.v.x = 1;
    yi.dev.v.y = 0;

    ev = &vi;
    yv = &yi;

    for (int i = va.v.x; i <= min(vb.v.x, s->canv->w - 1); i++)
    {
        if (vi.v.x < 0 || vi.v.x >= s->canv->w ||
        vi.v.y < 0 || vi.v.y >= s->canv->h)
        {
            YIELD_EVERTEX(ev, yv);
            continue;
        }

        a = shf(vi, mat, s);
        if (a.pos.x >= 0 && a.pos.x < s->canv->w && a.pos.y >= 0 && a.pos.y < s->canv->h && (vi.v.z >= s->perspective_props.near && vi.v.z <= s->perspective_props.far))
            SET_PIXEL_Z(s->canv, s->zbuf, a.pos.x, a.pos.y, ((vi.v.z)/(s->perspective_props.far - s->perspective_props.near)*ZBUF_DEPTH), &a.col);

        YIELD_EVERTEX(ev, yv);
    }

    YIELD_EVERTEX(ev, ya);
    YIELD_EVERTEX(ev, yb);
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
    
    yv.ev.light.r = a.light.r;
    yv.ev.light.g = a.light.g;
    yv.ev.light.b = a.light.b;
    yv.ev.light.a = a.light.a;

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
    
    yv.dev.light.r = (a.light.r - b.light.r) * yv.d;
    yv.dev.light.g = (a.light.g - b.light.g) * yv.d;
    yv.dev.light.b = (a.light.b - b.light.b) * yv.d;
    yv.dev.light.a = (a.light.a - b.light.a) * yv.d;

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

void draw_norm(scene_t *s, evertex_t v)
{
    evertex_t ea, eb;
    mat_t mat;
    mat.ambient = (rgba_t){255, 0, 0, 255};
    ea.v = v3_add(v.wv, v3_scale(v.wn, 5));
    eb.v = v3_add(v.wv, v3_scale(v.wn, 20));
    ea = world2viewport(ea, s);
    eb = world2viewport(eb, s);
    draw_line_3d(s, ea, eb, none_shader_f, &mat);
}
