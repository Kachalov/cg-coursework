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

    /*r.col.r = clamp(lroundf(a.wv.x) % 255, 0, 255);a.c.r;
    r.col.g = clamp(lroundf(a.wv.x) % 255, 0, 255);a.c.g;
    r.col.b = clamp(lroundf(a.wv.x) % 255, 0, 255);a.c.b;*/
    r.col = a.c;

    r.pos.x = a.v.x;
    r.pos.y = a.v.y;
    //return r;

    static int calls = 0;
    if(calls++ < 500 || calls > 600)
    {
        //r.col = (rgba_t){0, 0, 0, 255};
        //return r;
    }
    else console_log("wa(%lf %lf %lf)", a.wv.x, a.wv.y, a.wv.z);


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

                    float eps = 0;//0.005;//0.040;
                    if (0 + eps < ind && ind < 1 - eps)
                    {
                        lights &= ~(1 << lid);
                        //console_log("!ind=%lf d=%lf fid=%d lid=%d", ind, d, fid, lid);
                    }
                    else
                    {
                        //console_log("ind=%lf d=%lf fid=%d lid=%d", ind, d, fid, lid);
                    }
                }
            }
            else
            {
                // TODO (15.11.2018): yield_face_triangle
                abort();
            }
        }
    }

    for (int lid = 0; lid < s->ls.l; lid++)
    {
        if((lights & (1 << lid)) == 0)
            continue;
        //console_log("LIGHT %d", lid);

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

    //console_log("K %lf", attens);
    r.col = rgba_scale3(r.col, attens);

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


    s->viewport_props.eye = (v3_t){0, 0, -300};//{1, 0.4, -1};
    s->viewport_props.center = (v3_t){0, 0, 0};
    s->viewport_props.up = (v3_t){0, 1, 0};

    s->perspective_props.angle = 30;
    s->perspective_props.ratio = 1.0 * w / h;
    s->perspective_props.near = 0.1;
    s->perspective_props.far = 1000;

    calculate_mtrx(s);

    // TODO (27.11.18): Remove test part
    {m4_t m = s->mvp_mtrx;
    /*m4_t ma = (m4_t){
    1., 2., 3., 4., 5., 6., 7., 8., 9., 10., 11., 12., 13., 14., 15., 16.
    };
    v3_t vb = (v3_t){1., 2., 3.};
    v3_t r = m4_v3t_mul(&ma, &vb);
    console_log("[%lf; %lf; %lf]", r.x, r.y, r.z);
    abort();*/
    console_log("mvp");
    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    {m4_t m = s->view_mtrx;
    console_log("view");
    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    {m4_t m = s->proj_mtrx;
    console_log("proj");
    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    {m4_t m = s->viewport_mtrx;
    console_log("viewport");
    console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
    console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);}

    model_t *m = model_init(NULL, 1000, 1000, 1000);

    vertex_t vs[] = {
        {100, 0, -5},
        {100, 100, -5},
        {200, 100, -5},
        {200, 0, -5},

        {100, 0, 95},
        {100, 100, 95},
        {200, 100, 95},
        {200, 0, 95},

        {0, 0, 0},
        {50, 0, 0},
        {0, 100, 0},
        {0, 0, 50},

        {-300, 0, 300},
        {-300, 0, -300},
        {300, 0, -300},
        {300, 0, 300}
    };
    m = model_add_vertices_arr(m, vs, 12 + 4);

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

    ///////

    memcpy(vid, &((vertexid_t[3]){8, 9, 10}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){8, 10, 11}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){8, 9, 11}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);


    memcpy(vid, &((vertexid_t[3]){12, 13, 14}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){14, 15, 12}), sizeof(vertexid_t[3]));
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

    s->ls.d[0].pos = (v3_t){300, 180, 160};//-10, 60, 45
    s->ls.d[0].attens = (light_attens_t){1, 0.001, 0};
    s->ls.d[0].cols.ambient = (rgba_t){255, 255, 255, 255};
    s->ls.d[0].cols.diffuse = (rgba_t){255, 255, 255, 255};
    s->ls.d[0].cols.specular = (rgba_t){255, 255, 255, 255};

    s->ls.d[1].pos = (v3_t){300, 180, -40};
    s->ls.d[1].attens = (light_attens_t){1, 0.01, 0};
    s->ls.d[1].cols.ambient = (rgba_t){255, 255, 255, 255};
    s->ls.d[1].cols.diffuse = (rgba_t){255, 255, 255, 255};
    s->ls.d[1].cols.specular = (rgba_t){255, 255, 255, 255};

    vertex_t vt[] = {
        {200, 200, 0},
        {200, -200, 0},
        {-200, 200, 0},

        {50, 199, -1}, //L
        {50, 199, 90} //P
    };

    evertex_t evt[5];
    for (int i = 0; i < 5; i++)
    {
        evt[i].v = vt[i];
        //evt[i] = world2viewport(evt[i], s);
    }

    v3_t dir = v3_sub(evt[3].v, evt[4].v);
    float d = sqrt(v3_dot(dir, dir));
    float ind = intersect_triangle(evt[4].v, dir, evt[0].v, evt[1].v, evt[2].v);
    console_log("TEST %lf %lf", d, ind);

    v3_t v = (v3_t){111.27630615234375, 109.13348388671875, 45};
    dir = v3_sub(s->ls.d[0].pos, v);
    d = sqrt(v3_dot(dir, dir));
    console_log("TEST!(%lf %lf %lf)", s->ls.d[0].pos.x, s->ls.d[0].pos.y, s->ls.d[0].pos.z);
    console_log("TEST!(%lf %lf %lf)", dir.x, dir.y, dir.z);
    console_log("TEST!(%lf %lf %lf)", vs[8].x, vs[8].y, vs[8].z);
    console_log("TEST!(%lf %lf %lf)", vs[9].x, vs[9].y, vs[9].z);
    console_log("TEST!(%lf %lf %lf)", vs[10].x, vs[10].y, vs[10].z);
    ind = intersect_triangle(v, dir, vs[8], vs[9], vs[10]);
    console_log("TEST! %lf %lf", d, ind);
    //abort();

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

void calculate_mtrx(scene_t *s)
{
    s->view_mtrx = make_view(
        s->viewport_props.eye,
        s->viewport_props.center,
        s->viewport_props.up);
    s->proj_mtrx = make_projection(
        s->perspective_props.angle,
        s->perspective_props.ratio,
        s->perspective_props.near,
        s->perspective_props.far);
    s->viewport_mtrx = make_viewport(
        s->canv->w * 0.0 / 4,
        s->canv->h * 0.0 / 4,
        s->canv->w * 4.0 / 4,
        s->canv->h * 4.0 / 4);
    m4_t vp = m4_m4_mul(&s->proj_mtrx, &s->view_mtrx);
    s->mvp_mtrx = vp;//m4_m4_mul(&s->viewport_mtrx, &vp);
}

void move_viewport(scene_t *s, float hor, float vert, float tang, float norm)
{
    m3_t my = {
        cos(hor), 0, sin(hor),
        0, 1, 0,
        -sin(hor), 0, cos(hor)
    };

    m3_t mx = {
        1, 0, 0,
        0, cos(vert), -sin(vert),
        0, sin(vert), cos(vert)
    };

    v3_t center = s->viewport_props.center;
    center = v3_sub(center, s->viewport_props.eye);

    center = m3_v3t_mul(&my, &center);
    center = m3_v3t_mul(&mx, &center);
    s->viewport_props.center = v3_add(center, s->viewport_props.eye);

    calculate_mtrx(s);

    s->ls.d[0].pos.x += hor * 180 / 3.1415 * 10;
    s->ls.d[0].pos.z += vert * 180 / 3.1415 * 10;
}
