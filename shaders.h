#pragma once

typedef struct v3_st vertex_t;
typedef struct pixel_st pixel_t;
typedef struct scene_st scene_t;
typedef struct mat_st mat_t;
typedef struct evertex_st evertex_t;


typedef evertex_t (*shader_v_t)(evertex_t *vs, int i, scene_t *s);
typedef pixel_t (*shader_f_t)(evertex_t a, const mat_t *mat, scene_t *s);

typedef struct shaders_st {
    shader_f_t f;
    shader_v_t v;
    union {
        // TODO (27.11.18): Use params
        struct {
            uint8_t norm:1;
            uint8_t col:1;
        } interp_params;
        uint8_t interp_mask;
    };
} shaders_t;

pixel_t none_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t none_shader_v(const evertex_t *vs, int i, scene_t *s);

pixel_t plain_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t plain_shader_v(evertex_t *vs, int i, scene_t *s);

pixel_t lambert_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t lambert_shader_v(const evertex_t *vs, int i, scene_t *s);

pixel_t guro_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t guro_shader_v(const evertex_t *vs, int i, scene_t *s);

pixel_t phong_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t phong_shader_v(const evertex_t *vs, int i, scene_t *s);

pixel_t blinn_phong_shader_f(const evertex_t a, const mat_t *mat, scene_t *s);
evertex_t blinn_phong_shader_v(const evertex_t *vs, int i, scene_t *s);

#define CHECK_VISIBILITY(_r, _v, _n, _eps) \
if (v3_dot(_v, _n) < -_eps) \
{ \
    (_r).pos.x = -1; \
    (_r).pos.y = -1; \
    (_r).col.a = 0; \
    return (_r); \
}
