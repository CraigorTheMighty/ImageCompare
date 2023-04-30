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
#include "..\inc\ktx\ktx.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES	0x8D64
#endif
#ifndef GL_ETC1_SRGB8_NV
#define GL_ETC1_SRGB8_NV	0x88EE
#endif

static char *KTXErrToString(KTX_error_code err)
{
	switch(err)
	{
		case KTX_SUCCESS:					return "KTX_SUCCESS";
		case KTX_FILE_DATA_ERROR:			return "KTX_FILE_DATA_ERROR";
		case KTX_FILE_ISPIPE:				return "KTX_FILE_ISPIPE";
		case KTX_FILE_OPEN_FAILED:			return "KTX_FILE_OPEN_FAILED";
		case KTX_FILE_OVERFLOW:				return "KTX_FILE_OVERFLOW";
		case KTX_FILE_READ_ERROR:			return "KTX_FILE_READ_ERROR";
		case KTX_FILE_SEEK_ERROR:			return "KTX_FILE_SEEK_ERROR";
		case KTX_FILE_UNEXPECTED_EOF:		return "KTX_FILE_UNEXPECTED_EOF";
		case KTX_FILE_WRITE_ERROR:			return "KTX_FILE_WRITE_ERROR";
		case KTX_GL_ERROR:					return "KTX_GL_ERROR";
		case KTX_INVALID_OPERATION:			return "KTX_INVALID_OPERATION";
		case KTX_INVALID_VALUE:				return "KTX_INVALID_VALUE";
		case KTX_NOT_FOUND:					return "KTX_NOT_FOUND";
		case KTX_OUT_OF_MEMORY:				return "KTX_OUT_OF_MEMORY";
		case KTX_TRANSCODE_FAILED:			return "KTX_TRANSCODE_FAILED";
		case KTX_UNKNOWN_FILE_FORMAT:		return "KTX_UNKNOWN_FILE_FORMAT";
		case KTX_UNSUPPORTED_TEXTURE_TYPE:	return "KTX_UNSUPPORTED_TEXTURE_TYPE";
		case KTX_UNSUPPORTED_FEATURE:		return "KTX_UNSUPPORTED_FEATURE";
		case KTX_LIBRARY_NOT_LINKED:		return "KTX_LIBRARY_NOT_LINKED";
		default:							return "<UNKNOWN>";
	}
}

int KTX_LoadImage(char *filename, image_t *image)
{
	FILE *f;
	ktxTexture1* texture;
	KTX_error_code result;
	ktxTextureCreateFlags flags = KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT | KTX_TEXTURE_CREATE_SKIP_KVDATA_BIT;
	uint8_t *raw_data;
	uint32_t raw_size;

	fopen_s(&f, filename, "rb");

	if (!f)
		return -1;

	result = ktxTexture1_CreateFromStdioStream(f, flags, &texture);

	if (result != KTX_SUCCESS)
	{
		printf("Error loading \"%s\" (ktxTexture1_CreateFromStdioStream error = %s)\n", filename, KTXErrToString(result));
		ktxTexture_Destroy((ktxTexture*)texture);
		fclose(f);
		return -1;
	}

	raw_data = ktxTexture_GetData((ktxTexture*)texture);
	raw_size = (uint32_t)ktxTexture_GetDataSize((ktxTexture*)texture);

	if (!raw_data)
	{
		printf("Error loading \"%s\" (ktxTexture_GetData error = 0x%p, size = %i)\n", filename, raw_data, raw_size);
		ktxTexture_Destroy((ktxTexture*)texture);
		fclose(f);
		return -1;
	}

	*image = Image_Create(texture->baseWidth, texture->baseHeight, 0, texture->glInternalformat);
	memcpy(image->dst_encoded[0], raw_data, raw_size);

	ktxTexture_Destroy((ktxTexture*)texture);

	fclose(f);

	return 0;
}