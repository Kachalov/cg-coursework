#include "math.h"
#include "error.h"


int sign (int x)
{
    return x > 0 ? 1 : (x < 0 ? -1 : 0);
}

double pow2 (float x, int e)
{
    return x * x;
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

int mtrx_mul(float *res, float *a, int am, int an, float *b, int bm, int bn)
{
    if (an != bm)
        return EOOM;

    float sum;

    for (int i = 0; i < am; i++)
        for (int j = 0; j < bn; j++)
        {
            sum = 0;
            for (int k = 0; k < an; k++)
                sum += a[i * am + k] * b[k * an + j];
            res[i * bm + j] = sum;
        }

    return EOK;
}

m3_t m3_m3_mul(m3_t *a, m3_t *b)
{
    m3_t m;
    mtrx_mul((float *)(&m), (float *)a, 3, 3, (float *)b, 3, 3);
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
