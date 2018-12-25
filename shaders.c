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

    r.col = mat->ambient;

    // TODO
    r = phong_shader_f(a, mat, s);

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t plain_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    if (i == 0)
    {
        r.wn = v3_norm(v3_add(v3_add(vs[0].wn,vs[1].wn),vs[2].wn));
        r.n = v3_norm(v3_add(v3_add(vs[0].n,vs[1].n),vs[2].n));
    }
    else
    {
        r.wn = vs[0].wn;
        r.n = vs[0].n;
    }

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

    if (v3_dot(v, n) < -0.2)
    {
        r.pos.x = -1;
        r.pos.y = -1;
        r.col.a = 0;
        return r;
    }

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
        is = v3_add(is, v3_scale(amb /*TODO: spec*/, pow2(fmax(v3_dot(l, vr) * alpha, 0), 1)));
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

pixel_t ray_casting_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = a.c;
    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    model_t **modeli;
    model_t *model;
    face_t *face;
    evertex_t vs[3], el;
    float atten, attens = 0;
    int lights = 0;

    for (int lid = 0; lid < s->ls.l; lid++)
        lights |= (1 << lid);

    modeli = s->models.d;
    for (int mid = 0; mid < s->models.l; mid++, modeli++)
    {
        model = *modeli;
        face = model->fs.d;
        for (int fid = 0; fid < model->fs.l; fid++, face++)
        {
            if (face->l == 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    vs[i].wv = model->vs.d[face->v[i]];
                    if (face->n[i] != -1)
                        vs[i].n = model->ns.d[face->n[i]];
                }

                for (int lid = 0; lid < s->ls.l; lid++)
                {
                    //if((lights & (1 << lid)) == 0)
                    //    continue;

                    el.wv = s->ls.d[lid].pos;
                    v3_t dir = v3_sub(el.wv, a.wv);
                    float d = sqrt(v3_dot(dir, dir));
                    float ind = intersect_triangle(a.wv, dir, vs[0].wv, vs[1].wv, vs[2].wv);

                    float eps = 0;
                    if (0 + eps < ind && ind < 1 - eps)
                        lights &= ~(1 << lid);
                }
            }
            else
            {
                // TODO (15.11.2018): yield_face_triangle
            }
        }
    }

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        if((lights & (1 << lid)) == 0)
            continue;

        el.wv = s->ls.d[lid].pos;
        v3_t dir = v3_sub(el.wv, a.wv);
        float d = sqrt(v3_dot(dir, dir));

        atten = 1 / (s->ls.d[lid].attens.c +
        s->ls.d[lid].attens.l * d +
        s->ls.d[lid].attens.q * d * d);

        dir = v3_norm(dir);
        float alpha = v3_dot(dir, vs[0].wn);
        alpha = alpha < 0 ? -alpha : alpha;
        alpha = 1; // TODO
        attens += atten * alpha;
        lights |= (1 << lid);
    }

    r.col = rgba_scale3(r.col, attens);

    return r;
}
