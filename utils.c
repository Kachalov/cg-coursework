#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include "scene.h"
#include "math.h"


evertex_t world2viewport(evertex_t v, scene_t *s)
{
    evertex_t r = v;
    //console_log("(%lf %lf %lf)", r.v.x, r.v.y, r.v.z);
    r.v = m4_v3t_mul(&s->view_mtrx, &r.v);
    //console_log("(%lf %lf %lf)", r.v.x, r.v.y, r.v.z);
    r.v = m4_v3t_mul(&s->proj_mtrx, &r.v);
    //console_log("(%lf %lf %lf)", r.v.x, r.v.y, r.v.z);

    return r;
}

evertex_t viewport2world(evertex_t v, scene_t *s)
{
    evertex_t r = v;

    return r;
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
int intersect_triangle(v3_t from, v3_t dir, v3_t v0, v3_t v1, v3_t v2)
{
    v3_t e1 = v3_sub(v1, v0);
    v3_t e2 = v3_sub(v2, v0);
    v3_t pv = v3_cross(dir, e2);
    float det = v3_dot(e1, pv);

    if (fpclassify(det) == FP_ZERO)
        return -1;

    float inv_det = 1 / det;
    v3_t tv = v3_sub(from, v0);
    float u = v3_dot(tv, pv) * inv_det;

    if (u < 0 || 1 < u)
        return -1;

    v3_t qv = v3_cross(tv, e1);
    float v = v3_dot(dir, qv) * inv_det;
    if (v < 0 || 1 < u + v)
        return -1;

    return v3_dot(e2, qv) * inv_det;
}

float v3_distance(v3_t a, v3_t b)
{
    v3_t s = v3_sub(a, b);
    return sqrt(v3_dot(s, s));
}

m4_t make_viewport(v3_t eye, v3_t center, v3_t up)
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

m4_t make_perspective(float angle, float ratio, float near, float far)
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
