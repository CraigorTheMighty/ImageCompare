#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <commctrl.h>
#include <math.h>
#include <psapi.h>
#include <Dbghelp.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <gl\wglext.h>

#include "..\inc\image.h"

//#define DEBUG_BLOCK (block_x == 143 && block_y == 34)
//#define DEBUG_BLOCK (block_x == 115 && block_y == 86)
//#define DEBUG_BLOCK (block_x == 147 && block_y == 163)
#define DEBUG_BLOCK (0)

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES	0x8D64
#endif
#ifndef GL_ETC1_SRGB8_NV
#define GL_ETC1_SRGB8_NV	0x88EE
#endif

#define M_PI	3.1415926535897932384626433832795

static uint8_t g_inferno_colourmap[1024][3] = 
{
	{  0,   0,   4}, {  0,   0,   4}, {  0,   0,   4}, {  1,   0,   4}, {  1,   0,   5}, {  1,   0,   5}, {  1,   0,   5}, {  1,   1,   6}, 
	{  1,   1,   6}, {  1,   1,   7}, {  1,   1,   7}, {  1,   1,   7}, {  1,   1,   8}, {  1,   1,   8}, {  1,   1,   9}, {  1,   1,   9}, 
	{  2,   1,  10}, {  2,   1,  10}, {  2,   1,  11}, {  2,   1,  11}, {  2,   2,  12}, {  2,   2,  12}, {  2,   2,  13}, {  2,   2,  14}, 
	{  2,   2,  14}, {  3,   2,  15}, {  3,   2,  15}, {  3,   2,  16}, {  3,   2,  16}, {  3,   3,  17}, {  3,   3,  17}, {  3,   3,  18}, 
	{  4,   3,  18}, {  4,   3,  19}, {  4,   3,  19}, {  4,   3,  20}, {  4,   3,  20}, {  4,   3,  21}, {  5,   4,  22}, {  5,   4,  22}, 
	{  5,   4,  23}, {  5,   4,  23}, {  5,   4,  24}, {  6,   4,  24}, {  6,   4,  25}, {  6,   5,  25}, {  6,   5,  26}, {  6,   5,  26}, 
	{  7,   5,  27}, {  7,   5,  27}, {  7,   5,  28}, {  7,   5,  29}, {  7,   5,  29}, {  8,   6,  30}, {  8,   6,  30}, {  8,   6,  31}, 
	{  8,   6,  31}, {  9,   6,  32}, {  9,   6,  32}, {  9,   6,  33}, { 10,   7,  34}, { 10,   7,  34}, { 10,   7,  35}, { 10,   7,  35}, 
	{ 11,   7,  36}, { 11,   7,  36}, { 11,   7,  37}, { 12,   8,  38}, { 12,   8,  38}, { 12,   8,  39}, { 13,   8,  39}, { 13,   8,  40}, 
	{ 13,   8,  40}, { 13,   8,  41}, { 14,   9,  42}, { 14,   9,  42}, { 14,   9,  43}, { 15,   9,  43}, { 15,   9,  44}, { 15,   9,  45}, 
	{ 16,   9,  45}, { 16,   9,  46}, { 16,  10,  46}, { 17,  10,  47}, { 17,  10,  48}, { 17,  10,  48}, { 17,  10,  49}, { 18,  10,  49}, 
	{ 18,  10,  50}, { 18,  10,  51}, { 19,  10,  51}, { 19,  11,  52}, { 19,  11,  52}, { 20,  11,  53}, { 20,  11,  54}, { 20,  11,  54}, 
	{ 21,  11,  55}, { 21,  11,  55}, { 21,  11,  56}, { 22,  11,  57}, { 22,  11,  57}, { 23,  11,  58}, { 23,  11,  58}, { 23,  12,  59}, 
	{ 24,  12,  60}, { 24,  12,  60}, { 24,  12,  61}, { 25,  12,  61}, { 25,  12,  62}, { 25,  12,  63}, { 26,  12,  63}, { 26,  12,  64}, 
	{ 27,  12,  64}, { 27,  12,  65}, { 27,  12,  66}, { 28,  12,  66}, { 28,  12,  67}, { 28,  12,  67}, { 29,  12,  68}, { 29,  12,  69}, 
	{ 30,  12,  69}, { 30,  12,  70}, { 30,  12,  70}, { 31,  12,  71}, { 31,  12,  72}, { 32,  12,  72}, { 32,  12,  73}, { 32,  12,  73}, 
	{ 33,  12,  74}, { 33,  12,  75}, { 34,  12,  75}, { 34,  12,  76}, { 34,  12,  76}, { 35,  12,  77}, { 35,  12,  77}, { 36,  12,  78}, 
	{ 36,  12,  78}, { 37,  12,  79}, { 37,  12,  80}, { 37,  12,  80}, { 38,  12,  81}, { 38,  12,  81}, { 39,  12,  82}, { 39,  11,  82}, 
	{ 40,  11,  83}, { 40,  11,  83}, { 40,  11,  84}, { 41,  11,  84}, { 41,  11,  85}, { 42,  11,  85}, { 42,  11,  86}, { 43,  11,  86}, 
	{ 43,  11,  87}, { 43,  11,  87}, { 44,  11,  88}, { 44,  11,  88}, { 45,  11,  89}, { 45,  11,  89}, { 46,  10,  89}, { 46,  10,  90}, 
	{ 47,  10,  90}, { 47,  10,  91}, { 47,  10,  91}, { 48,  10,  92}, { 48,  10,  92}, { 49,  10,  92}, { 49,  10,  93}, { 50,  10,  93}, 
	{ 50,  10,  94}, { 51,  10,  94}, { 51,  10,  94}, { 51,  10,  95}, { 52,  10,  95}, { 52,  10,  95}, { 53,  10,  96}, { 53,  10,  96}, 
	{ 54,   9,  96}, { 54,   9,  97}, { 54,   9,  97}, { 55,   9,  97}, { 55,   9,  98}, { 56,   9,  98}, { 56,   9,  98}, { 57,   9,  99}, 
	{ 57,   9,  99}, { 58,   9,  99}, { 58,   9,  99}, { 58,   9, 100}, { 59,   9, 100}, { 59,   9, 100}, { 60,   9, 100}, { 60,   9, 101}, 
	{ 61,   9, 101}, { 61,   9, 101}, { 61,   9, 101}, { 62,   9, 102}, { 62,   9, 102}, { 63,   9, 102}, { 63,  10, 102}, { 63,  10, 103}, 
	{ 64,  10, 103}, { 64,  10, 103}, { 65,  10, 103}, { 65,  10, 103}, { 66,  10, 104}, { 66,  10, 104}, { 66,  10, 104}, { 67,  10, 104}, 
	{ 67,  10, 104}, { 68,  10, 104}, { 68,  10, 105}, { 69,  10, 105}, { 69,  10, 105}, { 69,  10, 105}, { 70,  11, 105}, { 70,  11, 105}, 
	{ 71,  11, 106}, { 71,  11, 106}, { 71,  11, 106}, { 72,  11, 106}, { 72,  11, 106}, { 73,  11, 106}, { 73,  11, 106}, { 73,  11, 107}, 
	{ 74,  12, 107}, { 74,  12, 107}, { 75,  12, 107}, { 75,  12, 107}, { 76,  12, 107}, { 76,  12, 107}, { 76,  12, 107}, { 77,  12, 107}, 
	{ 77,  13, 108}, { 78,  13, 108}, { 78,  13, 108}, { 78,  13, 108}, { 79,  13, 108}, { 79,  13, 108}, { 80,  13, 108}, { 80,  13, 108}, 
	{ 80,  14, 108}, { 81,  14, 108}, { 81,  14, 109}, { 82,  14, 109}, { 82,  14, 109}, { 82,  14, 109}, { 83,  14, 109}, { 83,  15, 109}, 
	{ 84,  15, 109}, { 84,  15, 109}, { 84,  15, 109}, { 85,  15, 109}, { 85,  15, 109}, { 86,  15, 109}, { 86,  15, 109}, { 86,  16, 109}, 
	{ 87,  16, 109}, { 87,  16, 110}, { 88,  16, 110}, { 88,  16, 110}, { 88,  16, 110}, { 89,  17, 110}, { 89,  17, 110}, { 90,  17, 110}, 
	{ 90,  17, 110}, { 90,  17, 110}, { 91,  17, 110}, { 91,  17, 110}, { 92,  18, 110}, { 92,  18, 110}, { 92,  18, 110}, { 93,  18, 110}, 
	{ 93,  18, 110}, { 94,  18, 110}, { 94,  18, 110}, { 94,  19, 110}, { 95,  19, 110}, { 95,  19, 110}, { 96,  19, 110}, { 96,  19, 110}, 
	{ 96,  19, 110}, { 97,  19, 110}, { 97,  20, 110}, { 98,  20, 110}, { 98,  20, 110}, { 98,  20, 110}, { 99,  20, 110}, { 99,  20, 110}, 
	{ 99,  21, 110}, {100,  21, 110}, {100,  21, 110}, {101,  21, 110}, {101,  21, 110}, {101,  21, 110}, {102,  21, 110}, {102,  22, 110}, 
	{103,  22, 110}, {103,  22, 110}, {103,  22, 110}, {104,  22, 110}, {104,  22, 110}, {105,  22, 110}, {105,  23, 110}, {105,  23, 110}, 
	{106,  23, 110}, {106,  23, 110}, {107,  23, 110}, {107,  23, 110}, {107,  23, 110}, {108,  24, 110}, {108,  24, 110}, {109,  24, 110}, 
	{109,  24, 110}, {109,  24, 110}, {110,  24, 110}, {110,  24, 110}, {111,  25, 110}, {111,  25, 110}, {111,  25, 110}, {112,  25, 110}, 
	{112,  25, 110}, {113,  25, 110}, {113,  25, 110}, {113,  26, 110}, {114,  26, 110}, {114,  26, 110}, {115,  26, 110}, {115,  26, 110}, 
	{115,  26, 110}, {116,  26, 110}, {116,  27, 110}, {116,  27, 110}, {117,  27, 110}, {117,  27, 110}, {118,  27, 110}, {118,  27, 110}, 
	{118,  27, 109}, {119,  28, 109}, {119,  28, 109}, {120,  28, 109}, {120,  28, 109}, {120,  28, 109}, {121,  28, 109}, {121,  28, 109}, 
	{122,  29, 109}, {122,  29, 109}, {122,  29, 109}, {123,  29, 109}, {123,  29, 109}, {124,  29, 109}, {124,  29, 109}, {124,  30, 109}, 
	{125,  30, 109}, {125,  30, 109}, {126,  30, 108}, {126,  30, 108}, {126,  30, 108}, {127,  30, 108}, {127,  31, 108}, {128,  31, 108}, 
	{128,  31, 108}, {128,  31, 108}, {129,  31, 108}, {129,  31, 108}, {130,  31, 108}, {130,  32, 108}, {130,  32, 108}, {131,  32, 107}, 
	{131,  32, 107}, {132,  32, 107}, {132,  32, 107}, {132,  32, 107}, {133,  33, 107}, {133,  33, 107}, {134,  33, 107}, {134,  33, 107}, 
	{134,  33, 107}, {135,  33, 107}, {135,  33, 107}, {136,  33, 106}, {136,  34, 106}, {136,  34, 106}, {137,  34, 106}, {137,  34, 106}, 
	{137,  34, 106}, {138,  34, 106}, {138,  34, 106}, {139,  35, 106}, {139,  35, 106}, {139,  35, 105}, {140,  35, 105}, {140,  35, 105}, 
	{141,  35, 105}, {141,  35, 105}, {141,  36, 105}, {142,  36, 105}, {142,  36, 105}, {143,  36, 105}, {143,  36, 104}, {143,  36, 104}, 
	{144,  36, 104}, {144,  37, 104}, {145,  37, 104}, {145,  37, 104}, {145,  37, 104}, {146,  37, 104}, {146,  37, 104}, {147,  37, 103}, 
	{147,  38, 103}, {147,  38, 103}, {148,  38, 103}, {148,  38, 103}, {149,  38, 103}, {149,  38, 103}, {149,  38, 103}, {150,  39, 102}, 
	{150,  39, 102}, {151,  39, 102}, {151,  39, 102}, {151,  39, 102}, {152,  39, 102}, {152,  39, 102}, {153,  40, 101}, {153,  40, 101}, 
	{153,  40, 101}, {154,  40, 101}, {154,  40, 101}, {154,  40, 101}, {155,  40, 101}, {155,  41, 100}, {156,  41, 100}, {156,  41, 100}, 
	{156,  41, 100}, {157,  41, 100}, {157,  41, 100}, {158,  42,  99}, {158,  42,  99}, {158,  42,  99}, {159,  42,  99}, {159,  42,  99}, 
	{160,  42,  99}, {160,  42,  99}, {160,  43,  98}, {161,  43,  98}, {161,  43,  98}, {162,  43,  98}, {162,  43,  98}, {162,  43,  98}, 
	{163,  44,  97}, {163,  44,  97}, {164,  44,  97}, {164,  44,  97}, {164,  44,  97}, {165,  44,  97}, {165,  44,  96}, {165,  45,  96}, 
	{166,  45,  96}, {166,  45,  96}, {167,  45,  96}, {167,  45,  95}, {167,  45,  95}, {168,  46,  95}, {168,  46,  95}, {169,  46,  95}, 
	{169,  46,  95}, {169,  46,  94}, {170,  46,  94}, {170,  47,  94}, {170,  47,  94}, {171,  47,  94}, {171,  47,  93}, {172,  47,  93}, 
	{172,  47,  93}, {172,  48,  93}, {173,  48,  93}, {173,  48,  92}, {174,  48,  92}, {174,  48,  92}, {174,  48,  92}, {175,  49,  92}, 
	{175,  49,  91}, {175,  49,  91}, {176,  49,  91}, {176,  49,  91}, {177,  49,  91}, {177,  50,  90}, {177,  50,  90}, {178,  50,  90}, 
	{178,  50,  90}, {178,  50,  90}, {179,  51,  89}, {179,  51,  89}, {180,  51,  89}, {180,  51,  89}, {180,  51,  89}, {181,  51,  88}, 
	{181,  52,  88}, {181,  52,  88}, {182,  52,  88}, {182,  52,  87}, {183,  52,  87}, {183,  53,  87}, {183,  53,  87}, {184,  53,  87}, 
	{184,  53,  86}, {184,  53,  86}, {185,  54,  86}, {185,  54,  86}, {186,  54,  85}, {186,  54,  85}, {186,  54,  85}, {187,  55,  85}, 
	{187,  55,  85}, {187,  55,  84}, {188,  55,  84}, {188,  55,  84}, {189,  56,  84}, {189,  56,  83}, {189,  56,  83}, {190,  56,  83}, 
	{190,  56,  83}, {190,  57,  82}, {191,  57,  82}, {191,  57,  82}, {191,  57,  82}, {192,  57,  81}, {192,  58,  81}, {192,  58,  81}, 
	{193,  58,  81}, {193,  58,  81}, {194,  59,  80}, {194,  59,  80}, {194,  59,  80}, {195,  59,  80}, {195,  59,  79}, {195,  60,  79}, 
	{196,  60,  79}, {196,  60,  79}, {196,  60,  78}, {197,  61,  78}, {197,  61,  78}, {197,  61,  78}, {198,  61,  77}, {198,  61,  77}, 
	{198,  62,  77}, {199,  62,  77}, {199,  62,  76}, {200,  62,  76}, {200,  63,  76}, {200,  63,  76}, {201,  63,  75}, {201,  63,  75}, 
	{201,  64,  75}, {202,  64,  74}, {202,  64,  74}, {202,  64,  74}, {203,  65,  74}, {203,  65,  73}, {203,  65,  73}, {204,  65,  73}, 
	{204,  66,  73}, {204,  66,  72}, {205,  66,  72}, {205,  66,  72}, {205,  67,  72}, {206,  67,  71}, {206,  67,  71}, {206,  67,  71}, 
	{207,  68,  70}, {207,  68,  70}, {207,  68,  70}, {208,  68,  70}, {208,  69,  69}, {208,  69,  69}, {209,  69,  69}, {209,  69,  69}, 
	{209,  70,  68}, {209,  70,  68}, {210,  70,  68}, {210,  71,  67}, {210,  71,  67}, {211,  71,  67}, {211,  71,  67}, {211,  72,  66}, 
	{212,  72,  66}, {212,  72,  66}, {212,  72,  66}, {213,  73,  65}, {213,  73,  65}, {213,  73,  65}, {214,  74,  64}, {214,  74,  64}, 
	{214,  74,  64}, {214,  74,  64}, {215,  75,  63}, {215,  75,  63}, {215,  75,  63}, {216,  76,  62}, {216,  76,  62}, {216,  76,  62}, 
	{217,  77,  62}, {217,  77,  61}, {217,  77,  61}, {217,  77,  61}, {218,  78,  60}, {218,  78,  60}, {218,  78,  60}, {219,  79,  60}, 
	{219,  79,  59}, {219,  79,  59}, {219,  80,  59}, {220,  80,  58}, {220,  80,  58}, {220,  81,  58}, {221,  81,  58}, {221,  81,  57}, 
	{221,  81,  57}, {221,  82,  57}, {222,  82,  56}, {222,  82,  56}, {222,  83,  56}, {223,  83,  56}, {223,  83,  55}, {223,  84,  55}, 
	{223,  84,  55}, {224,  84,  54}, {224,  85,  54}, {224,  85,  54}, {224,  85,  53}, {225,  86,  53}, {225,  86,  53}, {225,  86,  53}, 
	{226,  87,  52}, {226,  87,  52}, {226,  87,  52}, {226,  88,  51}, {227,  88,  51}, {227,  88,  51}, {227,  89,  51}, {227,  89,  50}, 
	{228,  89,  50}, {228,  90,  50}, {228,  90,  49}, {228,  91,  49}, {229,  91,  49}, {229,  91,  48}, {229,  92,  48}, {229,  92,  48}, 
	{230,  92,  48}, {230,  93,  47}, {230,  93,  47}, {230,  93,  47}, {231,  94,  46}, {231,  94,  46}, {231,  94,  46}, {231,  95,  45}, 
	{231,  95,  45}, {232,  96,  45}, {232,  96,  44}, {232,  96,  44}, {232,  97,  44}, {233,  97,  44}, {233,  97,  43}, {233,  98,  43}, 
	{233,  98,  43}, {233,  98,  42}, {234,  99,  42}, {234,  99,  42}, {234, 100,  41}, {234, 100,  41}, {235, 100,  41}, {235, 101,  41}, 
	{235, 101,  40}, {235, 101,  40}, {235, 102,  40}, {236, 102,  39}, {236, 103,  39}, {236, 103,  39}, {236, 103,  38}, {236, 104,  38}, 
	{237, 104,  38}, {237, 105,  37}, {237, 105,  37}, {237, 105,  37}, {237, 106,  36}, {238, 106,  36}, {238, 106,  36}, {238, 107,  35}, 
	{238, 107,  35}, {238, 108,  35}, {239, 108,  35}, {239, 108,  34}, {239, 109,  34}, {239, 109,  34}, {239, 110,  33}, {240, 110,  33}, 
	{240, 110,  33}, {240, 111,  32}, {240, 111,  32}, {240, 112,  32}, {240, 112,  31}, {241, 112,  31}, {241, 113,  31}, {241, 113,  30}, 
	{241, 114,  30}, {241, 114,  30}, {241, 115,  29}, {242, 115,  29}, {242, 115,  29}, {242, 116,  28}, {242, 116,  28}, {242, 117,  28}, 
	{242, 117,  27}, {243, 117,  27}, {243, 118,  27}, {243, 118,  26}, {243, 119,  26}, {243, 119,  26}, {243, 119,  26}, {244, 120,  25}, 
	{244, 120,  25}, {244, 121,  25}, {244, 121,  24}, {244, 122,  24}, {244, 122,  24}, {244, 122,  23}, {245, 123,  23}, {245, 123,  23}, 
	{245, 124,  22}, {245, 124,  22}, {245, 125,  22}, {245, 125,  21}, {245, 125,  21}, {246, 126,  21}, {246, 126,  20}, {246, 127,  20}, 
	{246, 127,  20}, {246, 128,  19}, {246, 128,  19}, {246, 128,  19}, {246, 129,  18}, {247, 129,  18}, {247, 130,  18}, {247, 130,  17}, 
	{247, 131,  17}, {247, 131,  17}, {247, 131,  16}, {247, 132,  16}, {247, 132,  16}, {247, 133,  15}, {248, 133,  15}, {248, 134,  15}, 
	{248, 134,  14}, {248, 135,  14}, {248, 135,  14}, {248, 135,  13}, {248, 136,  13}, {248, 136,  13}, {248, 137,  12}, {248, 137,  12}, 
	{249, 138,  12}, {249, 138,  12}, {249, 139,  11}, {249, 139,  11}, {249, 139,  11}, {249, 140,  10}, {249, 140,  10}, {249, 141,  10}, 
	{249, 141,  10}, {249, 142,   9}, {249, 142,   9}, {250, 143,   9}, {250, 143,   9}, {250, 144,   8}, {250, 144,   8}, {250, 144,   8}, 
	{250, 145,   8}, {250, 145,   8}, {250, 146,   7}, {250, 146,   7}, {250, 147,   7}, {250, 147,   7}, {250, 148,   7}, {250, 148,   7}, 
	{250, 149,   7}, {251, 149,   6}, {251, 149,   6}, {251, 150,   6}, {251, 150,   6}, {251, 151,   6}, {251, 151,   6}, {251, 152,   6}, 
	{251, 152,   6}, {251, 153,   6}, {251, 153,   6}, {251, 154,   6}, {251, 154,   6}, {251, 155,   6}, {251, 155,   6}, {251, 155,   6}, 
	{251, 156,   6}, {251, 156,   6}, {251, 157,   6}, {251, 157,   7}, {251, 158,   7}, {252, 158,   7}, {252, 159,   7}, {252, 159,   7}, 
	{252, 160,   7}, {252, 160,   8}, {252, 161,   8}, {252, 161,   8}, {252, 162,   8}, {252, 162,   9}, {252, 162,   9}, {252, 163,   9}, 
	{252, 163,   9}, {252, 164,  10}, {252, 164,  10}, {252, 165,  10}, {252, 165,  11}, {252, 166,  11}, {252, 166,  11}, {252, 167,  12}, 
	{252, 167,  12}, {252, 168,  13}, {252, 168,  13}, {252, 169,  13}, {252, 169,  14}, {252, 170,  14}, {252, 170,  15}, {252, 171,  15}, 
	{252, 171,  16}, {252, 172,  16}, {252, 172,  16}, {252, 172,  17}, {252, 173,  17}, {252, 173,  18}, {252, 174,  18}, {252, 174,  19}, 
	{252, 175,  19}, {252, 175,  20}, {252, 176,  20}, {252, 176,  21}, {252, 177,  21}, {252, 177,  22}, {252, 178,  22}, {252, 178,  23}, 
	{252, 179,  23}, {252, 179,  24}, {252, 180,  24}, {252, 180,  25}, {252, 181,  25}, {252, 181,  26}, {251, 182,  26}, {251, 182,  27}, 
	{251, 183,  27}, {251, 183,  28}, {251, 184,  28}, {251, 184,  29}, {251, 185,  29}, {251, 185,  30}, {251, 185,  31}, {251, 186,  31}, 
	{251, 186,  32}, {251, 187,  32}, {251, 187,  33}, {251, 188,  33}, {251, 188,  34}, {251, 189,  34}, {251, 189,  35}, {251, 190,  36}, 
	{251, 190,  36}, {251, 191,  37}, {251, 191,  37}, {250, 192,  38}, {250, 192,  38}, {250, 193,  39}, {250, 193,  40}, {250, 194,  40}, 
	{250, 194,  41}, {250, 195,  41}, {250, 195,  42}, {250, 196,  43}, {250, 196,  43}, {250, 197,  44}, {250, 197,  45}, {250, 198,  45}, 
	{249, 198,  46}, {249, 199,  46}, {249, 199,  47}, {249, 200,  48}, {249, 200,  48}, {249, 201,  49}, {249, 201,  50}, {249, 202,  50}, 
	{249, 202,  51}, {249, 203,  52}, {249, 203,  52}, {248, 204,  53}, {248, 204,  54}, {248, 205,  54}, {248, 205,  55}, {248, 206,  56}, 
	{248, 206,  56}, {248, 207,  57}, {248, 207,  58}, {248, 208,  58}, {248, 208,  59}, {247, 208,  60}, {247, 209,  60}, {247, 209,  61}, 
	{247, 210,  62}, {247, 210,  63}, {247, 211,  63}, {247, 211,  64}, {247, 212,  65}, {247, 212,  65}, {246, 213,  66}, {246, 213,  67}, 
	{246, 214,  68}, {246, 214,  68}, {246, 215,  69}, {246, 215,  70}, {246, 216,  71}, {246, 216,  71}, {246, 217,  72}, {245, 217,  73}, 
	{245, 218,  74}, {245, 218,  75}, {245, 219,  75}, {245, 219,  76}, {245, 220,  77}, {245, 220,  78}, {245, 221,  79}, {244, 221,  79}, 
	{244, 222,  80}, {244, 222,  81}, {244, 223,  82}, {244, 223,  83}, {244, 223,  84}, {244, 224,  85}, {244, 224,  85}, {243, 225,  86}, 
	{243, 225,  87}, {243, 226,  88}, {243, 226,  89}, {243, 227,  90}, {243, 227,  91}, {243, 228,  92}, {243, 228,  93}, {243, 229,  93}, 
	{243, 229,  94}, {242, 230,  95}, {242, 230,  96}, {242, 230,  97}, {242, 231,  98}, {242, 231,  99}, {242, 232, 100}, {242, 232, 101}, 
	{242, 233, 102}, {242, 233, 103}, {242, 234, 104}, {242, 234, 105}, {242, 234, 106}, {242, 235, 107}, {241, 235, 108}, {241, 236, 109}, 
	{241, 236, 110}, {241, 237, 111}, {241, 237, 112}, {241, 237, 113}, {241, 238, 114}, {241, 238, 115}, {241, 239, 116}, {241, 239, 117}, 
	{241, 239, 118}, {241, 240, 119}, {241, 240, 120}, {241, 241, 121}, {242, 241, 122}, {242, 241, 123}, {242, 242, 124}, {242, 242, 125}, 
	{242, 242, 126}, {242, 243, 128}, {242, 243, 129}, {242, 244, 130}, {242, 244, 131}, {242, 244, 132}, {243, 245, 133}, {243, 245, 134}, 
	{243, 245, 135}, {243, 246, 136}, {243, 246, 137}, {243, 246, 138}, {244, 247, 139}, {244, 247, 140}, {244, 247, 141}, {244, 248, 142}, 
	{245, 248, 143}, {245, 248, 144}, {245, 249, 145}, {245, 249, 146}, {246, 249, 147}, {246, 249, 148}, {246, 250, 149}, {246, 250, 150}, 
	{247, 250, 151}, {247, 251, 152}, {247, 251, 153}, {248, 251, 154}, {248, 252, 155}, {248, 252, 155}, {249, 252, 156}, {249, 252, 157}, 
	{249, 253, 158}, {250, 253, 159}, {250, 253, 160}, {250, 254, 161}, {251, 254, 162}, {251, 254, 163}, {252, 254, 164}, {252, 255, 164}
};

int Image_BlockSize(int gl_format)
{
	switch (gl_format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
		case GL_COMPRESSED_RED_RGTC1:           // BC4
			return 8;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			return 16;
		case GL_ETC1_RGB8_OES:
			return 8;
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_ETC2:
			return 8;
			break;
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			return 8;
			break;
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
			return 16;
			break;
		default:
			return 0;
	}
}
image_t Image_Create(int width, int height, int mipmaps, int encoded_format)
{
	image_t image;
	int i;
	int w = width;
	int h = height;
	int num_mipmap_levels = 1;

	if (mipmaps)
		while ((w > 1) || (h > 1))
		{
			w >>= w > 1 ? 1 : 0;
			h >>= h > 1 ? 1 : 0;
			num_mipmap_levels++;
		}

	image.dst_format = encoded_format;
	image.dst_block_size = Image_BlockSize(encoded_format);
	image.num_mipmaps = num_mipmap_levels;
	image.src_rgba_0_255 = malloc(sizeof(float*) * image.num_mipmaps);
	image.dst_encoded = malloc(sizeof(uint8_t*) * image.num_mipmaps);
	image.src_width = malloc(sizeof(int) * image.num_mipmaps);
	image.src_height = malloc(sizeof(int) * image.num_mipmaps);
	
	switch(image.dst_format)
	{
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
			image.num_channels = 4;
			break;
		case GL_COMPRESSED_RED_RGTC1:           // BC4
			image.num_channels = 1;
			break;
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			image.num_channels = 2;
			break;
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
		case GL_ETC1_RGB8_OES:
			image.num_channels = 3;
			break;
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_ETC2:
			image.num_channels = 3;
			break;
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
			image.num_channels = 4;
			break;
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
			image.num_channels = 4;
			break;
		default:
			image.num_channels = 4;
			break;
	}

	for (i = 0; i < image.num_mipmaps; i++)
	{
		image.src_width[i] = width >> i;
		image.src_height[i] = height >> i;

		image.src_width[i] = image.src_width[i] == 0 ? 1 : image.src_width[i];
		image.src_height[i] = image.src_height[i] == 0 ? 1 : image.src_height[i];

		image.src_rgba_0_255[i] = malloc(image.src_width[i] * image.src_height[i] * sizeof(float) * 4);

		image.dst_encoded[i] = malloc(((image.src_width[i] + 3) >> 2) * ((image.src_height[i] + 3) >> 2) * Image_BlockSize(encoded_format));
	}
	return image;
}
void Image_Destroy(image_t *image)
{
	int i;

	for (i = 0; i < image->num_mipmaps; i++)
	{
		free(image->src_rgba_0_255[i]);
		free(image->dst_encoded[i]);
	}

	free(image->src_width);
	free(image->src_height);
	free(image->src_rgba_0_255);
	free(image->dst_encoded);

	memset(image, 0, sizeof(image_t));
}

void Image_ExtractSubImageRGBAf32(float *dst, image_t *image, int mip_level, int width, int height, int x, int y)
{
	int yl;
	int local_width = width;
	int local_height = height;

	if (x + width > image->src_width[mip_level])
		local_width = image->src_width[mip_level] - x;
	if (y + height > image->src_height[mip_level])
		local_height = image->src_height[mip_level] - y;

	for (yl = 0; yl < width * height * 4; yl++)
		dst[yl] = 0.0f;

	for (yl = 0; yl < local_height; yl++)
	{
		int y_offset = (y + yl) * image->src_width[mip_level] * 4;
		int x_offset = x * 4;
		int src_offset = y_offset + x_offset;
		int dst_offset = (int)(yl * width * 4);
		int bytes = 4 * sizeof(float) * local_width;
		memcpy(&dst[dst_offset], &(image->src_rgba_0_255[mip_level][src_offset]), bytes);
	}
}

void Image_CopyEncodedToImage(image_t *image, uint8_t *src, int mip_level, int width, int height, int x, int y)
{
	int yl;
	int blocks_x = (image->src_width[mip_level] + 3) >> 2;
	int block_extents_x = (width + 3) >> 2;
	int block_extents_y = (height + 3) >> 2;
	int block_dst_x = (x + 3) >> 2;
	int block_dst_y = (y + 3) >> 2;

	for (yl = 0; yl < block_extents_y; yl++)
	{
		int src_offset = yl * block_extents_x * image->dst_block_size;
		int y_offset = (yl + block_dst_y) * blocks_x;
		int x_offset = block_dst_x;
		int dst_offset = (y_offset + x_offset) * image->dst_block_size;
		int bytes = block_extents_x * image->dst_block_size;
		memcpy(&image->dst_encoded[mip_level][dst_offset], &src[src_offset], bytes);
	}
}

void Image_PSNR(double *psnr_rgb, double *psnr_alpha, image_t *ref_image, image_t *compare_image)
{
	int x;
	int y;
	double error[2] = {0.0, 0.0};
	int pixel_count = 0;
	int num_channels = ref_image->num_channels < compare_image->num_channels ? ref_image->num_channels : compare_image->num_channels;

	for (y = 0; y < ref_image->src_height[0]; y++)
		for (x = 0; x < ref_image->src_width[0]; x++)
		{
			int base_offset = y * ref_image->src_width[0] * 4 + x * 4;
			int is_transparent = 0;

			if (num_channels > 3)
			{
				is_transparent = compare_image->src_rgba_0_255[0][base_offset + 3] == 0.0f ? 1 : 0;
			}
			if (!is_transparent)
			{
				pixel_count++;
				error[0] += (ref_image->src_rgba_0_255[0][base_offset + 0] - compare_image->src_rgba_0_255[0][base_offset + 0]) * (ref_image->src_rgba_0_255[0][base_offset + 0] - compare_image->src_rgba_0_255[0][base_offset + 0]);
				if (num_channels > 1)
					error[0] += (ref_image->src_rgba_0_255[0][base_offset + 1] - compare_image->src_rgba_0_255[0][base_offset + 1]) * (ref_image->src_rgba_0_255[0][base_offset + 1] - compare_image->src_rgba_0_255[0][base_offset + 1]);
				if (num_channels > 2)
					error[0] += (ref_image->src_rgba_0_255[0][base_offset + 2] - compare_image->src_rgba_0_255[0][base_offset + 2]) * (ref_image->src_rgba_0_255[0][base_offset + 2] - compare_image->src_rgba_0_255[0][base_offset + 2]);
				if (num_channels > 3)
					error[1] += (ref_image->src_rgba_0_255[0][base_offset + 3] - compare_image->src_rgba_0_255[0][base_offset + 3]) * (ref_image->src_rgba_0_255[0][base_offset + 3] - compare_image->src_rgba_0_255[0][base_offset + 3]);
			}
		}
	pixel_count = pixel_count == 0 ? 1 : pixel_count;

	*psnr_rgb = 10.0 * log10(255.0f * 255.0f / (error[0] / pixel_count));
	*psnr_alpha = 10.0 * log10(255.0f * 255.0f / (error[1] / pixel_count));
}

double *Image_SSIM_Init(int width)
{
	int i;
	double std = 1.5;
	double *weights = malloc(sizeof(double) * (2*width + 1));
	for (i = 0; i < 2*width + 1; i++)
	{
		weights[i] = (1.0/(std*sqrt(2*M_PI)))*exp(-(1.0/2.0)*pow((i - width)/std, 2.0));
	}
	return weights;
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

static double Image_RGBA8ToLuminance(float *rgba, int num_channels)
{
	if (num_channels == 1)
		return rgba[0];
	else if (num_channels == 2)
		return (rgba[0] + rgba[1]) / 2.0;
	else
		return 0.2126 * rgba[0] + 0.7152 * rgba[1] + 0.0722 * rgba[2];
}

static void Image_SSIM_Region(image_t *ref_image, image_t *compare_image, int xorg, int yorg, int window_size, double *window_weights, float *lx, float *cx, float *sx)
{
	double k1 = 0.01;
	double k2 = 0.03;
	double L = 255.0;
	double c1 = (k1*L)*(k1*L);
	double c2 = (k2*L)*(k2*L);
	double c3 = c2 / 2.0;
	double window_luminance[2];
	double window_mean[2];
	double variance[2];
	double covariance = 0.0;
	int width = ref_image->src_width[0];
	int height = ref_image->src_height[0];
	int i;
	int x;
	int y;
	int xmins = Math_Clampi32(xorg - window_size, 0, ref_image->src_width[0] - 1);
	int xmaxs = Math_Clampi32(xorg + window_size, 0, ref_image->src_width[0] - 1);
	int ymins = Math_Clampi32(yorg - window_size, 0, ref_image->src_height[0] - 1);
	int ymaxs = Math_Clampi32(yorg + window_size, 0, ref_image->src_height[0] - 1);
	double total_pixels = (xmaxs - xmins + 1) * (ymaxs - ymins + 1);
	float *rgba[2] = {ref_image->src_rgba_0_255[0], compare_image->src_rgba_0_255[0]};
	double std = 1.5;
	double weights_total = 0.0f;
	int x_offset = xmins - (xorg - window_size);
	int y_offset = ymins - (yorg - window_size);

	for (y = ymins; y <= ymaxs; y++)
		for (x = xmins; x <= xmaxs; x++)
		{
			weights_total += window_weights[x - xmins + x_offset] * window_weights[y - ymins + y_offset];
		}
	weights_total = weights_total ? weights_total : 1.0f;
	weights_total = 1.0 / weights_total;

	for (i = 0; i < 2; i++)
	{
		window_luminance[i] = 0.0;
		window_mean[i] = 0.0;
		variance[i] = 0.0;
			
		for (y = ymins; y <= ymaxs; y++)
			for (x = xmins; x <= xmaxs; x++)
			{
				if ((ref_image->num_channels == 4) && (compare_image->num_channels == 4) && ((rgba[0][(y * width + x) * 4 + 3] == 0.0f) || (rgba[1][(y * width + x) * 4 + 3] == 0.0f)))
					continue;
				window_mean[i] += Image_RGBA8ToLuminance(&rgba[i][(y * width + x) * 4], compare_image->num_channels) * weights_total * window_weights[x - xmins + x_offset] * weights_total * window_weights[y - ymins + y_offset];
			}

		window_mean[i] /= (double)total_pixels;

		for (y = ymins; y <= ymaxs; y++)
			for (x = xmins; x <= xmaxs; x++)
			{
				if ((ref_image->num_channels == 4) && (compare_image->num_channels == 4) && ((rgba[0][(y * width + x) * 4 + 3] == 0.0f) || (rgba[1][(y * width + x) * 4 + 3] == 0.0f)))
					continue;
				variance[i] += pow(Image_RGBA8ToLuminance(&rgba[i][(y * width + x) * 4], compare_image->num_channels) - window_mean[i], 2.0) * weights_total * window_weights[x - xmins + x_offset] * weights_total * window_weights[y - ymins + y_offset];
			}

		variance[i] = sqrt(variance[i]);
	}

	for (y = ymins; y <= ymaxs; y++)
		for (x = xmins; x <= xmaxs; x++)
		{
			if ((ref_image->num_channels == 4) && (compare_image->num_channels == 4) && ((rgba[0][(y * width + x) * 4 + 3] == 0.0f) || (rgba[1][(y * width + x) * 4 + 3] == 0.0f)))
				continue;
			covariance += (Image_RGBA8ToLuminance(&rgba[0][(y * width + x) * 4], compare_image->num_channels) - window_mean[0]) * (Image_RGBA8ToLuminance(&rgba[1][(y * width + x) * 4], compare_image->num_channels) - window_mean[1]) * weights_total * window_weights[x - xmins + x_offset] * weights_total * window_weights[y - ymins + y_offset];
		}

	*lx = (float)((2.0 * window_mean[0] * window_mean[1] + c1) / (pow(window_mean[0], 2.0) + pow(window_mean[1], 2.0) + c1));
	*cx = (float)((2.0 * variance[0] * variance[1] + c2) / (pow(variance[0], 2.0) + pow(variance[1], 2.0) + c2));
	*sx = (float)((covariance + c3) / (variance[0] * variance[1] + c3));
}
int __cdecl Image_FloatCmp(void const *arg0, void const *arg1)
{
	float const *arg0f = arg0;
	float const *arg1f = arg1;

	if (*arg0f < *arg1f)
		return -1;
	else if (*arg0f > *arg1f)
		return 1;
	else
		return 0;
}
void Image_SSIM(int is_literal, int window, image_t *ssim_lx, image_t *ssim_cx, image_t *ssim_sx, image_t *ssim, image_t *ref_image, image_t *compare_image, double *lx_range, double *cx_range, double *sx_range, double *ssim_range, double view_range_min, double view_range_max)
{
	double *weights;
	int x;
	int y;
	float *ssim_ptr[4];
	double range[4][2];
	float *temp_data[4];
	int view_offset[2];
	double temp[2];

	window = min(max(window, 0), 16);

	weights = Image_SSIM_Init(window);

	temp[0] = min(max(view_range_max, 0.0), 1.0);
	temp[1] = min(max(view_range_min, 0.0), 1.0);

	view_range_min = 1.0 - temp[0];
	view_range_max = 1.0 - temp[1];

	view_offset[0] = (int)(view_range_min * ref_image->src_width[0] * ref_image->src_height[0]);
	view_offset[1] = (int)(view_range_max * ref_image->src_width[0] * ref_image->src_height[0]);

	if (view_offset[0] < 0)
		view_offset[0] = 0;
	if (view_offset[1] < 0)
		view_offset[1] = 0;
	if (view_offset[0] >= ref_image->src_width[0] * ref_image->src_height[0])
		view_offset[0] = ref_image->src_width[0] * ref_image->src_height[0] - 1;
	if (view_offset[1] >= ref_image->src_width[0] * ref_image->src_height[0])
		view_offset[1] = ref_image->src_width[0] * ref_image->src_height[0] - 1;
	if (view_offset[0] > view_offset[1])
	{
		int temp = view_offset[0];
		view_offset[0] = view_offset[1];
		view_offset[1] = temp;
	}

	*ssim_lx = Image_Create(ref_image->src_width[0], ref_image->src_height[0], 0, 0);
	*ssim_cx = Image_Create(ref_image->src_width[0], ref_image->src_height[0], 0, 0);
	*ssim_sx = Image_Create(ref_image->src_width[0], ref_image->src_height[0], 0, 0);
	*ssim = Image_Create(ref_image->src_width[0], ref_image->src_height[0], 0, 0);

	ssim_ptr[0] = ssim_lx->src_rgba_0_255[0];
	ssim_ptr[1] = ssim_cx->src_rgba_0_255[0];
	ssim_ptr[2] = ssim_sx->src_rgba_0_255[0];
	ssim_ptr[3] = ssim->src_rgba_0_255[0];

#pragma omp parallel for private(y) private(x)
	for (y = 0; y < ref_image->src_height[0]; y++)
		for (x = 0; x < ref_image->src_width[0]; x++)
		{
			int offset = (y * ref_image->src_width[0] * 4 + x * 4);
			Image_SSIM_Region(ref_image, compare_image, x, y, window, weights, &ssim_lx->src_rgba_0_255[0][offset], &ssim_cx->src_rgba_0_255[0][offset], &ssim_sx->src_rgba_0_255[0][offset]);
			ssim->src_rgba_0_255[0][offset] = ssim_lx->src_rgba_0_255[0][offset] * ssim_cx->src_rgba_0_255[0][offset] * ssim_sx->src_rgba_0_255[0][offset];
		}
	free(weights);

	for (y = 0; y < 4; y++)
	{
		range[y][0] = ssim_ptr[y][0];
		range[y][1] = ssim_ptr[y][1];
	}

	for (y = 0; y < ref_image->src_width[0] * ref_image->src_height[0]; y++)
	{
		for (x = 0; x < 4; x++)
		{
			if (ssim_ptr[x][y * 4] < range[x][0])
				range[x][0] = ssim_ptr[x][y * 4];
			if (ssim_ptr[x][y * 4] > range[x][1])
				range[x][1] = ssim_ptr[x][y * 4];
		}
	}

	for (y = 0; y < 4; y++)
		temp_data[y] = malloc(ref_image->src_width[0] * ref_image->src_height[0] * sizeof(float));
	for (y = 0; y < ref_image->src_width[0] * ref_image->src_height[0]; y++)
	{
		temp_data[0][y] = ssim_lx->src_rgba_0_255[0][y*4];
		temp_data[1][y] = ssim_cx->src_rgba_0_255[0][y*4];
		temp_data[2][y] = ssim_sx->src_rgba_0_255[0][y*4];
		temp_data[3][y] = ssim->src_rgba_0_255[0][y*4];
	}
	for (y = 0; y < 4; y++)
		qsort(temp_data[y], ref_image->src_width[0] * ref_image->src_height[0], sizeof(float), Image_FloatCmp);
	
	// percentile range
	if (!is_literal)
	{
		for (x = 0; x < 4; x++)
		{
			range[x][0] = temp_data[x][view_offset[0]];
			range[x][1] = temp_data[x][view_offset[1]];
		}
	}
	else
	{
		for (x = 0; x < 4; x++)
		{
			range[x][0] = 1.0 - view_range_max;
			range[x][1] = 1.0 - view_range_min;
		}
	}

	for (y = 0; y < 4; y++)
		free(temp_data[y]);

	for (y = 0; y < ref_image->src_width[0] * ref_image->src_height[0]; y++)
		for (x = 0; x < 4; x++)
		{
			ssim_ptr[x][y * 4] = (float)((ssim_ptr[x][y * 4] - range[x][0]) / (range[x][1] - range[x][0]));
		}

	for (y = 0; y < ref_image->src_width[0] * ref_image->src_height[0]; y++)
		for (x = 0; x < 4; x++)
		{
			ssim_ptr[x][y * 4 + 1] = ssim_ptr[x][y * 4];
			ssim_ptr[x][y * 4 + 2] = ssim_ptr[x][y * 4];
			ssim_ptr[x][y * 4 + 3] = 1.0f;
		}

	lx_range[0] = range[0][0];
	lx_range[1] = range[0][1];
	cx_range[0] = range[1][0];
	cx_range[1] = range[1][1];
	sx_range[0] = range[2][0];
	sx_range[1] = range[2][1];
	ssim_range[0] = range[3][0];
	ssim_range[1] = range[3][1];
}
void Image_SSIM_MapColours(image_t *ssim)
{
	int i;

	for (i = 0; i < ssim->src_width[0] * ssim->src_height[0]; i++)
	{
		int index = 1023 - Math_Clampi32((int)floorf(ssim->src_rgba_0_255[0][i * 4 + 0] * 1023.0f), 0, 1023);
		ssim->src_rgba_0_255[0][i * 4 + 0] = g_inferno_colourmap[index][0] / 255.0f;
		ssim->src_rgba_0_255[0][i * 4 + 1] = g_inferno_colourmap[index][1] / 255.0f;
		ssim->src_rgba_0_255[0][i * 4 + 2] = g_inferno_colourmap[index][2] / 255.0f;
	}
}
static int __cdecl Image_ErrListSort(void const *arg0, void const *arg1)
{
	double const *arg0d = arg0;
	double const *arg1d = arg1;
	double diff[2];

	diff[0] = arg0d[1] - arg0d[0];
	diff[1] = arg1d[1] - arg1d[0];

	if (diff[0] < diff[1])
		return -1;
	else if (diff[0] > diff[1])
		return 1;
	else
		return 0;
}
double Image_CompareBlocks(int options, image_t *reference, image_t *image0, image_t *image1, double *higher_quality, double *same_quality, double *lower_quality, double *error_upper, double *error_lower)
{
	int block_x;
	int block_y;
	int num_blocks_x = (reference->src_width[0] + 3) >> 2;
	int num_blocks_y = (reference->src_height[0] + 3) >> 2;
	int higher_quality_blocks = 0;
	int same_quality_blocks = 0;
	int lower_quality_blocks = 0;
	int total_blocks = num_blocks_x * num_blocks_y;
	double *err = malloc(num_blocks_x * num_blocks_y * sizeof(double) * 4);
	int i;

	*error_upper = 0.0;
	*error_lower = 0.0;

	for (block_x = 0; block_x < num_blocks_x * num_blocks_y; block_x++)
		err[block_x] = 0.0;

	for (block_y = 0; block_y < num_blocks_y; block_y++)
		for (block_x = 0; block_x < num_blocks_x; block_x++)
		{
			float block_rgba32f[3][4 * 4 * 4];
			int local_width = 4;
			int local_height = 4;
			int x;
			int y;
			int c;
			double mse[2] = {0.0, 0.0};

			if ((block_x + 1) * 4 > reference->src_width[0])
				local_width = reference->src_width[0] - block_x * 4;
			if ((block_y + 1) * 4 > reference->src_height[0])
				local_height = reference->src_height[0] - block_y * 4;

			Image_ExtractSubImageRGBAf32(block_rgba32f[0], reference, 0, 4, 4, block_x * 4, block_y * 4);
			Image_ExtractSubImageRGBAf32(block_rgba32f[1], image0, 0, 4, 4, block_x * 4, block_y * 4);
			Image_ExtractSubImageRGBAf32(block_rgba32f[2], image1, 0, 4, 4, block_x * 4, block_y * 4);

			for (y = 0; y < local_height; y++)
				for (x = 0; x < local_width; x++)
					for (c = 0; c < image0->num_channels; c++)
					{
						mse[0] += (block_rgba32f[1][(y*4 + x)*4 + c] - block_rgba32f[0][(y*4 + x)*4 + c])*(block_rgba32f[1][(y*4 + x)*4 + c] - block_rgba32f[0][(y*4 + x)*4 + c]);
						mse[1] += (block_rgba32f[2][(y*4 + x)*4 + c] - block_rgba32f[0][(y*4 + x)*4 + c])*(block_rgba32f[2][(y*4 + x)*4 + c] - block_rgba32f[0][(y*4 + x)*4 + c]);
					}
			if (mse[0] < mse[1])
			{
				higher_quality_blocks++;
				*error_upper += mse[1] - mse[0];
			}
			else if (mse[0] == mse[1])
				same_quality_blocks++;
			else
			{
				err[lower_quality_blocks*4 + 0] = mse[0];
				err[lower_quality_blocks*4 + 1] = mse[1];
				err[lower_quality_blocks*4 + 2] = block_x;
				err[lower_quality_blocks*4 + 3] = block_y;
				lower_quality_blocks++;
				*error_lower += mse[0] - mse[1];
			}
		}

	qsort(err, lower_quality_blocks, sizeof(double) * 4, Image_ErrListSort);

	if (!options)
	{
		if (lower_quality_blocks)
		{
			printf("Top %i highest error blocks:\n", (lower_quality_blocks < 10 ? lower_quality_blocks : 10));
			for (i = 0; i < (lower_quality_blocks < 10 ? lower_quality_blocks : 10); i++)
				printf("\t[%5i %5i]: %f vs %f (%f)\n", (int)err[i*4 + 2], (int)err[i*4 + 3], err[i*4 + 0], err[i*4 + 1], err[i*4 + 0] - err[i*4 + 1]);
		}
	}

	*higher_quality = higher_quality_blocks / (double)total_blocks;
	*same_quality = same_quality_blocks / (double)total_blocks;
	*lower_quality = lower_quality_blocks / (double)total_blocks;

	return higher_quality_blocks / (double)total_blocks;
}