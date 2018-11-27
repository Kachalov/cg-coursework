#pragma once


typedef struct evertex_st evertex_t;

evertex_t world2viewport(evertex_t v, scene_t *s);
int16_t find_line_x(point_t a, point_t b, int16_t y);
