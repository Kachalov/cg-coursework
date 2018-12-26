#pragma once


typedef struct evertex_st evertex_t;

evertex_t world2viewport(evertex_t v, scene_t *s);
evertex_t viewport2world(evertex_t v, scene_t *s);
evertex_t vertex2evertex(vertex_t v, v3_t n, scene_t *s);
v4_t rgba2v4(rgba_t rgba);
v4_t frgba2v4(frgba_t frgba);
rgba_t v42rgba(v4_t v);
frgba_t v42frgba(v4_t v);
int16_t find_line_x(point_t a, point_t b, int16_t y);
float intersect_triangle(v3_t from, v3_t dir, v3_t v0, v3_t v1, v3_t v2);
float v3_distance(v3_t a, v3_t b);
m4_t make_view(v3_t eye, v3_t center, v3_t up);
m4_t make_viewport(int x, int y, int w, int h);
m4_t make_projection(float angle, float ratio, float near, float far);
m3_t make_rot_x(float a);
m3_t make_rot_y(float a);
m3_t make_rot_z(float a);
m4_t make_rot(float x, float y, float z);
