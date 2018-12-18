#include <webassembly.h>

#include "shaders.h"
#include "canvas.h"
#include "render.h"
#include "utils.h"

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

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    v3_t id = {0, 0, 0};
    v3_t is = {0, 0, 0};
    v3_t ia = {0, 0, 0};
    v3_t amb;

    v3_t p = a.wv;

    // TODO (18.12.2018): Ortho viepoint
    evertex_t evtmp;
    //evtmp.wv = evtmp.v = (v3_t){0, 0, 1};
    //evtmp = world2viewport(evtmp, s);
    //evtmp.v = v3_sub(evtmp.wv, evtmp.v);
    //evtmp = world2viewport(evtmp, s);

    v3_t v;// = v3_norm(evtmp.v);
    v = v3_norm(v3_sub(s->viewport_props.eye, p));
    v3_t n = a.wn;

    static int calls = 0;
    calls++;

    if (calls <= 5)
        console_log("NORM V %lf %lf %lf", v.x, v.y, v.z);

    if (v3_dot(v, n) < 0)
    {
        //r.pos.x = -1;
        //r.pos.y = -1;
        //r.col.a = 0;
        //return r;
    }

    v3_t vn = v3_sub((v3_t){0, 0, 0}, v);
    v3_t vr = v3_sub((v3_t){0, 0, 0}, v3_sub(vn, v3_scale(n, 2 * v3_dot(n, v))));

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        amb = (v3_t){
            s->ls.d[lid].cols.ambient.r,
            s->ls.d[lid].cols.ambient.g,
            s->ls.d[lid].cols.ambient.b
        };
        v3_t l = v3_norm(v3_sub(s->ls.d[lid].pos, p));

        id = v3_add(id, v3_scale(amb, fmax(v3_dot(n, l), 0)));
        is = v3_add(is, v3_scale(amb /*TODO: spec*/, pow2(fmax(v3_dot(l, vr), 0), 2)));
    }

    ia = v3_add(id, is);

    r.col = (rgba_t){
        clamp(a.c.r * ia.x / 255, 0, 255),
        clamp(a.c.g * ia.y / 255, 0, 255),
        clamp(a.c.b * ia.z / 255, 0, 255),
        255
    };

    return r;
}
