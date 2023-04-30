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
#include <dxgiformat.h>
#include <ddraw.h>

#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glext.h>
#include <gl\wglext.h>

#include "..\inc\image.h"

//#define DEBUG_BLOCK (block_x == 143 && block_y == 34)
//#define DEBUG_BLOCK (block_x == 115 && block_y == 86)
//#define DEBUG_BLOCK (block_x == 117 && block_y == 119)
#define DEBUG_BLOCK (0)

#define DDS_MAKE_FOURCC(s0, s1, s2, s3)	((s0 << 0) | (s1 << 8) | (s2 << 16) | (s3 << 24))

#define DDS_MAGIC								DDS_MAKE_FOURCC('D', 'D', 'S', ' ')

typedef enum D3DFORMAT 
{
	D3DFMT_UNKNOWN              =  0,

	D3DFMT_R8G8B8               = 20,
	D3DFMT_A8R8G8B8             = 21,
	D3DFMT_X8R8G8B8             = 22,
	D3DFMT_R5G6B5               = 23,
	D3DFMT_X1R5G5B5             = 24,
	D3DFMT_A1R5G5B5             = 25,
	D3DFMT_A4R4G4B4             = 26,
	D3DFMT_R3G3B2               = 27,
	D3DFMT_A8                   = 28,
	D3DFMT_A8R3G3B2             = 29,
	D3DFMT_X4R4G4B4             = 30,
	D3DFMT_A2B10G10R10          = 31,
	D3DFMT_A8B8G8R8             = 32,
	D3DFMT_X8B8G8R8             = 33,
	D3DFMT_G16R16               = 34,
	D3DFMT_A2R10G10B10          = 35,
	D3DFMT_A16B16G16R16         = 36,

	D3DFMT_A8P8                 = 40,
	D3DFMT_P8                   = 41,

	D3DFMT_L8                   = 50,
	D3DFMT_A8L8                 = 51,
	D3DFMT_A4L4                 = 52,

	D3DFMT_V8U8                 = 60,
	D3DFMT_L6V5U5               = 61,
	D3DFMT_X8L8V8U8             = 62,
	D3DFMT_Q8W8V8U8             = 63,
	D3DFMT_V16U16               = 64,
	D3DFMT_A2W10V10U10          = 67,

	D3DFMT_UYVY                 = DDS_MAKE_FOURCC('U', 'Y', 'V', 'Y'),
	D3DFMT_R8G8_B8G8            = DDS_MAKE_FOURCC('R', 'G', 'B', 'G'),
	D3DFMT_YUY2                 = DDS_MAKE_FOURCC('Y', 'U', 'Y', '2'),
	D3DFMT_G8R8_G8B8            = DDS_MAKE_FOURCC('G', 'R', 'G', 'B'),
	D3DFMT_DXT1                 = DDS_MAKE_FOURCC('D', 'X', 'T', '1'),
	D3DFMT_DXT2                 = DDS_MAKE_FOURCC('D', 'X', 'T', '2'),
	D3DFMT_DXT3                 = DDS_MAKE_FOURCC('D', 'X', 'T', '3'),
	D3DFMT_DXT4                 = DDS_MAKE_FOURCC('D', 'X', 'T', '4'),
	D3DFMT_DXT5                 = DDS_MAKE_FOURCC('D', 'X', 'T', '5'),

	D3DFMT_D16_LOCKABLE         = 70,
	D3DFMT_D32                  = 71,
	D3DFMT_D15S1                = 73,
	D3DFMT_D24S8                = 75,
	D3DFMT_D24X8                = 77,
	D3DFMT_D24X4S4              = 79,
	D3DFMT_D16                  = 80,

	D3DFMT_D32F_LOCKABLE        = 82,
	D3DFMT_D24FS8               = 83,

	D3DFMT_D32_LOCKABLE         = 84,
	D3DFMT_S8_LOCKABLE          = 85,

	D3DFMT_L16                  = 81,

	D3DFMT_VERTEXDATA           = 100,
	D3DFMT_INDEX16              = 101,
	D3DFMT_INDEX32              = 102,

	D3DFMT_Q16W16V16U16         = 110,

	D3DFMT_MULTI2_ARGB8         = DDS_MAKE_FOURCC('M', 'E', 'T', '1'),

	D3DFMT_R16F                 = 111,
	D3DFMT_G16R16F              = 112,
	D3DFMT_A16B16G16R16F        = 113,

	D3DFMT_R32F                 = 114,
	D3DFMT_G32R32F              = 115,
	D3DFMT_A32B32G32R32F        = 116,

	D3DFMT_CxV8U8               = 117,

	D3DFMT_A1                   = 118,
	D3DFMT_A2B10G10R10_XR_BIAS  = 119,
	D3DFMT_BINARYBUFFER         = 199,

	D3DFMT_FORCE_DWORD          = 0x7fffffff
}D3DFORMAT;

typedef struct dds_mixed_fmt_s
{
	int type;
	union
	{
		D3DFORMAT	d3dfmt;
		DXGI_FORMAT	dxgifmt;
	};
}dds_mixed_fmt_t;

typedef struct DDS_PIXELFORMAT
{
	DWORD dwSize;
	DWORD dwFlags;
	DWORD dwFourCC;
	DWORD dwRGBBitCount;
	DWORD dwRBitMask;
	DWORD dwGBitMask;
	DWORD dwBBitMask;
	DWORD dwABitMask;
}DDS_PIXELFORMAT;

typedef enum D3D10_RESOURCE_DIMENSION 
{
	D3D10_RESOURCE_DIMENSION_UNKNOWN = 0,
	D3D10_RESOURCE_DIMENSION_BUFFER = 1,
	D3D10_RESOURCE_DIMENSION_TEXTURE1D = 2,
	D3D10_RESOURCE_DIMENSION_TEXTURE2D = 3,
	D3D10_RESOURCE_DIMENSION_TEXTURE3D = 4
}D3D10_RESOURCE_DIMENSION;

typedef struct dds_header_s
{
	DWORD           dwSize;
	DWORD           dwFlags;
	DWORD           dwHeight;
	DWORD           dwWidth;
	DWORD           dwPitchOrLinearSize;
	DWORD           dwDepth;
	DWORD           dwMipMapCount;
	DWORD           dwReserved1[11];
	DDS_PIXELFORMAT ddspf;
	DWORD           dwCaps;
	DWORD           dwCaps2;
	DWORD           dwCaps3;
	DWORD           dwCaps4;
	DWORD           dwReserved2;
}dds_header_t;

typedef struct dds_header_dxt10_s
{
	DXGI_FORMAT              dxgiFormat;
	D3D10_RESOURCE_DIMENSION resourceDimension;
	UINT                     miscFlag;
	UINT                     arraySize;
	UINT                     miscFlags2;
}dds_header_dxt10_t;
typedef struct dds_s
{
	DWORD				magic;
	dds_header_t		header;
	dds_header_dxt10_t	header_dxt10;
}dds_t;


int DDS_BlockSizeFromGLFormat(int format)
{
	switch (format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
		case GL_COMPRESSED_RED_RGTC1:           // BC4
			//case GL_COMPRESSED_SIGNED_RED_RGTC1:    // BC4
			return 8;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			//case GL_COMPRESSED_SIGNED_RG_RGTC2:     // BC5
			//case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: // BC6
			//case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: // BC6
			//case GL_COMPRESSED_RGBA_BPTC_UNORM:     // BC7
			//case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: // BC7
			return 16;
		default:
			return -1;
	}
}
DWORD DDS_GLFormatToDXFormat(int format)
{
	switch(format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
			return DXGI_FORMAT_BC1_UNORM;
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
			return DXGI_FORMAT_BC1_UNORM;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
			return DXGI_FORMAT_BC1_UNORM_SRGB;
		case GL_COMPRESSED_RED_RGTC1:           // BC4
			return DXGI_FORMAT_BC4_UNORM;
		case GL_COMPRESSED_SIGNED_RED_RGTC1:    // BC4
			return DXGI_FORMAT_BC4_SNORM;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
			return DXGI_FORMAT_BC2_UNORM;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
			return DXGI_FORMAT_BC2_UNORM_SRGB;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
			return DXGI_FORMAT_BC3_UNORM;
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
			return DXGI_FORMAT_BC3_UNORM_SRGB;
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			return DXGI_FORMAT_BC5_UNORM;
		case GL_COMPRESSED_SIGNED_RG_RGTC2:     // BC5
			return DXGI_FORMAT_BC5_SNORM;
			//case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT: // BC6
			//case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT: // BC6
			//case GL_COMPRESSED_RGBA_BPTC_UNORM:     // BC7
			//case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM: // BC7
			//DDS_EncodeBC7Blockf32(local_block, pixel_rgbaf64, 0, 0, 0, x, y);
			//break;
		default:
			return 0;
	}
}
DWORD DDS_DX10FormatToGLFormat(int format)
{
	switch(format)
	{
		case DXGI_FORMAT_BC1_UNORM:
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;   // BC1
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;  // BC1
		case DXGI_FORMAT_BC4_UNORM:
			return GL_COMPRESSED_RED_RGTC1;           // BC4
		case DXGI_FORMAT_BC4_SNORM:
			return GL_COMPRESSED_SIGNED_RED_RGTC1;    // BC4
		case DXGI_FORMAT_BC2_UNORM:
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;  // BC2
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT; // BC2
		case DXGI_FORMAT_BC3_UNORM:
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;  // BC3
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT; // BC3
		case DXGI_FORMAT_BC5_UNORM:
			return GL_COMPRESSED_RG_RGTC2;            // BC5
		case DXGI_FORMAT_BC5_SNORM:
			return GL_COMPRESSED_SIGNED_RG_RGTC2;     // BC5
		default:
			return 0;
	}
}
DWORD DDS_DX9FormatToGLFormat(int format)
{
	switch(format)
	{
		case DDS_MAKE_FOURCC('D', 'X', 'T', '1'):
			return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		case DDS_MAKE_FOURCC('B', 'C', '4', 'U'):
			return GL_COMPRESSED_RED_RGTC1;
		case DDS_MAKE_FOURCC('D', 'X', 'T', '3'):
			return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		case DDS_MAKE_FOURCC('D', 'X', 'T', '5'):
			return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		case DDS_MAKE_FOURCC('A', 'T', 'I', '2'):
			return GL_COMPRESSED_RG_RGTC2;
		default:
			return 0;
	}
}

void Decode_BC1(float *block_out, uint8_t *block_in, int block_x, int block_y)
{
	uint8_t colour[4][4];
	int i;
	int j;

	colour[0][0] = (uint8_t)((*((uint16_t*)&block_in[0]) & 0xF800) >> 8);
	colour[0][1] = (uint8_t)((*((uint16_t*)&block_in[0]) & 0x07E0) >> 3);
	colour[0][2] = (uint8_t)((*((uint16_t*)&block_in[0]) & 0x001F) << 3);
	colour[0][3] = 255;

	colour[1][0] = (uint8_t)((*((uint16_t*)&block_in[2]) & 0xF800) >> 8);
	colour[1][1] = (uint8_t)((*((uint16_t*)&block_in[2]) & 0x07E0) >> 3);
	colour[1][2] = (uint8_t)((*((uint16_t*)&block_in[2]) & 0x001F) << 3);
	colour[1][3] = 255;

	colour[0][0] |= colour[0][0] >> 5;
	colour[0][1] |= colour[0][1] >> 6;
	colour[0][2] |= colour[0][2] >> 5;

	colour[1][0] |= colour[1][0] >> 5;
	colour[1][1] |= colour[1][1] >> 6;
	colour[1][2] |= colour[1][2] >> 5;

	if (*((uint16_t*)&block_in[0]) > *((uint16_t*)&block_in[2]))
	{
		// according to bcdec, r = (2 * r0 + r1 + 1) / 3;
		colour[2][0] = (uint8_t)((colour[0][0] * (2.0/3.0) + colour[1][0] * (1.0 / 3.0)) + 0.5);
		colour[2][1] = (uint8_t)((colour[0][1] * (2.0/3.0) + colour[1][1] * (1.0 / 3.0)) + 0.5);
		colour[2][2] = (uint8_t)((colour[0][2] * (2.0/3.0) + colour[1][2] * (1.0 / 3.0)) + 0.5);
		colour[2][3] = 255;

		// according to bcdec, r = (r0 + 2 * r1 + 1) / 3;
		colour[3][0] = (uint8_t)((colour[0][0] * (1.0/3.0) + colour[1][0] * (2.0 / 3.0)) + 0.5);
		colour[3][1] = (uint8_t)((colour[0][1] * (1.0/3.0) + colour[1][1] * (2.0 / 3.0)) + 0.5);
		colour[3][2] = (uint8_t)((colour[0][2] * (1.0/3.0) + colour[1][2] * (2.0 / 3.0)) + 0.5);
		colour[3][3] = 255;
	}
	else
	{
		colour[2][0] = (uint8_t)((colour[0][0] * (1.0/2.0) + colour[1][0] * (1.0 / 2.0)) + 0.5);
		colour[2][1] = (uint8_t)((colour[0][1] * (1.0/2.0) + colour[1][1] * (1.0 / 2.0)) + 0.5);
		colour[2][2] = (uint8_t)((colour[0][2] * (1.0/2.0) + colour[1][2] * (1.0 / 2.0)) + 0.5);
		colour[2][3] = 255;

		colour[3][0] = 0;
		colour[3][1] = 0;
		colour[3][2] = 0;
		colour[3][3] = 0;
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			int index = (block_in[4 + i] >> (j * 2)) & 0x03;

			block_out[i * 16 + j * 4 + 0] = (float)colour[index][0];
			block_out[i * 16 + j * 4 + 1] = (float)colour[index][1];
			block_out[i * 16 + j * 4 + 2] = (float)colour[index][2];
			block_out[i * 16 + j * 4 + 3] = (float)colour[index][3];
		}
	}
}

void Decode_BC2(float *block_out, uint8_t *block_in, int block_x, int block_y)
{
	uint8_t colour[4][3];
	uint8_t alpha[16];
	int i, j;

	for (i = 0; i < 8; i++)
	{
		alpha[i * 2 + 0]  = (uint8_t)(((uint32_t)((block_in[i] >> 0) & 0x0F) * 255) / 15);
		alpha[i * 2 + 1]  = (uint8_t)(((uint32_t)((block_in[i] >> 4) & 0x0F) * 255) / 15);
	}

	colour[0][0] = (uint8_t)((*((uint16_t*)&block_in[8]) & 0xF800) >> 8);
	colour[0][1] = (uint8_t)((*((uint16_t*)&block_in[8]) & 0x07E0) >> 3);
	colour[0][2] = (uint8_t)((*((uint16_t*)&block_in[8]) & 0x001F) << 3);

	colour[1][0] = (uint8_t)((*((uint16_t*)&block_in[10]) & 0xF800) >> 8);
	colour[1][1] = (uint8_t)((*((uint16_t*)&block_in[10]) & 0x07E0) >> 3);
	colour[1][2] = (uint8_t)((*((uint16_t*)&block_in[10]) & 0x001F) << 3);

	for (i = 0; i < 3; i++)
	{
		colour[2][i] = (uint8_t)(colour[0][i] * (2.0/3.0) + colour[1][i] * (1.0 / 3.0));
		colour[3][i] = (uint8_t)(colour[0][i] * (1.0/3.0) + colour[1][i] * (2.0 / 3.0));
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			int index = (block_in[12 + i] >> (j * 2)) & 0x03;

			block_out[i * 16 + j * 4 + 0] = (float)colour[index][0];
			block_out[i * 16 + j * 4 + 1] = (float)colour[index][1];
			block_out[i * 16 + j * 4 + 2] = (float)colour[index][2];
			block_out[i * 16 + j * 4 + 3] = (float)alpha[i * 4 + j];
		}
	}
}

void Decode_BC3(float *block_out, uint8_t *block_in, int block_x, int block_y)
{
	uint8_t colour[4][3];
	uint8_t alpha[8];
	int alpha_index[16];
	int i, j;
	uint32_t alpha_block[2];

	alpha[0] = block_in[0];
	alpha[1] = block_in[1];

	if (alpha[0] > alpha[1])
	{
		alpha[2] = (uint8_t)(alpha[0] * 6.0 / 7.0 + alpha[1] * 1.0 / 7.0);
		alpha[3] = (uint8_t)(alpha[0] * 5.0 / 7.0 + alpha[1] * 2.0 / 7.0);
		alpha[4] = (uint8_t)(alpha[0] * 4.0 / 7.0 + alpha[1] * 3.0 / 7.0);
		alpha[5] = (uint8_t)(alpha[0] * 3.0 / 7.0 + alpha[1] * 4.0 / 7.0);
		alpha[6] = (uint8_t)(alpha[0] * 2.0 / 7.0 + alpha[1] * 5.0 / 7.0);
		alpha[7] = (uint8_t)(alpha[0] * 1.0 / 7.0 + alpha[1] * 6.0 / 7.0);
	}
	else
	{
		alpha[2] = (uint8_t)(alpha[0] * 4.0 / 5.0 + alpha[1] * 1.0 / 5.0);
		alpha[3] = (uint8_t)(alpha[0] * 3.0 / 5.0 + alpha[1] * 2.0 / 5.0);
		alpha[4] = (uint8_t)(alpha[0] * 2.0 / 5.0 + alpha[1] * 3.0 / 5.0);
		alpha[5] = (uint8_t)(alpha[0] * 1.0 / 5.0 + alpha[1] * 4.0 / 5.0);
		alpha[6] = 0;
		alpha[7] = 255;
	}

	alpha_block[0] = block_in[2];
	alpha_block[0] |= block_in[3] << 8;
	alpha_block[0] |= block_in[4] << 16;
	alpha_block[1] = block_in[5];
	alpha_block[1] |= block_in[6] << 8;
	alpha_block[1] |= block_in[7] << 16;

	for (i = 0; i < 2; i++)
	{
		alpha_index[0 + i * 8] = (alpha_block[i] >> 0) & 0x07;
		alpha_index[1 + i * 8] = (alpha_block[i] >> 3) & 0x07;
		alpha_index[2 + i * 8] = (alpha_block[i] >> 6) & 0x07;
		alpha_index[3 + i * 8] = (alpha_block[i] >> 9) & 0x07;
		alpha_index[4 + i * 8] = (alpha_block[i] >> 12) & 0x07;
		alpha_index[5 + i * 8] = (alpha_block[i] >> 15) & 0x07;
		alpha_index[6 + i * 8] = (alpha_block[i] >> 18) & 0x07;
		alpha_index[7 + i * 8] = (alpha_block[i] >> 21) & 0x07;
	}

	colour[0][0] = (*((uint16_t*)&block_in[8]) & 0xF800) >> 8;
	colour[0][1] = (*((uint16_t*)&block_in[8]) & 0x07E0) >> 3;
	colour[0][2] = (*((uint16_t*)&block_in[8]) & 0x001F) << 3;

	colour[1][0] = (*((uint16_t*)&block_in[10]) & 0xF800) >> 8;
	colour[1][1] = (*((uint16_t*)&block_in[10]) & 0x07E0) >> 3;
	colour[1][2] = (*((uint16_t*)&block_in[10]) & 0x001F) << 3;

	for (i = 0; i < 3; i++)
	{
		colour[2][i] = (uint8_t)(colour[0][i] * (2.0/3.0) + colour[1][i] * (1.0 / 3.0));
		colour[3][i] = (uint8_t)(colour[0][i] * (1.0/3.0) + colour[1][i] * (2.0 / 3.0));
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			int index = (block_in[12 + i] >> (j * 2)) & 0x03;

			block_out[i * 16 + j * 4 + 0] = (float)colour[index][0];
			block_out[i * 16 + j * 4 + 1] = (float)colour[index][1];
			block_out[i * 16 + j * 4 + 2] = (float)colour[index][2];
			block_out[i * 16 + j * 4 + 3] = (float)alpha[alpha_index[i * 4 + j]];
		}
	}
}


void Decode_BC4(float *block_out, uint8_t *block_in, int block_x, int block_y)
{
	int16_t red[8];
	int red_index[16];
	int i, j;
	uint32_t red_block[2];

	red[0] = (uint8_t)block_in[0];
	red[1] = (uint8_t)block_in[1];

	if (red[0] > red[1])
	{
		red[2] = (int16_t)floorf(red[0] * 6.0f / 7.0f + red[1] * 1.0f / 7.0f + 0.5f);
		red[3] = (int16_t)floorf(red[0] * 5.0f / 7.0f + red[1] * 2.0f / 7.0f + 0.5f);
		red[4] = (int16_t)floorf(red[0] * 4.0f / 7.0f + red[1] * 3.0f / 7.0f + 0.5f);
		red[5] = (int16_t)floorf(red[0] * 3.0f / 7.0f + red[1] * 4.0f / 7.0f + 0.5f);
		red[6] = (int16_t)floorf(red[0] * 2.0f / 7.0f + red[1] * 5.0f / 7.0f + 0.5f);
		red[7] = (int16_t)floorf(red[0] * 1.0f / 7.0f + red[1] * 6.0f / 7.0f + 0.5f);
	}
	else
	{
		red[2] = (int16_t)floorf(red[0] * 4.0f / 5.0f + red[1] * 1.0f / 5.0f + 0.5f);
		red[3] = (int16_t)floorf(red[0] * 3.0f / 5.0f + red[1] * 2.0f / 5.0f + 0.5f);
		red[4] = (int16_t)floorf(red[0] * 2.0f / 5.0f + red[1] * 3.0f / 5.0f + 0.5f);
		red[5] = (int16_t)floorf(red[0] * 1.0f / 5.0f + red[1] * 4.0f / 5.0f + 0.5f);
		red[6] = 0;
		red[7] = 255;
	}

	red_block[0] = block_in[2];
	red_block[0] |= block_in[3] << 8;
	red_block[0] |= block_in[4] << 16;
	red_block[1] = block_in[5];
	red_block[1] |= block_in[6] << 8;
	red_block[1] |= block_in[7] << 16;

	for (i = 0; i < 2; i++)
	{
		red_index[0 + i * 8] = (red_block[i] >> 0) & 0x07;
		red_index[1 + i * 8] = (red_block[i] >> 3) & 0x07;
		red_index[2 + i * 8] = (red_block[i] >> 6) & 0x07;
		red_index[3 + i * 8] = (red_block[i] >> 9) & 0x07;
		red_index[4 + i * 8] = (red_block[i] >> 12) & 0x07;
		red_index[5 + i * 8] = (red_block[i] >> 15) & 0x07;
		red_index[6 + i * 8] = (red_block[i] >> 18) & 0x07;
		red_index[7 + i * 8] = (red_block[i] >> 21) & 0x07;
	}

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			block_out[i * 16 + j * 4 + 0] = (float)red[red_index[i * 4 + j]];
			block_out[i * 16 + j * 4 + 1] = (float)0;
			block_out[i * 16 + j * 4 + 2] = (float)0;
			block_out[i * 16 + j * 4 + 3] = 255;
		}
	}
}

void Decode_BC5(float *block_out, uint8_t *block_in, int block_x, int block_y)
{
	int16_t red[8];
	int16_t green[8];
	int red_index[16];
	int green_index[16];
	int i, j;
	uint32_t red_block[2];
	uint32_t green_block[2];

	red[0] = (uint8_t)block_in[0];
	red[1] = (uint8_t)block_in[1];

	if (red[0] > red[1])
	{
		red[2] = (int16_t)floorf(red[0] * 6.0f / 7.0f + red[1] * 1.0f / 7.0f + 0.5f);
		red[3] = (int16_t)floorf(red[0] * 5.0f / 7.0f + red[1] * 2.0f / 7.0f + 0.5f);
		red[4] = (int16_t)floorf(red[0] * 4.0f / 7.0f + red[1] * 3.0f / 7.0f + 0.5f);
		red[5] = (int16_t)floorf(red[0] * 3.0f / 7.0f + red[1] * 4.0f / 7.0f + 0.5f);
		red[6] = (int16_t)floorf(red[0] * 2.0f / 7.0f + red[1] * 5.0f / 7.0f + 0.5f);
		red[7] = (int16_t)floorf(red[0] * 1.0f / 7.0f + red[1] * 6.0f / 7.0f + 0.5f);
	}
	else
	{
		red[2] = (int16_t)floorf(red[0] * 4.0f / 5.0f + red[1] * 1.0f / 5.0f + 0.5f);
		red[3] = (int16_t)floorf(red[0] * 3.0f / 5.0f + red[1] * 2.0f / 5.0f + 0.5f);
		red[4] = (int16_t)floorf(red[0] * 2.0f / 5.0f + red[1] * 3.0f / 5.0f + 0.5f);
		red[5] = (int16_t)floorf(red[0] * 1.0f / 5.0f + red[1] * 4.0f / 5.0f + 0.5f);
		red[6] = 0;
		red[7] = 255;
	}

	red_block[0] = block_in[2];
	red_block[0] |= block_in[3] << 8;
	red_block[0] |= block_in[4] << 16;
	red_block[1] = block_in[5];
	red_block[1] |= block_in[6] << 8;
	red_block[1] |= block_in[7] << 16;

	for (i = 0; i < 2; i++)
	{
		red_index[0 + i * 8] = (red_block[i] >> 0) & 0x07;
		red_index[1 + i * 8] = (red_block[i] >> 3) & 0x07;
		red_index[2 + i * 8] = (red_block[i] >> 6) & 0x07;
		red_index[3 + i * 8] = (red_block[i] >> 9) & 0x07;
		red_index[4 + i * 8] = (red_block[i] >> 12) & 0x07;
		red_index[5 + i * 8] = (red_block[i] >> 15) & 0x07;
		red_index[6 + i * 8] = (red_block[i] >> 18) & 0x07;
		red_index[7 + i * 8] = (red_block[i] >> 21) & 0x07;
	}

	green[0] = (uint8_t)block_in[8];
	green[1] = (uint8_t)block_in[9];

	if (green[0] > green[1])
	{
		green[2] = (int16_t)floorf(green[0] * 6.0f / 7.0f + green[1] * 1.0f / 7.0f + 0.5f);
		green[3] = (int16_t)floorf(green[0] * 5.0f / 7.0f + green[1] * 2.0f / 7.0f + 0.5f);
		green[4] = (int16_t)floorf(green[0] * 4.0f / 7.0f + green[1] * 3.0f / 7.0f + 0.5f);
		green[5] = (int16_t)floorf(green[0] * 3.0f / 7.0f + green[1] * 4.0f / 7.0f + 0.5f);
		green[6] = (int16_t)floorf(green[0] * 2.0f / 7.0f + green[1] * 5.0f / 7.0f + 0.5f);
		green[7] = (int16_t)floorf(green[0] * 1.0f / 7.0f + green[1] * 6.0f / 7.0f + 0.5f);
	}
	else
	{
		green[2] = (int16_t)floorf(green[0] * 4.0f / 5.0f + green[1] * 1.0f / 5.0f + 0.5f);
		green[3] = (int16_t)floorf(green[0] * 3.0f / 5.0f + green[1] * 2.0f / 5.0f + 0.5f);
		green[4] = (int16_t)floorf(green[0] * 2.0f / 5.0f + green[1] * 3.0f / 5.0f + 0.5f);
		green[5] = (int16_t)floorf(green[0] * 1.0f / 5.0f + green[1] * 4.0f / 5.0f + 0.5f);
		green[6] = 0;
		green[7] = 255;
	}

	green_block[0] = block_in[10];
	green_block[0] |= block_in[11] << 8;
	green_block[0] |= block_in[12] << 16;

	green_block[1] = block_in[13];
	green_block[1] |= block_in[14] << 8;
	green_block[1] |= block_in[15] << 16;

	for (i = 0; i < 2; i++)
	{
		green_index[0 + i * 8] = (green_block[i] >> 0) & 0x07;
		green_index[1 + i * 8] = (green_block[i] >> 3) & 0x07;
		green_index[2 + i * 8] = (green_block[i] >> 6) & 0x07;
		green_index[3 + i * 8] = (green_block[i] >> 9) & 0x07;
		green_index[4 + i * 8] = (green_block[i] >> 12) & 0x07;
		green_index[5 + i * 8] = (green_block[i] >> 15) & 0x07;
		green_index[6 + i * 8] = (green_block[i] >> 18) & 0x07;
		green_index[7 + i * 8] = (green_block[i] >> 21) & 0x07;
	}
	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{
			block_out[i * 16 + j * 4 + 0] = (float)red[red_index[i * 4 + j]];
			block_out[i * 16 + j * 4 + 1] = (float)green[green_index[i * 4 + j]];
			block_out[i * 16 + j * 4 + 2] = (float)0;
			block_out[i * 16 + j * 4 + 3] = 255;
		}
	}
}


void DDS_DecodeBCN(image_t *image)
{
	int blocks_x = (image->src_width[0] + 3) >> 2;
	int blocks_y = (image->src_height[0] + 3) >> 2;
	int x;
	int y;
	int i;
	void (*decode_fp)(float *block_out, uint8_t *block_in, int block_x, int block_y) = 0;

	switch(image->dst_format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
			decode_fp = Decode_BC1;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
			decode_fp = Decode_BC2;
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
			decode_fp = Decode_BC3;
			break;
		case GL_COMPRESSED_RED_RGTC1:           // BC4
			decode_fp = Decode_BC4;
			break;
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			decode_fp = Decode_BC5;
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

int DDS_LoadImage(char *filename, image_t *image)
{
	dds_t dds;
	FILE *f;
	int gl_format;
	const int num_elements_common = 32;
	const int num_elements_dx10 = 5;
	int read_elements = 0;

	fopen_s(&f, filename, "rb");

	if (!f)
		return -1;

	read_elements += (int)fread(&dds.magic, sizeof(DWORD), 1, f);

	read_elements += (int)fread(&dds.header.dwSize, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwFlags, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwHeight, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwWidth, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwPitchOrLinearSize, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwDepth, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwMipMapCount, sizeof(DWORD), 1, f);
	read_elements += (int)fread(dds.header.dwReserved1, sizeof(DWORD), 11, f);

	read_elements += (int)fread(&dds.header.ddspf.dwSize, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwFlags, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwFourCC, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwRGBBitCount, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwRBitMask, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwGBitMask, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwBBitMask, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.ddspf.dwABitMask, sizeof(DWORD), 1, f);

	read_elements += (int)fread(&dds.header.dwCaps, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwCaps2, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwCaps3, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwCaps4, sizeof(DWORD), 1, f);
	read_elements += (int)fread(&dds.header.dwReserved2, sizeof(DWORD), 1, f);

	if (dds.magic != DDS_MAGIC)
	{
		fclose(f);
		return -1;
	}
	if (dds.header.dwSize != 124)
	{
		fclose(f);
		return -1;
	}

	if (dds.header.ddspf.dwFourCC == DDS_MAKE_FOURCC('D', 'X', '1', '0'))
	{
		read_elements += (int)fread(&dds.header_dxt10.dxgiFormat, sizeof(DXGI_FORMAT), 1, f);
		read_elements += (int)fread(&dds.header_dxt10.resourceDimension, sizeof(D3D10_RESOURCE_DIMENSION), 1, f);
		read_elements += (int)fread(&dds.header_dxt10.miscFlag, sizeof(UINT), 1, f);
		read_elements += (int)fread(&dds.header_dxt10.arraySize, sizeof(UINT), 1, f);
		read_elements += (int)fread(&dds.header_dxt10.miscFlags2, sizeof(UINT), 1, f);

		if (read_elements != num_elements_common + num_elements_dx10)
		{
			fclose(f);
			return -1;
		}

		gl_format = DDS_DX10FormatToGLFormat(dds.header_dxt10.dxgiFormat);
	}
	else
	{
		if (read_elements != num_elements_common)
		{
			fclose(f);
			return -1;
		}

		gl_format = DDS_DX9FormatToGLFormat(dds.header.ddspf.dwFourCC);
	}

	*image = Image_Create(dds.header.dwWidth, dds.header.dwHeight, 0, gl_format);

	read_elements = (int)fread(image->dst_encoded[0], 1, DDS_BlockSizeFromGLFormat(gl_format) * ((dds.header.dwWidth + 3) >> 2) * ((dds.header.dwHeight + 3) >> 2), f);

	if (read_elements != DDS_BlockSizeFromGLFormat(gl_format) * ((dds.header.dwWidth + 3) >> 2) * ((dds.header.dwHeight + 3) >> 2))
	{
		fclose(f);
		Image_Destroy(image);
		return -1;
	}

	fclose(f);

	return 0;
}