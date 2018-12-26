#include <webassembly.h>

#include "shaders.h"
#include "canvas.h"
#include "render.h"
#include "utils.h"

pixel_t none_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = mat->ambient;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t none_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    return r;
}

pixel_t plain_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = (rgba_t){
        clampf(a.light.r * a.c.r / 255, 0, 255),
        clampf(a.light.g * a.c.g / 255, 0, 255),
        clampf(a.light.b * a.c.b / 255, 0, 255),
        255
    };

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t plain_shader_v(evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    if (i == 0)
    {
        v3_t v12 = v3_scale(v3_add(vs[1].wn, vs[2].wn), 1.0/2);
        r.wn = v3_norm(v3_scale(v3_add(v3_scale(vs[0].wn, 2), v12), 1.0/3));
        v12 = v3_scale(v3_add(vs[1].n, vs[2].n), 1.0/2);
        r.n = v3_norm(v3_scale(v3_add(v3_scale(vs[0].n, 2), v12), 1.0/3));

        v3_t amb;
        v3_t p = r.wv;
        v3_t v = s->cam.dir;
        v3_t n = r.wn;

        v3_t id = {0, 0, 0};
        v3_t is = {0, 0, 0};
        v3_t ia;
        float a = 0;
        float alpha;

        v3_t vn = v3_sub((v3_t){0, 0, 0}, v);
        v3_t vr = v3_sub((v3_t){0, 0, 0}, v3_sub(vn, v3_scale(n, 2 * v3_dot(n, v))));

        for (int lid = 0; lid < s->ls.l; lid++)
        {
            alpha = s->ls.d[lid].cols.ambient.a * 1.0 / 255;
            a += alpha;
            amb = (v3_t){
                s->ls.d[lid].cols.ambient.r,
                s->ls.d[lid].cols.ambient.g,
                s->ls.d[lid].cols.ambient.b
            };
            v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));

            id = v3_add(id, v3_scale(amb, fmax(v3_dot(n, l) * alpha, 0)));
            is = v3_add(is, v3_scale(amb, powf(fmax(v3_dot(l, vr) * alpha, 0), 4)));
        }

        ia = v3_add(id, is);

        r.light = (frgba_t){
            ia.x * r.c.r / 255,
            ia.y * r.c.g / 255,
            ia.z * r.c.b / 255,
            a * 255,
        };
    }
    else
    {
        r.wn = vs[0].wn;
        r.n = vs[0].n;
        r.light = vs[0].light;
    }

    return r;
}

pixel_t lambert_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = (rgba_t){
        clampf(a.light.r * a.c.r / 255, 0, 255),
        clampf(a.light.g * a.c.g / 255, 0, 255),
        clampf(a.light.b * a.c.b / 255, 0, 255),
        255
    };

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t lambert_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    v3_t amb;
    v3_t p = vs[i].wv;
    v3_t n = vs[i].wn;

    v3_t id = {0, 0, 0};
    v3_t ia;
    float a = 0;
    float alpha;
    float f = 0.1;

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        alpha = s->ls.d[lid].cols.ambient.a * 1.0 / 255;
        a += alpha;
        amb = (v3_t){
            s->ls.d[lid].cols.ambient.r,
            s->ls.d[lid].cols.ambient.g,
            s->ls.d[lid].cols.ambient.b
        };
        v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));

        id = v3_scale(v3_add(id, v3_scale(amb, fmax((v3_dot(n, l) + f) * alpha, 0))), 1.0/(1 + f));
    }

    ia = id;

    r.light = (frgba_t){
        ia.x * r.c.r / 255,
        ia.y * r.c.g / 255,
        ia.z * r.c.b / 255,
        a * 255,
    };

    return r;
}

pixel_t guro_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = (rgba_t){
        clampf(a.light.r * a.c.r / 255, 0, 255),
        clampf(a.light.g * a.c.g / 255, 0, 255),
        clampf(a.light.b * a.c.b / 255, 0, 255),
        255
    };

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t guro_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    v3_t amb;
    v3_t p = vs[i].wv;
    v3_t v = s->cam.dir;
    v3_t n = vs[i].wn;

    v3_t id = {0, 0, 0};
    v3_t is = {0, 0, 0};
    v3_t ia;
    float a = 0;
    float alpha;

    v3_t vn = v3_sub((v3_t){0, 0, 0}, v);
    v3_t vr = v3_sub((v3_t){0, 0, 0}, v3_sub(vn, v3_scale(n, 2 * v3_dot(n, v))));

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        alpha = s->ls.d[lid].cols.ambient.a * 1.0 / 255;
        a += alpha;
        amb = (v3_t){
            s->ls.d[lid].cols.ambient.r,
            s->ls.d[lid].cols.ambient.g,
            s->ls.d[lid].cols.ambient.b
        };
        v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));

        id = v3_add(id, v3_scale(amb, fmax(v3_dot(n, l) * alpha, 0)));
        is = v3_add(is, v3_scale(amb, powf(fmax(v3_dot(l, vr) * alpha, 0), 4)));
    }

    ia = v3_add(id, is);

    r.light = (frgba_t){
        ia.x * r.c.r / 255,
        ia.y * r.c.g / 255,
        ia.z * r.c.b / 255,
        a * 255,
    };

    return r;
}

pixel_t phong_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    v3_t id = {0, 0, 0};
    v3_t is = {0, 0, 0};
    v3_t ia = {0, 0, 0};
    v3_t amb;

    v3_t p = a.wv;
    v3_t v = s->cam.dir;
    v3_t n = a.wn;

    CHECK_VISIBILITY(r, v, n, 0.2);

    v3_t vn = v3_sub((v3_t){0, 0, 0}, v);
    v3_t vr = v3_sub((v3_t){0, 0, 0}, v3_sub(vn, v3_scale(n, 2 * v3_dot(n, v))));
    float alpha;

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        alpha = s->ls.d[lid].cols.ambient.a * 1.0 / 255;
        amb = (v3_t){
            s->ls.d[lid].cols.ambient.r,
            s->ls.d[lid].cols.ambient.g,
            s->ls.d[lid].cols.ambient.b
        };
        v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));

        id = v3_add(id, v3_scale(amb, fmax(v3_dot(n, l) * alpha, 0)));
        is = v3_add(is, v3_scale(amb /*TODO: spec*/, powf(fmax(v3_dot(l, vr) * alpha, 0), 4)));
    }

    ia = v3_add(id, is);

    r.col = (rgba_t){
        clampf(ia.x * a.c.r / 255, 0, 255),
        clampf(ia.y * a.c.g / 255, 0, 255),
        clampf(ia.z * a.c.b / 255, 0, 255),
        255
    };

    return r;
}

evertex_t phong_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    return r;
}

pixel_t blinn_phong_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    v3_t id = {0, 0, 0};
    v3_t is = {0, 0, 0};
    v3_t ia = {0, 0, 0};
    v3_t amb;

    v3_t p = a.wv;
    v3_t v = s->cam.dir;
    v3_t n = a.wn;
    v3_t h;
    v3_t lv;

    CHECK_VISIBILITY(r, v, n, 0.2);

    float alpha;

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        alpha = s->ls.d[lid].cols.ambient.a * 1.0 / 255;
        amb = (v3_t){
            s->ls.d[lid].cols.ambient.r,
            s->ls.d[lid].cols.ambient.g,
            s->ls.d[lid].cols.ambient.b
        };
        v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));
        lv = v3_add(l, v);
        h = v3_norm(lv);

        id = v3_add(id, v3_scale(amb, fmax(v3_dot(n, l) * alpha, 0)));
        is = v3_add(is, v3_scale(amb, powf(fmax(v3_dot(n, h) * alpha, 0), 1)));
    }

    ia = v3_add(id, is);

    r.col = (rgba_t){
        clampf(ia.x * a.c.r / 255, 0, 255),
        clampf(ia.y * a.c.g / 255, 0, 255),
        clampf(ia.z * a.c.b / 255, 0, 255),
        255
    };

    return r;
}

evertex_t blinn_phong_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    return r;
}
