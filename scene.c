#include <webassembly.h>
#include <math.h>

#include "scene.h"
#include "models.h"
#include "render.h"
#include "math.h"
#include "utils.h"


void scene_example(scene_t *s)
{
    scene_create_cube(
        s, 100, 0, -5, 4,
        rgba2int((rgba_t){255, 140, 80, 255}),
        100, 100, 100);

    scene_create_sphere(
        s, -60, 200, 20, 4,
        rgba2int((rgba_t){232, 224, 142, 255}),
        20, 100);

    scene_create_light(
        s, -150, 50, 50,
        rgba2int((rgba_t){255, 242, 194, 100}));

    {
        m4_t m = s->mvp_mtrx;
        console_log("mvp");
        console_log("|%lf; %lf; %lf; %lf|", m.d[0].x, m.d[0].y, m.d[0].z, m.d[0].w);
        console_log("|%lf; %lf; %lf; %lf|", m.d[1].x, m.d[1].y, m.d[1].z, m.d[1].w);
        console_log("|%lf; %lf; %lf; %lf|", m.d[2].x, m.d[2].y, m.d[2].z, m.d[2].w);
        console_log("|%lf; %lf; %lf; %lf|", m.d[3].x, m.d[3].y, m.d[3].z, m.d[3].w);
    }

    model_t *m = model_init(NULL, 4, 1, 2);

    vertex_t vs[] = {
        {-300, 300, -0.8},
        {-300, -300, -0.8},
        {300, -300, -0.8},
        {300, 300, -0.8}
    };
    m = model_add_vertices_arr(m, vs, 4);

    v3_t ns[] = {{0, 0, 1}};
    m = model_add_normals_arr(m, ns, 1);

    normalid_t nid[] = {0, 0, 0};
    vertexid_t vid[3] = {0, 1, 2};

    m = model_add_face_arr(m, vid, nid, 3);
    memcpy(vid, &((vertexid_t[3]){0, 2, 3}), sizeof(vertexid_t[3]));
    m = model_add_face_arr(m, vid, nid, 3);

    m->props.mat.ambient = (rgba_t){255, 255, 255, 255};

    m->props.shaders.f = phong_shader_f;
    m->props.shaders.v = phong_shader_v;

    scene_add_model(s, m);
}

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

    s->viewport_props.eye = (v3_t){0, 0, -400};//{1, 0.4, -1};
    s->viewport_props.center = (v3_t){0, 0, 0};
    s->viewport_props.up = (v3_t){0, 1, 0};

    s->perspective_props.angle = 30;
    s->perspective_props.ratio = 1.0 * w / h;
    s->perspective_props.near = 0.1;
    s->perspective_props.far = 2000;

    calculate_mtrx(s);
    clear(s);
    scene = s;

    scene_example(s);

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

//TODO(20.12.18): refactor
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

    /*v3_t center = s->viewport_props.center;
    center = v3_sub(center, s->viewport_props.eye);

    center = m3_v3t_mul(&my, &center);
    center = m3_v3t_mul(&mx, &center);
    s->viewport_props.center = v3_add(center, s->viewport_props.eye);*/

    calculate_mtrx(s);

    s->ls.d[0].pos.x -= hor * 180 / 3.1415 * 10;
    s->ls.d[0].pos.y += vert * 180 / 3.1415 * 10;
}

export
int scene_add_light(scene_t *s, light_t *light)
{
    int so = s->ls.l;
    int sn = s->ls.l + 1;
    light_t *ptr = heap_realloc(s->ls.d, sizeof(light_t) * sn);
    if (!ptr)
        return -1;

    s->ls.d = ptr;
    s->ls.l = sn;
    s->ls.d[so] = *light;

    return so;
}

export
int scene_create_light(scene_t *s, int x, int y, int z, int ambient)
{
    light_t l;
    l.pos = (v3_t){x, y, z};
    l.attens = (light_attens_t){1, 0, 0};
    l.cols.ambient = int2rgba(ambient);
    l.cols.diffuse = (rgba_t){255, 255, 255, 255};
    l.cols.specular = (rgba_t){255, 255, 255, 255};

    return scene_add_light(s, &l);
}

export int scene_create_cube(scene_t *s, int x, int y, int z,
    int shader, int color, int w, int h, int d)
{
    model_t *m = model_init(NULL, 8, 6, 12);
    m->props.mat.ambient = int2rgba(color);

    vertex_t vs[8];
    float Z = 0; // TODO: Memset workaround
    v3_t ns[6] = {
        {0, 0, -1},
        {0, 0, 1},
        {-1, 0, 0},
        {1, 0, 0},
        {0, -1, Z},
        {0, 1, 0},
    };
    m = model_add_normals_arr(m, ns, 6);

    for (int i = 0; i < 8; i++)
    {
        vs[i] = (vertex_t){
            x + w * ((i & 1) != 0),
            y + h * ((i & 2) != 0),
            z + d * ((i & 4) != 0)
        };
    }
    m = model_add_vertices_arr(m, vs, 8);

    m = model_add_face_arr(m, (vertexid_t[3]){0, 1, 2}, (normalid_t[3]){0, 0, 0}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){2, 3, 1}, (normalid_t[3]){0, 0, 0}, 3);

    m = model_add_face_arr(m, (vertexid_t[3]){4, 5, 6}, (normalid_t[3]){1, 1, 1}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){6, 7, 5}, (normalid_t[3]){1, 1, 1}, 3);

    m = model_add_face_arr(m, (vertexid_t[3]){0, 2, 4}, (normalid_t[3]){2, 2, 2}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){4, 6, 2}, (normalid_t[3]){2, 2, 2}, 3);

    m = model_add_face_arr(m, (vertexid_t[3]){1, 3, 5}, (normalid_t[3]){3, 3, 3}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){5, 7, 3}, (normalid_t[3]){3, 3, 3}, 3);

    m = model_add_face_arr(m, (vertexid_t[3]){0, 1, 4}, (normalid_t[3]){4, 4, 4}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){4, 5, 1}, (normalid_t[3]){4, 4, 4}, 3);

    m = model_add_face_arr(m, (vertexid_t[3]){2, 3, 6}, (normalid_t[3]){5, 5, 5}, 3);
    m = model_add_face_arr(m, (vertexid_t[3]){6, 7, 3}, (normalid_t[3]){5, 5, 5}, 3);

    m->props.shaders.f = scene_select_shder_f(shader);
    m->props.shaders.v = scene_select_shder_v(shader);

    return scene_add_model(s, m);
}

export int scene_create_sphere(scene_t *s, int x, int y, int z,
    int shader, int color, int segs, int r)
{
    model_t *m = model_init(
        NULL,
        segs * (segs - 2) + 2,
        segs * (segs - 2) + 2,
        2 * segs * (segs - 1));
    m->props.mat.ambient = int2rgba(color);

    vertex_t vs[4];
    v3_t ns[4];

    vs[0] = (v3_t){x, y, z - r};
    ns[0] = (v3_t){0, 0, -1};
    vs[1] = (v3_t){x, y, z + r};
    ns[1] = (v3_t){0, 0, 1};

    m = model_add_normals_arr(m, ns, 2);
    m = model_add_vertices_arr(m, vs, 2);

    int vid = 2;

    v3_t rv = {r, 0, 0};
    v3_t rij;
    m4_t rot;

    int seg2 = (segs - 2)/2;
    int seg2p = (segs - 2)%2;
    int seg2np = 1 - seg2p;

    for (int i = -seg2-seg2p; i < seg2; i++)
    {
        for (int j = 0; j < segs; j++)
        {
            rot = make_rot(
                M_PI/2,
                j*2.0*M_PI/segs,
                (i + 0.5 * seg2np)*M_PI/segs);
            rij = m4_v3t_mul(&rot, &rv);
            vs[0] = (vertex_t){
                x + rij.x,
                y + rij.y,
                z + rij.z
            };
            ns[0] = v3_norm(rij);
            m = model_add_normals_arr(m, ns, 1);
            m = model_add_vertices_arr(m, vs, 1);
        }
    }

    vid = 2;
    for (int j = 0; j < segs - 3; j++)
    {
        for (int i = 0; i < segs; i++)
        {
            m = model_add_face_arr(
                m,
                (vertexid_t[3]){vid + i, vid + ((i + 1) % (segs)), vid + segs + i},
                (normalid_t[3]){vid + i, vid + ((i + 1) % (segs)), vid + segs + i}, 3);
            m = model_add_face_arr(
                m,
                (vertexid_t[3]){vid + ((i + 1) % (segs)), vid + segs + i, vid + segs + ((i + 1) % (segs))},
                (normalid_t[3]){vid + ((i + 1) % (segs)), vid + segs + i, vid + segs + ((i + 1) % (segs))}, 3);
        }
        vid += segs;
    }

    for (int i = 0; i < segs; i++)
    {
        vid = 2;
        m = model_add_face_arr(
            m,
            (vertexid_t[3]){0, vid + i, vid + ((i + 1) % (segs))},
            (normalid_t[3]){0, vid + i, vid + ((i + 1) % (segs))}, 3);
        vid = segs * (segs - 3) + 2;
        m = model_add_face_arr(
            m,
            (vertexid_t[3]){1, vid + i, vid + ((i + 1) % (segs))},
            (normalid_t[3]){1, vid + i, vid + ((i + 1) % (segs))}, 3);
    }

    m->props.shaders.f = scene_select_shder_f(shader);
    m->props.shaders.v = scene_select_shder_v(shader);

    return scene_add_model(s, m);
}

void *scene_select_shder_v(int n)
{
    switch(n)
    {
        case 0:
            return none_shader_v;
        case 1:
            return plain_shader_v;
        case 2:
            return none_shader_v;
        case 3:
            return none_shader_v;
        case 4:
            return phong_shader_v;
        case 5:
            return none_shader_v;
        default:
            return none_shader_v;
    }
}

void *scene_select_shder_f(int n)
{
    switch(n)
    {
        case 0:
            return none_shader_f;
        case 1:
            return plain_shader_f;
        case 2:
            return none_shader_f;
        case 3:
            return none_shader_f;
        case 4:
            return phong_shader_f;
        case 5:
            return none_shader_f;
        default:
            return none_shader_f;
    }
}
