#include <webassembly.h>

#include "shaders.h"
#include "canvas.h"
#include "render.h"

pixel_t mat_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = mat->ambient;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    return r;
}

evertex_t plain_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    return r;
}

pixel_t phong_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col = a.c;
    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    v3_t p = a.wv;
    v3_t l = v3_norm(v3_sub(s->ls.d[0].pos, p));
    v3_t v = v3_norm(v3_sub(s->viewport_props.eye, p));
    v3_t n = a.wn;

    v3_t vn = v3_sub((v3_t){0, 0, 0}, v);
    v3_t vr = v3_sub((v3_t){0, 0, 0}, v3_sub(vn, v3_scale(n, 2 * v3_dot(n, v))));

    float id = fmax(v3_dot(n, l), 0);
    float is = pow2(fmax(v3_dot(l, vr), 0), 2);
    r.col = rgba_scale3(r.col, id + is);

    return r;
}
