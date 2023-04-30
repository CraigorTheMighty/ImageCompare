#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <commctrl.h>
#include <math.h>
#include <psapi.h>
#include <Dbghelp.h>
#include <inttypes.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <gl\wglext.h>

#include "..\inc\image.h"

//#define DEBUG_BLOCK (block_x == 149 && block_y == 74)
#define DEBUG_BLOCK (0)

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES	0x8D64
#endif
#ifndef GL_ETC1_SRGB8_NV
#define GL_ETC1_SRGB8_NV	0x88EE
#endif

#define __constant
#define __global

typedef uint8_t uchar;
typedef uint32_t uint;

__constant const int g_partition_indexes[2][2][8] =
{
    {
        {0, 1, 4, 5, 8, 9, 12, 13},
        {2, 3, 6, 7, 10, 11, 14, 15}
    },
    {
        {0, 1, 2, 3, 4, 5, 6, 7},
        {8, 9, 10, 11, 12, 13, 14, 15}
    }
};

__constant const int g_table[8][4] =
{
    {  2,  8,   -2,   -8 },
    {  5, 17,   -5,  -17 },
    {  9, 29,   -9,  -29 },
    { 13, 42,  -13,  -42 },
    { 18, 60,  -18,  -60 },
    { 24, 80,  -24,  -80 },
    { 33, 106, -33, -106 },
    { 47, 183, -47, -183 }
};

__constant const int g_etc2_tmode_distance[8] =
{
    3, 6, 11, 16, 23, 32, 41, 64
};

__constant const int g_alpha[16][8] =
{
    { -3, -6,  -9, -15, 2, 5, 8, 14 },
    { -3, -7, -10, -13, 2, 6, 9, 12 },
    { -2, -5,  -8, -13, 1, 4, 7, 12 },
    { -2, -4,  -6, -13, 1, 3, 5, 12 },
    { -3, -6,  -8, -12, 2, 5, 7, 11 },
    { -3, -7,  -9, -11, 2, 6, 8, 10 },
    { -4, -7,  -8, -11, 3, 6, 7, 10 },
    { -3, -5,  -8, -11, 2, 4, 7, 10 },
    { -2, -6,  -8, -10, 1, 5, 7,  9 },
    { -2, -5,  -8, -10, 1, 4, 7,  9 },
    { -2, -4,  -8, -10, 1, 3, 7,  9 },
    { -2, -5,  -7, -10, 1, 4, 6,  9 },
    { -3, -4,  -7, -10, 2, 3, 6,  9 },
    { -1, -2,  -3, -10, 0, 1, 2,  9 },
    { -4, -6,  -8,  -9, 3, 5, 7,  8 },
    { -3, -5,  -7,  -9, 2, 4, 6,  8 }
};
__constant const int g_alpha_range[16] =
{
    14 + 15,
    12 + 13,
    12 + 13,
    12 + 13,
    11 + 12,
    10 + 11,
    10 + 11,
    10 + 11,
    9 + 10,
    9 + 10,
    9 + 10,
    9 + 10,
    9 + 10,
    9 + 10,
    8 + 9,
    8 + 9
};


static uint ETC_ReadBits(uchar *block, uint start_bit, uchar num_bits)
{
    uint i = 0;
    uint result = 0;

    for (i = 0; i < num_bits; i++)
    {
        uchar src_byte = (start_bit + i) >> 3;
        uchar src_shift = (start_bit + i) & 0x07;
        uchar bit = (block[src_byte] >> src_shift) & 0x01;

        result |= bit << i;
    }
    return result;
}

static int ETC_SignExtend(uint x, uchar num_bits)
{
    int offset = -(1 << num_bits);
    if (x & (1 << (num_bits - 1)))
        return offset + x;
    else
        return x;
}

static float clamp(float x, float mins, float maxs)
{
    if (x < mins)
        return mins;
    else if (x > maxs)
        return maxs;
    else
        return x;
}
static int Math_Clampi32(int x, int mins, int maxs)
{
    if (x < mins)
        return mins;
    else if (x > maxs)
        return maxs;
    else
        return x;
}

static void ETC_BlockSwapOrder(__global uchar *out_block, uchar *in_block)
{
    out_block[0] = in_block[7];
    out_block[1] = in_block[6];
    out_block[2] = in_block[5];
    out_block[3] = in_block[4];
    out_block[4] = in_block[3];
    out_block[5] = in_block[2];
    out_block[6] = in_block[1];
    out_block[7] = in_block[0];
}


void ETC_DecodeETC1(float *block_out, uchar *block_in, int block_x, int block_y)
{
    int index[16];
    int diff;
    uchar flip;
    uint table[2];
    uint base_colour_u32[2][3];   // partition, rgb
    uint colour[2][4][3];       // partition, index, rgb
    int i;
    int j;
    uchar block[8];

    ETC_BlockSwapOrder(block, block_in);

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            index[j*4 + i] = ETC_ReadBits(block, i*4 + j, 1) | (ETC_ReadBits(block, 16 + i*4 + j, 1) << 1);

    flip = ETC_ReadBits(block, 32, 1);
    diff = ETC_ReadBits(block, 33, 1);
    table[1] = ETC_ReadBits(block, 34, 3);
    table[0] = ETC_ReadBits(block, 37, 3);

    if (diff)
    {
        int delta[3];

        base_colour_u32[0][0] = ETC_ReadBits(block, 59, 5);
        base_colour_u32[0][1] = ETC_ReadBits(block, 51, 5);
        base_colour_u32[0][2] = ETC_ReadBits(block, 43, 5);

        delta[0] = ETC_SignExtend(ETC_ReadBits(block, 56, 3), 3);
        delta[1] = ETC_SignExtend(ETC_ReadBits(block, 48, 3), 3);
        delta[2] = ETC_SignExtend(ETC_ReadBits(block, 40, 3), 3);

        base_colour_u32[1][0] = Math_Clampi32((int)base_colour_u32[0][0] + delta[0], 0, 31);
        base_colour_u32[1][1] = Math_Clampi32((int)base_colour_u32[0][1] + delta[1], 0, 31);
        base_colour_u32[1][2] = Math_Clampi32((int)base_colour_u32[0][2] + delta[2], 0, 31);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_u32[i][j] = (base_colour_u32[i][j] << 3) | (base_colour_u32[i][j] >> 2);
    }
    else
    {
        base_colour_u32[0][0] = ETC_ReadBits(block, 60, 4);
        base_colour_u32[0][1] = ETC_ReadBits(block, 52, 4);
        base_colour_u32[0][2] = ETC_ReadBits(block, 44, 4);
        base_colour_u32[1][0] = ETC_ReadBits(block, 56, 4);
        base_colour_u32[1][1] = ETC_ReadBits(block, 48, 4);
        base_colour_u32[1][2] = ETC_ReadBits(block, 40, 4);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_u32[i][j] = (base_colour_u32[i][j] << 4) | base_colour_u32[i][j];
    }
    for (i = 0; i < 2; i++)
        for (j = 0; j < 4; j++)
        {
            colour[i][j][0] = Math_Clampi32((int)base_colour_u32[i][0] + g_table[table[i]][j], 0, 255);
            colour[i][j][1] = Math_Clampi32((int)base_colour_u32[i][1] + g_table[table[i]][j], 0, 255);
            colour[i][j][2] = Math_Clampi32((int)base_colour_u32[i][2] + g_table[table[i]][j], 0, 255);
        }
    for (i = 0; i < 2; i++)
        for (j = 0; j < 8; j++)
        {
            int out_index = g_partition_indexes[flip][i][j];
            
            block_out[out_index * 4 + 0] = (float)colour[i][index[out_index]][0];
            block_out[out_index * 4 + 1] = (float)colour[i][index[out_index]][1];
            block_out[out_index * 4 + 2] = (float)colour[i][index[out_index]][2];
            block_out[out_index * 4 + 3] = 255.0f;
        }
}

void ETC_DecodeETC2RGB(float *block_out, uchar *block_in, int block_x, int block_y)
{
    int index[16];
    int diff;
    uchar flip;
    uint table[2];
    int base_colour_i32[2][3];   // partition, rgb
    uint colour[2][4][3];       // partition, index, rgb
    int i;
    int j;
    uchar block[8];
    int delta[3];

    ETC_BlockSwapOrder(block, block_in);

    diff = ETC_ReadBits(block, 33, 1);

    for (i = 0; i < 64; i++)
        block_out[i] = 0;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            index[j*4 + i] = ETC_ReadBits(block, i*4 + j, 1) | (ETC_ReadBits(block, 16 + i*4 + j, 1) << 1);

    if (diff == 0)
    {
        base_colour_i32[0][0] = ETC_ReadBits(block, 60, 4);
        base_colour_i32[0][1] = ETC_ReadBits(block, 52, 4);
        base_colour_i32[0][2] = ETC_ReadBits(block, 44, 4);
        base_colour_i32[1][0] = ETC_ReadBits(block, 56, 4);
        base_colour_i32[1][1] = ETC_ReadBits(block, 48, 4);
        base_colour_i32[1][2] = ETC_ReadBits(block, 40, 4);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_i32[i][j] = (base_colour_i32[i][j] << 4) | base_colour_i32[i][j];

        flip = ETC_ReadBits(block, 32, 1);
        table[1] = ETC_ReadBits(block, 34, 3);
        table[0] = ETC_ReadBits(block, 37, 3);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 4; j++)
            {
                colour[i][j][0] = Math_Clampi32((int)base_colour_i32[i][0] + g_table[table[i]][j], 0, 255);
                colour[i][j][1] = Math_Clampi32((int)base_colour_i32[i][1] + g_table[table[i]][j], 0, 255);
                colour[i][j][2] = Math_Clampi32((int)base_colour_i32[i][2] + g_table[table[i]][j], 0, 255);
            }

        for (i = 0; i < 2; i++)
            for (j = 0; j < 8; j++)
            {
                int out_index = g_partition_indexes[flip][i][j];

                block_out[out_index * 4 + 0] = (float)colour[i][index[out_index]][0];
                block_out[out_index * 4 + 1] = (float)colour[i][index[out_index]][1];
                block_out[out_index * 4 + 2] = (float)colour[i][index[out_index]][2];
                block_out[out_index * 4 + 3] = 255.0f;
            }
    }
    else
    {
        base_colour_i32[0][0] = ETC_ReadBits(block, 59, 5);
        base_colour_i32[0][1] = ETC_ReadBits(block, 51, 5);
        base_colour_i32[0][2] = ETC_ReadBits(block, 43, 5);

        delta[0] = ETC_SignExtend(ETC_ReadBits(block, 56, 3), 3);
        delta[1] = ETC_SignExtend(ETC_ReadBits(block, 48, 3), 3);
        delta[2] = ETC_SignExtend(ETC_ReadBits(block, 40, 3), 3);

        base_colour_i32[1][0] = base_colour_i32[0][0] + delta[0];
        base_colour_i32[1][1] = base_colour_i32[0][1] + delta[1];
        base_colour_i32[1][2] = base_colour_i32[0][2] + delta[2];

        if (base_colour_i32[1][0] < 0 || base_colour_i32[1][0] > 31) // T mode
        {
            int dist_index;

            base_colour_i32[0][0] = ETC_ReadBits(block, 56, 2) | (ETC_ReadBits(block, 59, 2) << 2);
            base_colour_i32[0][1] = ETC_ReadBits(block, 52, 4);
            base_colour_i32[0][2] = ETC_ReadBits(block, 48, 4);

            base_colour_i32[1][0] = ETC_ReadBits(block, 44, 4);
            base_colour_i32[1][1] = ETC_ReadBits(block, 40, 4);
            base_colour_i32[1][2] = ETC_ReadBits(block, 36, 4);

            for (i = 0; i < 2; i++)
                for (j = 0; j < 3; j++)
                    base_colour_i32[i][j] = (base_colour_i32[i][j] << 4) | base_colour_i32[i][j];

            dist_index = (ETC_ReadBits(block, 34, 2) << 1) | (ETC_ReadBits(block, 32, 1));

            colour[0][0][0] = base_colour_i32[0][0];
            colour[0][0][1] = base_colour_i32[0][1];
            colour[0][0][2] = base_colour_i32[0][2];

            colour[0][1][0] = Math_Clampi32(base_colour_i32[1][0] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][1][1] = Math_Clampi32(base_colour_i32[1][1] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][1][2] = Math_Clampi32(base_colour_i32[1][2] + g_etc2_tmode_distance[dist_index], 0, 255);

            colour[0][2][0] = base_colour_i32[1][0];
            colour[0][2][1] = base_colour_i32[1][1];
            colour[0][2][2] = base_colour_i32[1][2];

            colour[0][3][0] = Math_Clampi32(base_colour_i32[1][0] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][3][1] = Math_Clampi32(base_colour_i32[1][1] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][3][2] = Math_Clampi32(base_colour_i32[1][2] - g_etc2_tmode_distance[dist_index], 0, 255);

            for (i = 0; i < 16; i++)
            {
                block_out[i * 4 + 0] = (float)colour[0][index[i]][0];
                block_out[i * 4 + 1] = (float)colour[0][index[i]][1];
                block_out[i * 4 + 2] = (float)colour[0][index[i]][2];
                block_out[i * 4 + 3] = 255.0f;
            }
        }
        else if (base_colour_i32[1][1] < 0 || base_colour_i32[1][1] > 31) // H mode
        {
            int dist_index;
            uint32_t dist_bit1[2];

            base_colour_i32[0][0] = ETC_ReadBits(block, 59, 4);
            base_colour_i32[0][1] = ETC_ReadBits(block, 52, 1) | (ETC_ReadBits(block, 56, 3) << 1);
            base_colour_i32[0][2] = ETC_ReadBits(block, 47, 3) | (ETC_ReadBits(block, 51, 1) << 3);

            base_colour_i32[1][0] = ETC_ReadBits(block, 43, 4);
            base_colour_i32[1][1] = ETC_ReadBits(block, 39, 4);
            base_colour_i32[1][2] = ETC_ReadBits(block, 35, 4);

            dist_bit1[0] = (base_colour_i32[0][0] << 16) | (base_colour_i32[0][1] << 8) | base_colour_i32[0][2];
            dist_bit1[1] = (base_colour_i32[1][0] << 16) | (base_colour_i32[1][1] << 8) | base_colour_i32[1][2];

            for (i = 0; i < 2; i++)
                for (j = 0; j < 3; j++)
                    base_colour_i32[i][j] = (base_colour_i32[i][j] << 4) | base_colour_i32[i][j];

            dist_index = (ETC_ReadBits(block, 34, 1) << 2) | (ETC_ReadBits(block, 32, 1) << 1) | (dist_bit1[0] > dist_bit1[1] ? 1 : 0);

            colour[0][0][0] = Math_Clampi32(base_colour_i32[0][0] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][0][1] = Math_Clampi32(base_colour_i32[0][1] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][0][2] = Math_Clampi32(base_colour_i32[0][2] + g_etc2_tmode_distance[dist_index], 0, 255);

            colour[0][1][0] = Math_Clampi32(base_colour_i32[0][0] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][1][1] = Math_Clampi32(base_colour_i32[0][1] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][1][2] = Math_Clampi32(base_colour_i32[0][2] - g_etc2_tmode_distance[dist_index], 0, 255);

            colour[0][2][0] = Math_Clampi32(base_colour_i32[1][0] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][2][1] = Math_Clampi32(base_colour_i32[1][1] + g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][2][2] = Math_Clampi32(base_colour_i32[1][2] + g_etc2_tmode_distance[dist_index], 0, 255);

            colour[0][3][0] = Math_Clampi32(base_colour_i32[1][0] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][3][1] = Math_Clampi32(base_colour_i32[1][1] - g_etc2_tmode_distance[dist_index], 0, 255);
            colour[0][3][2] = Math_Clampi32(base_colour_i32[1][2] - g_etc2_tmode_distance[dist_index], 0, 255);

            for (i = 0; i < 16; i++)
            {
                block_out[i * 4 + 0] = (float)colour[0][index[i]][0];
                block_out[i * 4 + 1] = (float)colour[0][index[i]][1];
                block_out[i * 4 + 2] = (float)colour[0][index[i]][2];
                block_out[i * 4 + 3] = 255.0f;
            }
        }
        else if (base_colour_i32[1][2] < 0 || base_colour_i32[1][2] > 31) // Planar mode
        {
            int base_colour[3];
            int uv_colour[2][3];

            base_colour[0] = ETC_ReadBits(block, 57, 6);
            base_colour[1] = ETC_ReadBits(block, 49, 6) | (ETC_ReadBits(block, 56, 1) << 6);
            base_colour[2] = ETC_ReadBits(block, 39, 3) | (ETC_ReadBits(block, 43, 2) << 3) | (ETC_ReadBits(block, 48, 1) << 5);

            uv_colour[0][0] = ETC_ReadBits(block, 32, 1) | (ETC_ReadBits(block, 34, 5) << 1); 
            uv_colour[0][1] = ETC_ReadBits(block, 25, 7); 
            uv_colour[0][2] = ETC_ReadBits(block, 19, 6); 

            uv_colour[1][0] = ETC_ReadBits(block, 13, 6); 
            uv_colour[1][1] = ETC_ReadBits(block, 6, 7); 
            uv_colour[1][2] = ETC_ReadBits(block, 0, 6); 

            base_colour[0] = (base_colour[0] << 2) | (base_colour[0] >> 4);
            base_colour[1] = (base_colour[1] << 1) | (base_colour[1] >> 6);
            base_colour[2] = (base_colour[2] << 2) | (base_colour[2] >> 4);

            uv_colour[0][0] = (uv_colour[0][0] << 2) | (uv_colour[0][0] >> 4);
            uv_colour[0][1] = (uv_colour[0][1] << 1) | (uv_colour[0][1] >> 6);
            uv_colour[0][2] = (uv_colour[0][2] << 2) | (uv_colour[0][2] >> 4);

            uv_colour[1][0] = (uv_colour[1][0] << 2) | (uv_colour[1][0] >> 4);
            uv_colour[1][1] = (uv_colour[1][1] << 1) | (uv_colour[1][1] >> 6);
            uv_colour[1][2] = (uv_colour[1][2] << 2) | (uv_colour[1][2] >> 4);

            for (i = 0; i < 4; i++)
                for (j = 0; j < 4; j++)
                {
                    block_out[(i * 4 + j) * 4 + 0] = j * (uv_colour[0][0] - base_colour[0]) / 4.0f + i * (uv_colour[1][0] - base_colour[0]) / 4.0f + base_colour[0];
                    block_out[(i * 4 + j) * 4 + 1] = j * (uv_colour[0][1] - base_colour[1]) / 4.0f + i * (uv_colour[1][1] - base_colour[1]) / 4.0f + base_colour[1];
                    block_out[(i * 4 + j) * 4 + 2] = j * (uv_colour[0][2] - base_colour[2]) / 4.0f + i * (uv_colour[1][2] - base_colour[2]) / 4.0f + base_colour[2];
                    block_out[(i * 4 + j) * 4 + 3] = 255.0f;
                }
        }
        else // Differential mode
        {
            for (i = 0; i < 2; i++)
                for (j = 0; j < 3; j++)
                    base_colour_i32[i][j] = (base_colour_i32[i][j] << 3) | (base_colour_i32[i][j] >> 2);

            flip = ETC_ReadBits(block, 32, 1);
            table[1] = ETC_ReadBits(block, 34, 3);
            table[0] = ETC_ReadBits(block, 37, 3);

            for (i = 0; i < 2; i++)
                for (j = 0; j < 4; j++)
                {
                    colour[i][j][0] = Math_Clampi32((int)base_colour_i32[i][0] + g_table[table[i]][j], 0, 255);
                    colour[i][j][1] = Math_Clampi32((int)base_colour_i32[i][1] + g_table[table[i]][j], 0, 255);
                    colour[i][j][2] = Math_Clampi32((int)base_colour_i32[i][2] + g_table[table[i]][j], 0, 255);
                }

            for (i = 0; i < 2; i++)
                for (j = 0; j < 8; j++)
                {
                    int out_index = g_partition_indexes[flip][i][j];

                    block_out[out_index * 4 + 0] = (float)colour[i][index[out_index]][0];
                    block_out[out_index * 4 + 1] = (float)colour[i][index[out_index]][1];
                    block_out[out_index * 4 + 2] = (float)colour[i][index[out_index]][2];
                    block_out[out_index * 4 + 3] = 255.0f;
                }
        }
    }
}

void ETC_DecodeETC2RGBA1(float *block_out, uchar *block_in, int block_x, int block_y)
{
    int index[16];
    int opaque;
    uchar flip;
    uint table[2];
    int base_colour_i32[2][3];   // partition, rgb
    uint colour[2][4][3];       // partition, index, rgb
    int i;
    int j;
    uchar block[8];
    int delta[3];

    ETC_BlockSwapOrder(block, block_in);

    opaque = ETC_ReadBits(block, 33, 1);

    for (i = 0; i < 64; i++)
        block_out[i] = 0;

    for (i = 0; i < 4; i++)
        for (j = 0; j < 4; j++)
            index[j*4 + i] = ETC_ReadBits(block, i*4 + j, 1) | (ETC_ReadBits(block, 16 + i*4 + j, 1) << 1);

    base_colour_i32[0][0] = ETC_ReadBits(block, 59, 5);
    base_colour_i32[0][1] = ETC_ReadBits(block, 51, 5);
    base_colour_i32[0][2] = ETC_ReadBits(block, 43, 5);

    delta[0] = ETC_SignExtend(ETC_ReadBits(block, 56, 3), 3);
    delta[1] = ETC_SignExtend(ETC_ReadBits(block, 48, 3), 3);
    delta[2] = ETC_SignExtend(ETC_ReadBits(block, 40, 3), 3);

    base_colour_i32[1][0] = base_colour_i32[0][0] + delta[0];
    base_colour_i32[1][1] = base_colour_i32[0][1] + delta[1];
    base_colour_i32[1][2] = base_colour_i32[0][2] + delta[2];

    if (block_x == 324/4 && block_y == 180/4)
    {
        printf("mode %i %i %i\n", (base_colour_i32[1][0] < 0 || base_colour_i32[1][0] > 31), 
            (base_colour_i32[1][1] < 0 || base_colour_i32[1][1] > 31),
            (base_colour_i32[1][2] < 0 || base_colour_i32[1][2] > 31));
    }

    if (base_colour_i32[1][0] < 0 || base_colour_i32[1][0] > 31) // T mode
    {
        int dist_index;

        base_colour_i32[0][0] = ETC_ReadBits(block, 56, 2) | (ETC_ReadBits(block, 59, 2) << 2);
        base_colour_i32[0][1] = ETC_ReadBits(block, 52, 4);
        base_colour_i32[0][2] = ETC_ReadBits(block, 48, 4);

        base_colour_i32[1][0] = ETC_ReadBits(block, 44, 4);
        base_colour_i32[1][1] = ETC_ReadBits(block, 40, 4);
        base_colour_i32[1][2] = ETC_ReadBits(block, 36, 4);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_i32[i][j] = (base_colour_i32[i][j] << 4) | base_colour_i32[i][j];

        dist_index = (ETC_ReadBits(block, 34, 2) << 1) | (ETC_ReadBits(block, 32, 1));

        colour[0][0][0] = base_colour_i32[0][0];
        colour[0][0][1] = base_colour_i32[0][1];
        colour[0][0][2] = base_colour_i32[0][2];

        colour[0][1][0] = Math_Clampi32(base_colour_i32[1][0] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][1][1] = Math_Clampi32(base_colour_i32[1][1] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][1][2] = Math_Clampi32(base_colour_i32[1][2] + g_etc2_tmode_distance[dist_index], 0, 255);

        colour[0][2][0] = base_colour_i32[1][0];
        colour[0][2][1] = base_colour_i32[1][1];
        colour[0][2][2] = base_colour_i32[1][2];

        if (opaque == 0)
        {
            colour[0][2][0] = 0;
            colour[0][2][1] = 0;
            colour[0][2][2] = 0;
        }

        colour[0][3][0] = Math_Clampi32(base_colour_i32[1][0] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][3][1] = Math_Clampi32(base_colour_i32[1][1] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][3][2] = Math_Clampi32(base_colour_i32[1][2] - g_etc2_tmode_distance[dist_index], 0, 255);

        for (i = 0; i < 16; i++)
        {
            block_out[i * 4 + 0] = (float)colour[0][index[i]][0];
            block_out[i * 4 + 1] = (float)colour[0][index[i]][1];
            block_out[i * 4 + 2] = (float)colour[0][index[i]][2];
            if (opaque == 0 && index[i] == 2)
                block_out[i * 4 + 3] = 0.0f;
            else
                block_out[i * 4 + 3] = 255.0f;
        }
    }
    else if (base_colour_i32[1][1] < 0 || base_colour_i32[1][1] > 31) // H mode
    {
        int dist_index;
        uint32_t dist_bit1[2];

        base_colour_i32[0][0] = ETC_ReadBits(block, 59, 4);
        base_colour_i32[0][1] = ETC_ReadBits(block, 52, 1) | (ETC_ReadBits(block, 56, 3) << 1);
        base_colour_i32[0][2] = ETC_ReadBits(block, 47, 3) | (ETC_ReadBits(block, 51, 1) << 3);

        base_colour_i32[1][0] = ETC_ReadBits(block, 43, 4);
        base_colour_i32[1][1] = ETC_ReadBits(block, 39, 4);
        base_colour_i32[1][2] = ETC_ReadBits(block, 35, 4);

        dist_bit1[0] = (base_colour_i32[0][0] << 16) | (base_colour_i32[0][1] << 8) | base_colour_i32[0][2];
        dist_bit1[1] = (base_colour_i32[1][0] << 16) | (base_colour_i32[1][1] << 8) | base_colour_i32[1][2];

        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_i32[i][j] = (base_colour_i32[i][j] << 4) | base_colour_i32[i][j];

        dist_index = (ETC_ReadBits(block, 34, 1) << 2) | (ETC_ReadBits(block, 32, 1) << 1) | (dist_bit1[0] > dist_bit1[1] ? 1 : 0);

        colour[0][0][0] = Math_Clampi32(base_colour_i32[0][0] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][0][1] = Math_Clampi32(base_colour_i32[0][1] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][0][2] = Math_Clampi32(base_colour_i32[0][2] + g_etc2_tmode_distance[dist_index], 0, 255);

        colour[0][1][0] = Math_Clampi32(base_colour_i32[0][0] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][1][1] = Math_Clampi32(base_colour_i32[0][1] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][1][2] = Math_Clampi32(base_colour_i32[0][2] - g_etc2_tmode_distance[dist_index], 0, 255);

        colour[0][2][0] = Math_Clampi32(base_colour_i32[1][0] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][2][1] = Math_Clampi32(base_colour_i32[1][1] + g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][2][2] = Math_Clampi32(base_colour_i32[1][2] + g_etc2_tmode_distance[dist_index], 0, 255);

        if (opaque == 0)
        {
            colour[0][2][0] = 0;
            colour[0][2][1] = 0;
            colour[0][2][2] = 0;
        }

        colour[0][3][0] = Math_Clampi32(base_colour_i32[1][0] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][3][1] = Math_Clampi32(base_colour_i32[1][1] - g_etc2_tmode_distance[dist_index], 0, 255);
        colour[0][3][2] = Math_Clampi32(base_colour_i32[1][2] - g_etc2_tmode_distance[dist_index], 0, 255);

        for (i = 0; i < 16; i++)
        {
            block_out[i * 4 + 0] = (float)colour[0][index[i]][0];
            block_out[i * 4 + 1] = (float)colour[0][index[i]][1];
            block_out[i * 4 + 2] = (float)colour[0][index[i]][2];

            if (opaque == 0 && index[i] == 2)
                block_out[i * 4 + 3] = 0.0f;
            else
                block_out[i * 4 + 3] = 255.0f;
        }
    }
    else if (base_colour_i32[1][2] < 0 || base_colour_i32[1][2] > 31) // Planar mode
    {
        int base_colour[3];
        int uv_colour[2][3];

        base_colour[0] = ETC_ReadBits(block, 57, 6);
        base_colour[1] = ETC_ReadBits(block, 49, 6) | (ETC_ReadBits(block, 56, 1) << 6);
        base_colour[2] = ETC_ReadBits(block, 39, 3) | (ETC_ReadBits(block, 43, 2) << 3) | (ETC_ReadBits(block, 48, 1) << 5);

        uv_colour[0][0] = ETC_ReadBits(block, 32, 1) | (ETC_ReadBits(block, 34, 5) << 1); 
        uv_colour[0][1] = ETC_ReadBits(block, 25, 7); 
        uv_colour[0][2] = ETC_ReadBits(block, 19, 6); 

        uv_colour[1][0] = ETC_ReadBits(block, 13, 6); 
        uv_colour[1][1] = ETC_ReadBits(block, 6, 7); 
        uv_colour[1][2] = ETC_ReadBits(block, 0, 6); 

        base_colour[0] = (base_colour[0] << 2) | (base_colour[0] >> 4);
        base_colour[1] = (base_colour[1] << 1) | (base_colour[1] >> 6);
        base_colour[2] = (base_colour[2] << 2) | (base_colour[2] >> 4);

        uv_colour[0][0] = (uv_colour[0][0] << 2) | (uv_colour[0][0] >> 4);
        uv_colour[0][1] = (uv_colour[0][1] << 1) | (uv_colour[0][1] >> 6);
        uv_colour[0][2] = (uv_colour[0][2] << 2) | (uv_colour[0][2] >> 4);

        uv_colour[1][0] = (uv_colour[1][0] << 2) | (uv_colour[1][0] >> 4);
        uv_colour[1][1] = (uv_colour[1][1] << 1) | (uv_colour[1][1] >> 6);
        uv_colour[1][2] = (uv_colour[1][2] << 2) | (uv_colour[1][2] >> 4);

        for (i = 0; i < 4; i++)
            for (j = 0; j < 4; j++)
            {
                block_out[(i * 4 + j) * 4 + 0] = j * (uv_colour[0][0] - base_colour[0]) / 4.0f + i * (uv_colour[1][0] - base_colour[0]) / 4.0f + base_colour[0];
                block_out[(i * 4 + j) * 4 + 1] = j * (uv_colour[0][1] - base_colour[1]) / 4.0f + i * (uv_colour[1][1] - base_colour[1]) / 4.0f + base_colour[1];
                block_out[(i * 4 + j) * 4 + 2] = j * (uv_colour[0][2] - base_colour[2]) / 4.0f + i * (uv_colour[1][2] - base_colour[2]) / 4.0f + base_colour[2];
                block_out[(i * 4 + j) * 4 + 3] = 255.0f;
            }
    }
    else // Differential mode
    {
        for (i = 0; i < 2; i++)
            for (j = 0; j < 3; j++)
                base_colour_i32[i][j] = (base_colour_i32[i][j] << 3) | (base_colour_i32[i][j] >> 2);

        flip = ETC_ReadBits(block, 32, 1);
        table[1] = ETC_ReadBits(block, 34, 3);
        table[0] = ETC_ReadBits(block, 37, 3);

        for (i = 0; i < 2; i++)
            for (j = 0; j < 4; j++)
            {
                colour[i][j][0] = Math_Clampi32((int)base_colour_i32[i][0] + g_table[table[i]][j], 0, 255);
                colour[i][j][1] = Math_Clampi32((int)base_colour_i32[i][1] + g_table[table[i]][j], 0, 255);
                colour[i][j][2] = Math_Clampi32((int)base_colour_i32[i][2] + g_table[table[i]][j], 0, 255);
            }

        if (opaque == 0)
        {
            for (i = 0; i < 2; i++)
            {
                colour[i][0][0] = Math_Clampi32((int)base_colour_i32[i][0], 0, 255);
                colour[i][0][1] = Math_Clampi32((int)base_colour_i32[i][1], 0, 255);
                colour[i][0][2] = Math_Clampi32((int)base_colour_i32[i][2], 0, 255);
                colour[i][2][0] = 0;
                colour[i][2][1] = 0;
                colour[i][2][2] = 0;
            }
        }

        for (i = 0; i < 2; i++)
            for (j = 0; j < 8; j++)
            {
                int out_index = g_partition_indexes[flip][i][j];

                block_out[out_index * 4 + 0] = (float)colour[i][index[out_index]][0];
                block_out[out_index * 4 + 1] = (float)colour[i][index[out_index]][1];
                block_out[out_index * 4 + 2] = (float)colour[i][index[out_index]][2];
                if (opaque == 0 && index[out_index] == 2)
                    block_out[out_index * 4 + 3] = 0.0f;
                else
                    block_out[out_index * 4 + 3] = 255.0f;
            }
    }
}

void ETC_DecodeETC2RGBA8(float *block_out, uchar *block_in, int block_x, int block_y)
{
    int codeword;
    int multiplier;
    int table;
    uchar block[8];
    int index[16];
    int i;

    ETC_BlockSwapOrder(block, block_in);

    ETC_DecodeETC2RGB(block_out, &block_in[8], block_x, block_y);

    codeword = ETC_ReadBits(block, 56, 8); 
    multiplier = ETC_ReadBits(block, 52, 4); 
    table = ETC_ReadBits(block, 48, 4); 

    for (i = 0; i < 16; i++)
        index[i] = ETC_ReadBits(block, 45 - i*3, 3); 

    for (i = 0; i < 16; i++)
    {
        int u = i / 4;
        int v = i % 4;
        int out_index = u + v*4;

        block_out[out_index*4 + 3] = (float)Math_Clampi32(codeword + multiplier * g_alpha[table][index[i]], 0, 255);
    }
}

void ETC_DecodeETCN(image_t *image)
{
    int blocks_x = (image->src_width[0] + 3) >> 2;
    int blocks_y = (image->src_height[0] + 3) >> 2;
    int x;
    int y;
    int i;
    void (*decode_fp)(float *block_out, uint8_t *block_in, int block_x, int block_y) = 0;

    switch(image->dst_format)
    {
        case GL_ETC1_RGB8_OES:
            decode_fp = ETC_DecodeETC1;
            break;
        case GL_COMPRESSED_SRGB8_ETC2:
        case GL_COMPRESSED_RGB8_ETC2:
            decode_fp = ETC_DecodeETC2RGB;
            break;
        case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
        case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
            decode_fp = ETC_DecodeETC2RGBA1;
            break;
        case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
        case GL_COMPRESSED_RGBA8_ETC2_EAC:
            decode_fp = ETC_DecodeETC2RGBA8;
            break;
    }

    if (!decode_fp)
        return;

    for (y = 0; y < blocks_y; y++)
        for (x = 0; x < blocks_x; x++)
        {
            float block_rgba[4 * 4 * 4];
            uint8_t *block_in = &image->dst_encoded[0][(y * blocks_x + x) * image->dst_block_size];
            int stride = blocks_x * 4 * 4;
            int out_x;
            int out_y;
            int local_width = (x + 1) * 4 > image->src_width[0] ? image->src_width[0] - x * 4 : 4;
            int local_height = (y + 1) * 4 > image->src_height[0] ? image->src_height[0] - y * 4 : 4;

            decode_fp(block_rgba, block_in, x, y);

            for (out_y = 0; out_y < local_height; out_y++)
                for (out_x = 0; out_x < local_width; out_x++)
                    for (i = 0; i < 4; i++)
                        image->src_rgba_0_255[0][(y * 4 + out_y) * image->src_width[0] * 4 + (x * 4 + out_x) * 4 + i] = block_rgba[(out_y * 4 + out_x)*4 + i];
        }
}