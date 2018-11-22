#include <webassembly.h>

#include "bitmask.h"


export
bitmask_t *bitmask_init(int w, int h)
{
    int bw = w / 8 + sign(w % 8);
    int bh = h / 8 + sign(h % 8);
    bitmask_t *ptr = heap_alloc(sizeof(bitmask_t) + bw * bh);
    ptr->w = bw;
    ptr->h = bh;
    ptr->data = (uint8_t *)(ptr + 1);
    return ptr;
}

export
void bitmask_free(bitmask_t *ptr)
{
    heap_free(ptr);
}

export
void bitmask_invert(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] ^= a_mask;
        it[b_byte] ^= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = ~it[i] & 0xff;
        }
    }
}

export
void bitmask_set(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] |= a_mask;
        it[b_byte] |= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = 0xff;
        }
    }
}

export
void bitmask_unset(bitmask_t *bmask, point_t a, point_t b)
{
    int a_byte = a.x / 8;
    int a_bit = a.x % 8;

    int b_byte = b.x / 8;
    int b_bit = b.x % 8;

    int a_mask = ((uint8_t)((1 << (a_bit + 1)) - 1) & 0xff) ^ 0xff;
    int b_mask = (uint8_t)((1 << (b_bit + 1)) - 1) & 0xff;

    uint8_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] &= a_mask;
        it[b_byte] &= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = 0;
        }
    }
}

export
void log_bitmask_row(bitmask_t *bmask, int row)
{
    char *str = heap_alloc(bmask->w * 8 + 1);
    for (int byte = 0; byte < bmask->w; byte++)
        for (int bit = 0; bit < 8; bit++)
            str[byte * 8 + bit] = bmask->data[bmask->w * row + byte] & (1 << bit) ? '1' : '0';
    str[bmask->w * 8] = 0;
    // LOG
    heap_free(str);
}
