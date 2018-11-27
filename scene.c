#include <webassembly.h>

#include "scene.h"
#include "models.h"
#include "render.h"


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

    return r;
}
// End of test part

export
scene_t *scene_init(uint32_t w, uint32_t h, rgba_t *canv, uint8_t *zbuf)
{
    int canv_s = canv ? 0 : sizeof(canvas_t);
    int zbuf_s = zbuf ? 0 : sizeof(zbuf_t);

    scene_t *s = heap_alloc(sizeof(scene_t));
    char *after = (char *)(s + 1);

    s->canv = canvas_init(w, h, canv ? canv : (rgba_t *)(after + canv_s));
    s->zbuf = zbuf_init(w, h, zbuf ? zbuf : (uint8_t *)(after + canv_s + zbuf_s));

    s->bmask_row = bitmask_init(w, 1);

    s->ls.d = 0;
    s->ls.l = 0;

    s->cams.d = 0;
    s->cams.l = 0;
    s->cams.cur = 0;

    s->models.d = 0;
    s->models.l = 0;

    // TODO (27.11.18): Remove test part
    model_t *m = model_init(3, 3, 1);
    m->vs.d[0].x=100;
    m->vs.d[0].y=100;
    m->vs.d[0].z=0;

    m->vs.d[1].x=100;
    m->vs.d[1].y=200;
    m->vs.d[1].z=100;

    m->vs.d[2].x=300;
    m->vs.d[2].y=200;
    m->vs.d[2].z=-100;

    m->fs.d[0].d = heap_alloc(sizeof (uint32_t) * 3);
    m->fs.d[0].l = 0;

    m->fs.d[0].d[0] = 0;
    m->fs.d[0].d[1] = 1;
    m->fs.d[0].d[2] = 2;

    m->props.mat.ambient.r = 0;
    m->props.mat.ambient.g = 0;
    m->props.mat.ambient.b = 0;
    m->props.mat.ambient.a = 255;

    m->props.shaders.f = test_shader_f;
    m->props.shaders.v = 0;

    scene_add_model(s, m);
    // End of test part

    scene = s;
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
zbuf_t *zbuf_init(int w, int h, uint8_t *data)
{
    int size_data = data ? 0 : w * h * sizeof(uint8_t);
    int size = sizeof(zbuf_t) + size_data;
    zbuf_t *r = (zbuf_t *)heap_alloc(size);

    r->data = data ? data : (uint8_t *)(r + 1);
    r->w = w;
    r->h = h;

    return r;
}

export void clear(scene_t *s)
{
    rgba_t *it = (rgba_t *)s->canv->data;

    for (int i = 0; i < s->canv->h * s->canv->w; i++, it++)
    {
        it->a = 0;
    }
}
