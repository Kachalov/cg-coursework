#include <webassembly.h>
#include <math.h>

#include "scene.h"
#include "models.h"
#include "render.h"
#include "math.h"
#include "utils.h"


// TODO (27.11.18): Remove test part
pixel_t test_shader_f(const evertex_t a, const mat_t *mat, scene_t *s)
{
    pixel_t r;

    r.col.r = a.c.r; //(10*lroundf(a.v.x/10.)) % 255;
    r.col.g = a.c.g;
    r.col.b = a.c.b;
    r.col.a = a.c.a;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;

    evertex_t wa = viewport2world(a, s);

    model_t **modeli;
    model_t *model;
    face_t *face;
    evertex_t vs[3];
    float atten, attens = 0;

    modeli = s->models.d;
    for (int i = 0; i < s->models.l; i++, modeli++)
    {
        model = *modeli;
        face = model->fs.d;
        for (int i = 0; i < model->fs.l; i++, face++)
        {
            if (face->l == 3)
            {
                for (int i = 0; i < 3; i++)
                {
                    vs[i].v = model->vs.d[face->v[i]];
                    if (face->n[i] != -1)
                        vs[i].n = model->ns.d[face->n[i]];
                    vs[i] = world2viewport(vs[i], s);
                }

                for (int i = 0; i < s->ls.l; i++)
                {
                    v3_t dir = v3_sub(s->ls.d[i].pos, wa.v);
                    float d = sqrt(v3_dot(dir, dir));
                    float ind = intersect_triangle(wa.v, dir, vs[0].v, vs[1].v, vs[2].v);
                    if (ind == -1 || ind < 1 || d < ind)
                    {
                        atten = 1 / (s->ls.d[i].attens.c +
                        s->ls.d[i].attens.l * d +
                        s->ls.d[i].attens.q * d * d);

                        dir = v3_norm(dir);
                        float alpha = v3_dot(dir, vs[0].n);
                        alpha = alpha < 0 ? -alpha : alpha;
                        attens += atten * alpha;
                    }
                }
            }
            else
            {
                // TODO (15.11.2018): yield_face_triangle
            }
        }
    }

    //r.col = rgba_scale3(r.col, attens > 1 ? 1 : attens);

    return r;
}

evertex_t test_shader_v(const evertex_t *vs, int i, scene_t *s)
{
    evertex_t r = vs[i];

    v3_t a, b;

    a = v3_sub(vs[1].v, vs[0].v);
    b = v3_sub(vs[2].v, vs[0].v);
    r.n = v3_cross(a, b);

    r.c = (rgba_t){255 * (i == 0), 255 * (i == 1), 255 * (i == 2), 255};

    return r;
}
// End of test part

export
scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint16_t *zbuf)
{
    scene_t *s = heap_alloc(sizeof(scene_t));
    char *after = (char *)(s + 1);

    s->canv = canvas_init(w, h, canv ? canv : 0);
    s->zbuf = zbuf_init(w, h, zbuf ? zbuf : 0);

    s->bmask_row = bitmask_init(w, 1);

    s->ls.d = 0;
    s->ls.l = 0;

    /*s->cams.d = 0;
    s->cams.l = 0;
    s->cams.cur = 0;*/

    s->models.d = 0;
    s->models.l = 0;

    s->view_mtrx = make_viewport((v3_t){1, 0.4, -1}, (v3_t){0, 0, 0}, (v3_t){0, 1, 0});
    s->proj_mtrx = make_perspective(45, s->canv->w / s->canv->h, 0.1, 1000);
    s->mvp_mtrx = m4_m4_mul(&s->proj_mtrx, &s->view_mtrx);

    // TODO (27.11.18): Remove test part
    {m4_t m = s->view_mtrx;
    /*m4_t ma = (m4_t){
    1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16.
    };
    v3_t vb = (v3_t){1., 2., 3.};
    v3_t r = m4_v3t_mul(&ma, &vb);
    console_log("[%lf; %lf; %lf]", r.x, r.y, r.z);
    abort();*/

    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    {m4_t m = s->proj_mtrx;
    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    model_t *m = model_init(NULL, 1000, 1000, 1000);

    vertex_t vs[] = {
        {100, 100, 500},
        {100, 200, 500},
        {200, 200, 500},
        {200, 100, 500},

        {100, 100, 600},
        {100, 200, 600},
        {200, 200, 600},
        {200, 100, 600}
    };
    m = model_add_vertices_arr(m, vs, 8);

    normalid_t nid[] = {-1, -1, -1};
    vertexid_t vid[3] = {0, 1, 2};
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){2, 3, 0}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    memcpy(vid, &((vertexid_t[3]){4, 5, 6}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){6, 7, 4}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    memcpy(vid, &((vertexid_t[3]){0, 1, 5}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){5, 4, 0}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    // TODO(13.12.18): Полоса
    memcpy(vid, &((vertexid_t[3]){2, 3, 6}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){6, 7, 3}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    memcpy(vid, &((vertexid_t[3]){1, 2, 5}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){5, 6, 2}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    memcpy(vid, &((vertexid_t[3]){3, 0, 7}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){7, 4, 0}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    m->props.mat.ambient.r = 0;
    m->props.mat.ambient.g = 0;
    m->props.mat.ambient.b = 0;
    m->props.mat.ambient.a = 255;

    m->props.shaders.f = test_shader_f;
    m->props.shaders.v = test_shader_v;

    scene_add_model(s, m);

    s->ls.d = heap_alloc(sizeof (light_t) * 2);
    s->ls.l = 1;

    s->ls.d[0].pos = (v3_t){90, 90, 100};
    s->ls.d[0].attens = (light_attens_t){1, 0.007, 0};
    s->ls.d[0].cols.ambient = (rgba_t){255, 255, 255, 255};
    s->ls.d[0].cols.diffuse = (rgba_t){255, 255, 255, 255};
    s->ls.d[0].cols.specular = (rgba_t){255, 255, 255, 255};

    s->ls.d[1].pos = (v3_t){210, 210, 700};
    s->ls.d[1].attens = (light_attens_t){1, 0.00001, 0};
    s->ls.d[1].cols.ambient = (rgba_t){255, 255, 255, 255};
    s->ls.d[1].cols.diffuse = (rgba_t){255, 255, 255, 255};
    s->ls.d[1].cols.specular = (rgba_t){255, 255, 255, 255};

    // End of test part

    scene = s;
    clear(s);
    return s;
}

export
int scene_add_model(scene_t *s, model_t *model)
{
    int so = s->models.l;
    int sn = s->models.l + 1;
    model_t **ptr = heap_realloc(s->models.d, sizeof(model_t *) * sn);
    if (!ptr)
        return -1;

    s->models.d = ptr;
    s->models.l = sn;
    s->models.d[so] = model;

    return so;
}

export
void scene_free(scene_t *s)
{
    bitmask_free(s->bmask_row);
    heap_free(s);
}

export
zbuf_t *zbuf_init(int w, int h, uint16_t *data)
{
    int size_data = data ? 0 : w * h * sizeof(uint16_t);
    int size = sizeof(zbuf_t) + size_data;
    zbuf_t *r = (zbuf_t *)heap_alloc(size);

    r->data = data ? data : (uint16_t *)(r + 1);
    r->w = w;
    r->h = h;

    return r;
}

export void clear(scene_t *s)
{
    rgba_t *cit = (rgba_t *)s->canv->data;
    for (int i = 0; i < s->canv->h * s->canv->w; i++, cit++)
        cit->a = 0;

    uint16_t *zit = s->zbuf->data;
    for (int i = 0; i < s->zbuf->h * s->zbuf->w; i++, zit++)
        *zit = (uint16_t)-1;
}
