#include <inttypes.h>
#include <math.h>

#include "utils.h"
#include "scene.h"
#include "math.h"
#include "shaders.h"


evertex_t world2viewport(evertex_t v, scene_t *s)
{
    evertex_t r = v;
    v4_t v4;
    v3_t v3;

    /*static float tmp[8];
    static float res[8];

    tmp[0] = r.v.x;
    tmp[1] = r.n.x;
    tmp[2] = r.v.y;
    tmp[3] = r.n.y;
    tmp[4] = r.v.z;
    tmp[5] = r.n.z;
    tmp[6] = 0;
    tmp[7] = 0;*/

    /*v4 = m4_v3t_mul_v4(&s->mvp_mtrx, &r.v);
    v3 = (v3_t){v4.x, v4.y, v4.z};
    //console_log("MVP %lf %lf %lf %lf", v4.x, v4.y, v4.z, v4.w);
    if (v4.w)
        v3 = v3_scale(v3, 1.0/v4.w);
    //console_log("MVP w=1 %lf %lf %lf", r.v.x, r.v.y, r.v.z);
    r.v = m4_v3t_mul(&s->viewport_mtrx, &v3);

    v4 = m4_v3t_mul_v4(&s->mvp_mtrx, &r.n);
    v3 = (v3_t){v4.x, v4.y, v4.z};
    if (v4.w)
        v3 = v3_scale(v3, 1.0/v4.w);
    r.n = m4_v3t_mul(&s->viewport_mtrx, &v3);*/

    m3_t rx = make_rot_x((90 + 40) * 1.0/180*3.1415);
    m3_t ry = make_rot_y((0) * 1.0/180*3.1415);
    m3_t rz = make_rot_z((30) * 1.0/180*3.1415);
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

    v3 = m4_v3t_mul(/*&(m4_t){
        sqrt(1./2), 0, -sqrt(1./2), 0,
        sqrt(1./6), sqrt(2./3), sqrt(1./6), 0,
        sqrt(1./3), -sqrt(1./3), sqrt(1./3), 0,
        0, 0, 0, 1
    }*/&mr, &r.v);
    r.v = (v3_t){
        v3.x + s->canv->w / 2,
        v3.y + s->canv->h / 2,
        v3.z - s->viewport_props.eye.z
    };

    /*mtrx_mul(&res, &s->mvp_mtrx, 4, 4, &tmp, 4, 2);

    r.v = (v3_t){tmp[0], tmp[2], tmp[4]};
    r.n = (v3_t){tmp[1], tmp[3], tmp[5]};*/

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
float intersect_triangle(v3_t from, v3_t dir, v3_t v0, v3_t v1, v3_t v2)
{
    v3_t e1 = v3_sub(v1, v0);
    v3_t e2 = v3_sub(v2, v0);
    v3_t pv = v3_cross(dir, e2);
    float det = v3_dot(e1, pv);
    float t;

    /*console_log("int v2(%lf %lf %lf)", v2.x, v2.y,v2.z);
    console_log("int v1(%lf %lf %lf)", v1.x, v1.y,v1.z);
    console_log("int v0(%lf %lf %lf)", v0.x, v0.y,v0.z);
    console_log("int e1(%lf %lf %lf)", e1.x, e1.y,e1.z);
    console_log("int e2(%lf %lf %lf)", e2.x, e2.y,e2.z);
    console_log("int dir(%lf %lf %lf)", dir.x, dir.y,dir.z);
    console_log("int pv(%lf %lf %lf)", pv.x, pv.y,pv.z);
    console_log("INT %lf", det);*/

    if (fpclassify(det) == FP_ZERO)
        return -10;

    float inv_det = 1 / det;
    v3_t tv = v3_sub(from, v0);
    //console_log("int tv(%lf %lf %lf)", tv.x, tv.y,tv.z);
    float u = v3_dot(tv, pv) * inv_det;
    //console_log("int u %lf)", u);
    if (u < 0 || 1 < u)
        return -20;

    v3_t qv = v3_cross(tv, e1);
    //console_log("int qv(%lf %lf %lf)", qv.x, qv.y,qv.z);
    float v = v3_dot(dir, qv) * inv_det;
    //console_log("int v %lf)", v);
    if (v < 0 || 1 < u + v)
        return -30;

    //console_log("INTr %lf %lf", v3_dot(e2, qv), inv_det);
    t = v3_dot(e2, qv) * inv_det;
    //console_log("INTr=%lf)", t);
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
