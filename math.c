#include "math.h"


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
