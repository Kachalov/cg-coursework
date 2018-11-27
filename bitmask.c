#include <webassembly.h>

#include "bitmask.h"


export
bitmask_t *bitmask_init(int w, int h)
{
    int bw = w / BITMASK_CHUNK_SIZE + sign(w % BITMASK_CHUNK_SIZE);
    int bh = h;
    // console_log("Bitmask(%d, %d)", bw, bh);
    bitmask_t *ptr = heap_alloc(sizeof(bitmask_t) + bw * bh);
    ptr->w = bw;
    ptr->h = bh;
    ptr->data = (bitmask_data_t *)(ptr + 1);
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
    a.x = max(0, min(bmask->w * BITMASK_CHUNK_SIZE, a.x));
    a.y = max(0, min(bmask->h, a.y));

    int a_byte = a.x / BITMASK_CHUNK_SIZE;
    int a_bit = a.x % BITMASK_CHUNK_SIZE;

    int b_byte = b.x / BITMASK_CHUNK_SIZE;
    int b_bit = b.x % BITMASK_CHUNK_SIZE;

    bitmask_data_t a_mask = ((bitmask_data_t)((1 << (a_bit + 1)) - 1) & BITMASK_CHUNK_MASK) ^ BITMASK_CHUNK_MASK;
    bitmask_data_t b_mask = (bitmask_data_t)((1 << (b_bit + 1)) - 1) & BITMASK_CHUNK_MASK;

    bitmask_data_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] ^= a_mask;
        it[b_byte] ^= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = ~it[i] & BITMASK_CHUNK_MASK;
        }
    }
}

export
void bitmask_set(bitmask_t *bmask, point_t a, point_t b)
{
    a.x = max(0, min(bmask->w * BITMASK_CHUNK_SIZE, a.x));
    a.y = max(0, min(bmask->h, a.y));

    int a_byte = a.x / BITMASK_CHUNK_SIZE;
    int a_bit = a.x % BITMASK_CHUNK_SIZE;

    int b_byte = b.x / BITMASK_CHUNK_SIZE;
    int b_bit = b.x % BITMASK_CHUNK_SIZE;

    bitmask_data_t a_mask = ((bitmask_data_t)((1 << (a_bit + 1)) - 1) & BITMASK_CHUNK_MASK) ^ BITMASK_CHUNK_MASK;
    bitmask_data_t b_mask = (bitmask_data_t)((1 << (b_bit + 1)) - 1) & BITMASK_CHUNK_MASK;

    bitmask_data_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] |= a_mask;
        it[b_byte] |= b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            it[i] = BITMASK_CHUNK_MASK;
        }
    }
}

export
void bitmask_unset(bitmask_t *bmask, point_t a, point_t b)
{
    a.x = max(0, min(bmask->w * BITMASK_CHUNK_SIZE, a.x));
    a.y = max(0, min(bmask->h, a.y));

    int a_byte = a.x / BITMASK_CHUNK_SIZE;
    int a_bit = a.x % BITMASK_CHUNK_SIZE;

    int b_byte = b.x / BITMASK_CHUNK_SIZE;
    int b_bit = b.x % BITMASK_CHUNK_SIZE;

    bitmask_data_t a_mask = ((bitmask_data_t)((1 << (a_bit + 1)) - 1) & BITMASK_CHUNK_MASK) ^ BITMASK_CHUNK_MASK;
    bitmask_data_t b_mask = (bitmask_data_t)((1 << (b_bit + 1)) - 1) & BITMASK_CHUNK_MASK;

    bitmask_data_t *it = bmask->data + bmask->w * a.y;
    for (int j = a.y; j < b.y; j++, it+=bmask->w)
    {
        it[a_byte] &= ~a_mask;
        it[b_byte] &= ~b_mask;
        for (int i = a_byte + 1; i < b_byte - 1; i++)
        {
            // TODO (25.11.2018): Some shitty magic with RAM happens here
            it[i] = 0;
        }
    }
}

export
void log_bitmask_row(bitmask_t *bmask, int row)
{
    char *str = heap_alloc(bmask->w * BITMASK_CHUNK_SIZE + 1);
    for (int byte = 0; byte < bmask->w; byte++)
        for (int bit = 0; bit < BITMASK_CHUNK_SIZE; bit++)
            str[byte * BITMASK_CHUNK_SIZE + bit] = bmask->data[bmask->w * row + byte] & (1 << bit) ? '1' : '0';
    str[bmask->w * BITMASK_CHUNK_SIZE] = 0;
    // LOG
    heap_free(str);
}
