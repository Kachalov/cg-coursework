#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include "scene.h"
#include "math.h"
#include "shaders.h"


evertex_t world2viewport(evertex_t v, scene_t *s)
{
    evertex_t r = v;
    v3_t v3;

    v3 = m4_v3t_mul(&s->ortho_mtrx, &r.v);
    r.v = (v3_t){
        v3.x + s->canv->w / 2,
        v3.y + s->canv->h / 2,
        v3.z - s->viewport_props.eye.z
    };

    v3 = m4_v3t_mul(&s->ortho_mtrx, &r.n);
    r.n = v3_norm((v3_t){
        v3.x + s->canv->w / 2,
        v3.y + s->canv->h / 2,
        v3.z - s->viewport_props.eye.z
    });

    return r;
}

evertex_t viewport2world(evertex_t v, scene_t *s)
{
    evertex_t r = v;

    return r;
}

evertex_t vertex2evertex(vertex_t v, v3_t n, scene_t *s)
{
    evertex_t r;

    r.wv = v;
    r.wn = n;

    r.v = v;
    r.n = n;
    r = world2viewport(r, s);

    r.c = (rgba_t){0, 0, 0, 0};
    r.light = (frgba_t){0, 0, 0, 0};

    return r;
}

v4_t rgba2v4(rgba_t rgba)
{
    return (v4_t) {
        rgba.r,
        rgba.g,
        rgba.b,
        rgba.a
    };
}

v4_t frgba2v4(frgba_t frgba)
{
    return (v4_t) {
        frgba.r,
        frgba.g,
        frgba.b,
        frgba.a
    };
}

rgba_t v42rgba(v4_t v)
{
    return (rgba_t) {
        clamp(v.x, 0, 255),
        clamp(v.y, 0, 255),
        clamp(v.z, 0, 255),
        clamp(v.w, 0, 255),
    };
}

frgba_t v42frgba(v4_t v)
{
    return (frgba_t) {
        v.x,
        v.y,
        v.z,
        v.w,
    };
}

int16_t find_line_x(point_t a, point_t b, int16_t y)
{
    if ((a.y <= b.y && (a.y > y || b.y < y)) ||
        (a.y > b.y && (b.y > y || a.y < y)))
        return INT16_MIN;

    if (a.y == y)
        return a.x;

    if (b.y == y)
        return b.x;

    int dx = b.x - a.x;
    int dy = b.y - a.y;

    if (dx == 0)
        return min(a.x, b.x);

    dx = abs(a.x - b.x);
    dy = abs(a.y - b.y);
    int steep = dy > dx;

    if (steep)
    {
        int tmp = a.x;
        a.x = a.y;
        a.y = tmp;

        tmp = b.x;
        b.x = b.y;
        b.y = tmp;

        dx = abs(a.x - b.x);
        dy = abs(a.y - b.y);
    }

    if (a.x > b.x)
    {
        point_t tmp = a;
        a = b;
        b = tmp;

        dx = abs(a.x - b.x);
        dy = abs(a.y - b.y);
    }

    int err = 0;
    int derr = dy;
    int yi = a.y;
    int sy = sign(b.y - a.y);

    for (int x = a.x; x <= b.x; x++)
    {
        if ((steep && x == y) || (!steep && yi == y)) return steep ? yi : x;
        err += derr;
        if (2 * err >= dx)
        {
            yi += sy;
            err -= dx;
        }
    }

    return (int)round(a.x + ((double)((y - a.y) * dx))/dy);
}

/**
 * http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
 */
float intersect_triangle(v3_t from, v3_t dir, v3_t v0, v3_t v1, v3_t v2)
{
    v3_t e1 = v3_sub(v1, v0);
    v3_t e2 = v3_sub(v2, v0);
    v3_t pv = v3_cross(dir, e2);
    float det = v3_dot(e1, pv);
    float t;

    if (fpclassify(det) == FP_ZERO)
        return -10;

    float inv_det = 1 / det;
    v3_t tv = v3_sub(from, v0);
    float u = v3_dot(tv, pv) * inv_det;
    if (u < 0 || 1 < u)
        return -20;

    v3_t qv = v3_cross(tv, e1);
    float v = v3_dot(dir, qv) * inv_det;
    if (v < 0 || 1 < u + v)
        return -30;

    t = v3_dot(e2, qv) * inv_det;
    return t;
}

float v3_distance(v3_t a, v3_t b)
{
    v3_t s = v3_sub(a, b);
    return sqrt(v3_dot(s, s));
}

m4_t make_view(v3_t eye, v3_t center, v3_t up)
{
    m4_t m;

    v3_t z = v3_norm(v3_sub(eye, center));
    v3_t y = up;
    v3_t x = v3_cross(y, z);
    y = v3_norm(v3_cross(z, x));
    x = v3_norm(x);

    m.d[0].x = x.x;
    m.d[1].x = x.y;
    m.d[2].x = x.z;
    m.d[3].x = -v3_dot(x, eye);
    m.d[0].y = y.x;
    m.d[1].y = y.y;
    m.d[2].y = y.z;
    m.d[3].y = -v3_dot(y, eye);
    m.d[0].z = z.x;
    m.d[1].z = z.y;
    m.d[2].z = z.z;
    m.d[3].z = -v3_dot(z, eye);
    m.d[0].w = 0;
    m.d[1].w = 0;
    m.d[2].w = 0;
    m.d[3].w = 1;

    return m;
}

m4_t make_viewport(int x, int y, int w, int h)
{
    m4_t m;

    for (float *it = (float *)(&m); it < (float *)(&m) + 16; it++)
        *it = 0;

    m.d[0].w = x + w * 1.0/2;
    m.d[1].w = y + h * 1.0/2;
    m.d[2].w = 1 * 1.0/2;
    m.d[3].w = 1;

    m.d[0].x = w * 1.0/2;
    m.d[1].y = h * 1.0/2;
    m.d[2].z = 1 * 1.0/2;

    return m;
}

m4_t make_projection(float angle, float ratio, float near, float far)
{
    m4_t m;
    float tan_half_angle = tan(angle / 2);

    m.d[0].x = 1.0 / (ratio * tan_half_angle);
    m.d[1].y = 1.0 / tan_half_angle;
    m.d[2].z = -(far + near) / (far - near);
    m.d[3].z = -1;
    m.d[2].w = -(2 * far * near) / (far - near);

    m.d[0].y = m.d[0].z = m.d[0].w = \
    m.d[1].x = m.d[1].z = m.d[1].w = \
    m.d[2].x = m.d[2].y = \
    m.d[3].x = m.d[3].y = m.d[3].w = 0;
    return m;
}

m3_t make_rot_x(float a)
{
    m3_t m = {
        1, 0, 0,
        0, cos(a), -sin(a),
        0, sin(a), cos(a)
    };

    return m;
}

m3_t make_rot_y(float a)
{
    m3_t m = {
        cos(a), 0, sin(a),
        0, 1, 0,
        -sin(a), 0, cos(a)
    };

    return m;
}

m3_t make_rot_z(float a)
{
    m3_t m = {
        cos(a), -sin(a), 0,
        sin(a), cos(a), 0,
        0, 0, 1
    };

    return m;
}

m4_t make_rot(float x, float y, float z)
{
    m3_t rx = make_rot_x(x);
    m3_t ry = make_rot_y(y);
    m3_t rz = make_rot_z(z);
    m3_t rr = m3_m3_mul(&rx, &ry);
    rr = m3_m3_mul(&rr, &rz);
    m4_t mr;
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
        {
            ((float *)&mr)[i*4+j] = ((float *)&rr)[i*3+j];
        }

    mr.d[0].w = mr.d[1].w = mr.d[2].w = mr.d[3].x = mr.d[3].y = mr.d[0].z = 0;
    mr.d[3].w = 1;

    return mr;
}
