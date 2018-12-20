#include "models.h"
#include "error.h"


model_t *model_init(model_t *old, int v, int n, int f)
{
    int vs = v * sizeof(vertex_t);
    int ns = n * sizeof(normal_t);
    int fs = f * sizeof(face_t);

    model_t *m = heap_alloc(sizeof(model_t) + vs + ns + fs);
    if (!m)
        return NULL;

    char *after = (char *)(m + 1);
    char *it = after;

    m->vs.d = (vertex_t *)it;
    if (old)
    {
        memcpy(m->vs.d, old->vs.d, old->vs.l * sizeof(vertex_t));
        m->vs.l = old->vs.l;
    }
    else
    {
        m->vs.l = 0;
    }
    m->vs.alloc = v;
    it += vs;

    m->ns.d = (normal_t *)it;
    if (old)
    {
        memcpy(m->ns.d, old->ns.d, old->ns.l * sizeof(normal_t));
        m->ns.l = old->ns.l;
    }
    else
    {
        m->ns.l = 0;
    }
    m->ns.alloc = n;
    it += ns;

    m->fs.d = (face_t *)it;
    if (old)
    {
        memcpy(m->fs.d, old->fs.d, old->fs.l * sizeof(face_t));
        m->fs.l = old->fs.l;
    }
    else
    {
        m->fs.l = 0;
    }
    m->fs.alloc = f;

    m->props.shaders.f = 0;
    m->props.shaders.v = 0;

    if (old)
        heap_free(old);

    return m;
}

model_t *model_add_vertices_arr(model_t *m, vertex_t *vs, int n)
{
    if (m->vs.alloc - m->vs.l < n)
    {
        m = model_init(
            m, m->vs.alloc * 2 - m->vs.l >= n ? m->vs.alloc * 2 : m->vs.alloc + n,
            m->ns.alloc, m->fs.alloc);
    }

    vertex_t *dst = m->vs.d + m->vs.l;
    vertex_t *src = vs;
    for (int i = 0; i < n; i++, src++, dst++)
        *dst = *src;
    m->vs.l += n;

    return m;
}

model_t *model_add_normals_arr(model_t *m, v3_t *ns, int n)
{
    if (m->ns.alloc - m->ns.l < n)
    {
        m = model_init(
            m, m->vs.alloc,
            m->ns.alloc * 2 - m->ns.l >= n ? m->ns.alloc * 2 : m->ns.alloc + n,
            m->fs.alloc);
    }

    vertex_t *dst = m->ns.d + m->ns.l;
    vertex_t *src = ns;
    for (int i = 0; i < n; i++, src++, dst++)
        *dst = *src;
    m->ns.l += n;

    return m;
}

model_t *model_add_face_arr(model_t *m, vertexid_t *vs, normalid_t *ns, int n)
{
    if (m->fs.alloc - m->fs.l < 1)
    {
        m = model_init(
            m, m->vs.alloc, m->ns.alloc,
            m->fs.alloc * 2 - m->fs.l >= 1 ? m->fs.alloc * 2 : m->fs.alloc + 1);
    }

    face_t *f = m->fs.d + m->fs.l++;
    *f = model_face_static_init(n);

    vertexid_t *dstv = f->v + f->l;
    vertexid_t *srcv = vs;
    normalid_t *dstn = f->n + f->l;
    normalid_t *srcn = ns;
    for (int i = 0; i < n; i++, dstv++, dstn++, srcv++, srcn++)
    {
        *dstv = *srcv;
        *dstn = *srcn;
    }
    f->l += n;

    return m;
}

face_t model_face_static_init(int n)
{
    face_t f;
    int vs = n * sizeof (vertexid_t);
    int ns = n * sizeof (normalid_t);

    f.v = heap_alloc(vs + ns);
    f.n = (normalid_t *)(f.v + n);
    f.l = 0;
    f.alloc = n;

    return f;
}
