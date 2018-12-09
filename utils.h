#pragma once


typedef struct evertex_st evertex_t;

evertex_t world2viewport(evertex_t v, scene_t *s);
evertex_t viewport2world(evertex_t v, scene_t *s);
int16_t find_line_x(point_t a, point_t b, int16_t y);
int intersect_triangle(v3_t from, v3_t dir, v3_t v0, v3_t v1, v3_t v2);
float v3_distance(v3_t a, v3_t b);
m4_t make_viewport(v3_t eye, v3_t center, v3_t up);
m4_t make_perspective(float angle, float ratio, float near, float far);
