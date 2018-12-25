#pragma once


typedef struct v3_st {
    float x;
    float y;
    float z;
} v3_t;

typedef struct v4_st {
    float x;
    float y;
    float z;
    float w;
} v4_t;

typedef struct m3_st {
    v3_t d[3];
} m3_t;

typedef struct m3x4_st {
    v4_t d[3];
} m3x4_t;

typedef struct m4x3_st {
    v3_t d[4];
} m4x3_t;

typedef struct m4_st {
    v4_t d[4];
} m4_t;

int sign (int x);
double pow2 (float x, int e);
int int_inter(int a, int b, float t);
float float_inter(float a, float b, float t);
int min(int a, int b);
int max(int a, int b);

int clamp(int v, int f, int t);
float clampf(float v, float f, float t);

v3_t v3_add(v3_t a, v3_t b);
v4_t v4_add(v4_t a, v4_t b);
v3_t v3_sub(v3_t a, v3_t b);
v3_t v3_cross(v3_t a, v3_t b);
v3_t v3_norm(v3_t a);
float v3_dot(v3_t a, v3_t b);
v3_t v3_scale(v3_t a, float k);
v4_t v4_scale(v4_t a, float k);

int mtrx_mul(float *res, float *a, int am, int an, float *b, int bm, int bn);
m3_t m3_m3_mul(m3_t *a, m3_t *b);
v3_t m3_v3t_mul(m3_t *m, v3_t *v);
m4_t m4_m4_mul(m4_t *a, m4_t *b);
m4_t m4_transp(m4_t *a);
v3_t m4_v3t_mul(m4_t *m, v3_t *v);
v4_t m4_v3t_mul_v4(m4_t *m, v3_t *v);
