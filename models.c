#include "models.h"


model_t *model_init(int v, int n, int f)
{
    int vs = v * sizeof(vertex_t);
    int ns = n * sizeof(normal_t);
    int fs = f * sizeof(face_t);

    model_t *m = heap_alloc(sizeof(model_t) + vs + ns + fs);
    char *after = (char *)(m + 1);
    char *it = after;

    m->vs.d = (vertex_t *)it;
    m->vs.l = v;
    it += vs;

    m->ns.d = (normal_t *)it;
    m->ns.l = n;
    it += ns;

    m->fs.d = (face_t *)it;
    m->fs.l = f;
    it += fs;

    m->props.shaders.f = 0;
    m->props.shaders.v = 0;

    /*m->props.mat.ambient = {255, 255, 255, 255};
    m->props.mat.diffuse = {255, 255, 255, 255};
    m->props.mat.specular = {255, 255, 255, 255};*/

    return m;
}
