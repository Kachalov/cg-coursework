#include "math.h"
#include "error.h"


int sign (int x)
{
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

double pow2 (float x, int e)
{
    float r = x;
    for (int i = 1; i < e; i++)
        r *= x;
    return r;
}

int int_inter(int a, int b, float t)
{
    return (b - a) * t + a;
}

float float_inter(float a, float b, float t)
{
    return (b - a) * t + a;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int clamp(int v, int f, int t)
{
    return v > t ? t : v < f ? f : v;
}

float clampf(float v, float f, float t)
{
    return v > t ? t : v < f ? f : v;
}

v3_t v3_add(v3_t a, v3_t b)
{
    return (v3_t){a.x + b.x, a.y + b.y, a.z + b.z};
}

v3_t v3_sub(v3_t a, v3_t b)
{
    return (v3_t){a.x - b.x, a.y - b.y, a.z - b.z};
}

v3_t v3_cross(v3_t a, v3_t b)
{
    return (v3_t){
        a.y * b.z - b.y * a.z,
        a.z * b.x - b.z * a.x,
        a.x * b.y - b.x - a.y
    };
}

v3_t v3_norm(v3_t a)
{
    float len = sqrt(v3_dot(a, a));
    return (v3_t){a.x / len, a.y / len, a.z / len};
}

float v3_dot(v3_t a, v3_t b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

v3_t v3_scale(v3_t a, float k)
{
    return (v3_t){
        a.x * k,
        a.y * k,
        a.z * k
    };
}

int mtrx_mul(float *res, float *a, int am, int an, float *b, int bm, int bn)
{
    if (an != bm)
        return EOOM;

    for (int i = 0; i < am * bn; i++)
        res[i] = 0;

    float *resit = res;

    for (int i = 0; i < am; i++)
        for (int j = 0; j < bn; j++, resit++)
            for (int k = 0; k < an; k++)
                *resit += a[i * an + k] * b[k * bn + j];

    return EOK;
}

m3_t m3_m3_mul(m3_t *a, m3_t *b)
{
    m3_t m;
    mtrx_mul((float *)(&m), (float *)a, 3, 3, (float *)b, 3, 3);
    return m;
}

m4_t m4_m4_mul(m4_t *a, m4_t *b)
{
    m4_t m;
    mtrx_mul((float *)&m, (float *)a, 4, 4, (float *)b, 4, 4);
    return m;
}

m4_t m4_transp(m4_t *a)
{
    m4_t m;

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            ((float *)&m)[i*4 + j] = ((float *)a)[j*4 + i];

    return m;
}

v3_t m4_v3t_mul(m4_t *m, v3_t *v)
{
    v3_t r;
    v4_t v4, r4;
    memcpy(&v4, v, sizeof(v3_t));
    v4.w = 1;

    mtrx_mul((float *)(&r4), (float *)m, 4, 4, (float *)(&v4), 4, 1);
    memcpy(&r, &r4, sizeof(v3_t));
    return r;
}

v4_t m4_v3t_mul_v4(m4_t *m, v3_t *v)
{
    v4_t v4, r;
    memcpy(&v4, v, sizeof(v3_t));
    v4.w = 1;

    mtrx_mul((float *)(&r), (float *)m, 4, 4, (float *)(&v4), 4, 1);
    return r;
}

v3_t m3_v3t_mul(m3_t *m, v3_t *v)
{
    v3_t r;
    mtrx_mul((float *)(&r), (float *)m, 3, 3, (float *)(v), 3, 1);
    return r;
}
