#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <conio.h>
#include <process.h>
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

#include "..\inc\IL\il.h"
#include "..\inc\IL\ilu.h"
#include "..\inc\IL\ilut.h"

#include "..\inc\image.h"
#include "..\inc\dds.h"
#include "..\inc\etc.h"
#include "..\inc\ktx_loader.h"

#ifndef GL_ETC1_RGB8_OES
#define GL_ETC1_RGB8_OES	0x8D64
#endif
#ifndef GL_ETC1_SRGB8_NV
#define GL_ETC1_SRGB8_NV	0x88EE
#endif

#define FORMAT_GROUP_BCN	0
#define FORMAT_GROUP_ETC	1
#define FORMAT_GROUP_RGB	2

static char *g_compare[3] = {0};
static int g_num_files = 0;
static int g_output_ssim = 0;
static double g_ssim_lower = 0.0;
static double g_ssim_upper = 1.0;
static int g_minimal = 0;
static char *g_output_dir = 0;
static int g_window = 5;
static int g_ssim_literal = 0;
static int g_decode = 0;

void System_PrintUsage()
{
	printf("\n");
	printf("=====\n");
	printf("Usage\n");
	printf("=====\n");
	printf("\n");
	printf("Compare [options]\n");
	printf("\n");
	printf("Example: Compare -ref \"test_image.png\" -comp \"encoded1.ktx\" -comp \"encoded2.ktx\" -ssim -outdir \"output\"\n");
	printf("\n");
	printf("=======\n");
	printf("Options\n");
	printf("=======\n");
	printf("\n");
	printf("\n");
	printf("-ref \"[filename]\"\n");
	printf("\n");
	printf("\tAttempt to load image file [filename] as the reference file. This option MUST be specified exactly once.\n");
	printf("\n");
	printf("-comp \"[filename]\"\n");
	printf("\n");
	printf("\tAttempt to load image file [filename] as a comparison file. This option MUST be specified either one or two times.\n");
	printf("\n");
	printf("\t\tIf specified once, the image will be compared to the reference image.\n");
	printf("\t\tIf specified twice, both images will be compared to the reference image and to each other.\n");
	printf("\n");
	printf("-minimal\n");
	printf("\n");
	printf("\tOnly enable minimal log prints.\n");
	printf("\n");
	printf("-decode\n");
	printf("\n");
	printf("\tDecode the reference and comparison images and save as .tga files.\n");
	printf("\n");
	printf("-ssim\n");
	printf("\n");
	printf("\tEnable SSIM comparisons.\n");
	printf("\n");
	printf("-outdir [output path]\n");
	printf("\n");
	printf("\tSaves SSIM and/or decoded output to the specified directory. The specified directory MUST already exist.\n");
	printf("\n");
	printf("=====================\n");
	printf("SSIM-specific options\n");
	printf("=====================\n");
	printf("\n");
	printf("\tIf this option is not specified, SSIM files will be saved in same directories as the files they represent.\n");
	printf("\n");
	printf("-ssim_window [value]\n");
	printf("\n");
	printf("\tSet the width of the Gaussian SSIM window. This value will be silently clamped to the range [0, 16]. Default value is 5.\n");
	printf("\n");
	printf("-ssim_literal\n");
	printf("\n");
	printf("\tInterpret \"ssim_range\" as a literal range, rather than percentiles.\n");
	printf("\n");
	printf("-ssim_range [value lower] [value upper]\n");
	printf("\n");
	printf("\tSet the upper and lower bound of values displayed in SSIM output images. The default range is [0, 1].\n");
	printf("\n");
	printf("\tIf \"ssim_literal\" has not been specified, this is the range of percentiles to display. e.g. \"ssim_range 0.2 0.7\" will\n");
	printf("\tscale the output SSIM images display values to only show values that lie between the 20th and 70th percentile, and the\n");
	printf("\tdefault range \"ssim_range 0 1\" will scale the output to display all values.\n");
	printf("\n");
	printf("\tIf \"ssim_literal\" has been specified, this is the literal range to display, e.g. \"ssim_range 0.992 0.997\". In this case,\n");
	printf("\tthe original range can be shown by specifying \"-ssim\" without \"ssim_literal\" and without \"ssim_range\".\n");
	printf("\n");
}

int System_ParseOptions(int argc, char **argv)
{
	int i;
	int file_index = 1;

	for (i = 1; i < argc; i++)
	{
		if (!_stricmp(argv[i], "-ref"))
		{
			i++;
			if (i == argc)
				return 1;
			g_compare[0] = malloc(strlen(argv[i]) + 1);
			strcpy_s(g_compare[0], strlen(argv[i]) + 1, argv[i]);
		}
		else if (!_stricmp(argv[i], "-ssim"))
		{
			g_output_ssim = 1;
		}
		else if (!_stricmp(argv[i], "-ssim_range"))
		{
			i++;
			if (i == argc)
				return 1;
			g_ssim_lower = atof(argv[i]);
			i++;
			if (i == argc)
				return 1;
			g_ssim_upper = atof(argv[i]);
		}
		else if (!_stricmp(argv[i], "-ssim_literal"))
		{
			g_ssim_literal = 1;
		}
		else if (!_stricmp(argv[i], "-minimal"))
		{
			g_minimal = 1;
		}
		else if (!_stricmp(argv[i], "-decode"))
		{
			g_decode = 1;
		}
		else if (!_stricmp(argv[i], "-ssim_window"))
		{
			i++;
			if (i == argc)
				return 1;
			g_window = (atoi(argv[i]) - 1) / 2;
		}
		else if (!_stricmp(argv[i], "-outdir"))
		{
			char *ptr[2];
			i++;
			if (i == argc)
				return 1;
			g_output_dir = malloc(strlen(argv[i]) + 1);
			strcpy_s(g_output_dir, strlen(argv[i]) + 1, argv[i]);

			while(ptr[1] = strchr(g_output_dir, '/'))
			{
				*(strchr(g_output_dir, '/')) = '\\';
			}
			while((strlen(g_output_dir) > 0) && (g_output_dir[strlen(g_output_dir) - 1] == '\\'))
				g_output_dir[strlen(g_output_dir) - 1] = 0;
		}
		else if (!_stricmp(argv[i], "-comp"))
		{
			if (file_index == 3)
				return -1;
			i++;
			if (i == argc)
				return 1;
			g_compare[file_index] = malloc(strlen(argv[i]) + 1);
			strcpy_s(g_compare[file_index], strlen(argv[i]) + 1, argv[i]);
			file_index++;
			g_num_files++;
		}
		else
			return 1;
	}
	return 0;
}

int System_GLFormatToFormatGroup(int format)
{
	switch (format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:   // BC1
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:  // BC1
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: // BC1
		case GL_COMPRESSED_RED_RGTC1:           // BC4
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:  // BC2
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: // BC2
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:  // BC3
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: // BC3
		case GL_COMPRESSED_RG_RGTC2:            // BC5
			return FORMAT_GROUP_BCN;
		case GL_ETC1_RGB8_OES:
		case GL_COMPRESSED_SRGB8_ETC2:
		case GL_COMPRESSED_RGB8_ETC2:
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:
		case GL_COMPRESSED_RGBA8_ETC2_EAC:
			return FORMAT_GROUP_ETC;
		default:
			return -1;
	}
}
char *System_GLFormatAsString(int format)
{
	switch (format)
	{
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGB_S3TC_DXT1_EXT";
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT";
		case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT: return "GL_COMPRESSED_SRGB_S3TC_DXT1_EXT";
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT";
		case GL_COMPRESSED_RED_RGTC1: return "GL_COMPRESSED_RED_RGTC1";
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT";
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT";
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT: return "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT";
		case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: return "GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT";
		case GL_COMPRESSED_RG_RGTC2: return "GL_COMPRESSED_RG_RGTC2";
		case GL_ETC1_RGB8_OES: return "GL_ETC1_RGB8_OES";
		case GL_COMPRESSED_SRGB8_ETC2: return "GL_COMPRESSED_SRGB8_ETC2";
		case GL_COMPRESSED_RGB8_ETC2: return "GL_COMPRESSED_RGB8_ETC2";
		case GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2: return "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2";
		case GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: return "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2";
		case GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC: return "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC";
		case GL_COMPRESSED_RGBA8_ETC2_EAC: return "GL_COMPRESSED_RGBA8_ETC2_EAC";
		default:	return "<UNKNOWN>";
	}
}

int System_LoadFile(image_t *image, char *filename)
{
	char *ptr;
	ILboolean result;
	int width;
	int height;
	float *rgba;
	ILuint uint_result;
	ILuint devil_image;
	int ret;
	wchar_t in_wpath[2048];

	swprintf(in_wpath, 2048, L"%S", filename);

	ptr = strrchr(filename, '.');

	if (!ptr)
	{
		printf("Unknown type of file \"%s\"\n", filename);
		return -1;
	}

	if (!_strnicmp(ptr, ".dds", 4))
	{
		ret = DDS_LoadImage(filename, image);
		if (ret)
		{
			printf("Error reading file \"%s\"\n", filename);
			return -1;
		}
		DDS_DecodeBCN(image);
		return FORMAT_GROUP_BCN;
	}
	else if (!_strnicmp(ptr, ".ktx", 4))
	{
		int format_group;

		ret = KTX_LoadImage(filename, image);
		if (ret)
		{
			printf("Error reading file \"%s\"\n", filename);
			return -1;
		}

		format_group = System_GLFormatToFormatGroup(image->dst_format);

		if (format_group == -1)
		{
			printf("Unknown format %i 0x%08X\n", image->dst_format, image->dst_format);
			return -1;
		}

		if (format_group == FORMAT_GROUP_BCN)
			DDS_DecodeBCN(image);
		else if (format_group == FORMAT_GROUP_ETC)
			ETC_DecodeETCN(image);

		return format_group;
	}
	else
	{
		int i;

		ilGenImages(1, &devil_image);
		ilBindImage(devil_image);

		result = ilLoadImage(in_wpath);

		if (!result)
		{
			ilDeleteImages(1, &devil_image);
			printf("Error loading file \"%ls\".\n", in_wpath);
			return -1;
		}
		ilBindImage(devil_image);
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);

		if (width * height > 67108864)
		{
			ilDeleteImages(1, &devil_image);
			printf("Image too large: max size of image must be less than 67.1 MP (image is %.2f MP)\n", (width * height) / 1000000.0);
			return -1;
		}

		rgba = malloc((uint64_t)width * (uint64_t)height * 4 * sizeof(float));
		uint_result = ilCopyPixels(0, 0, 0, width, height, 1, IL_RGBA, IL_FLOAT, rgba);

		for (i = 0; i < ((uint64_t)width * (uint64_t)height * 4); i++)
			rgba[i] *= 255.0f;

		*image = Image_Create(width, height, 0, 0);
		free(image->src_rgba_0_255[0]);
		image->src_rgba_0_255[0] = rgba;

		return FORMAT_GROUP_RGB;
	}
}

char *System_ExtractOutDirFromPath(char *path)
{
	if (g_output_dir && strlen(g_output_dir))
	{
		char *dir = malloc(strlen(g_output_dir) + 2);
		strcpy_s(dir, strlen(g_output_dir) + 1, g_output_dir);
		dir[strlen(g_output_dir)] = '\\';
		dir[strlen(g_output_dir) + 1] = 0;
		return dir;
	}
	else
	{
		char *ptr[2];
		char *dir = malloc(strlen(path) + 2);
		int len;
		strcpy_s(dir, strlen(path) + 1, path);
		while(ptr[1] = strchr(dir, '/'))
		{
			*(strchr(dir, '/')) = '\\';
		}
		ptr[0] = strrchr(dir, '\\');

		if (!ptr[0])
			dir[0] = 0;
		else
			ptr[0][0] = 0;

		len = (int)strlen(dir);

		if (len && dir[len - 1] != '\\')
		{
			dir[len] = '\\';
			dir[len + 1] = 0;
		}

		return dir;
	}
}
char *System_StripExtension(char *path)
{
	char *ptr;
	char *new_path = malloc(strlen(path) + 1);
	strcpy_s(new_path, strlen(path) + 1, path);
	ptr = strrchr(new_path, '.');

	if (ptr)
		*ptr = 0;

	return new_path;
}
char *System_ExtractNameFromPath(char *path)
{
	char *ptr[2];
	char *name;
	ptr[0] = strrchr(path, '\\');
	ptr[1] = strrchr(path, '/');

	ptr[0] = max(ptr[0], ptr[1]);

	if (ptr[0])
		ptr[0]++;
	else
		ptr[0] = path;

	name = malloc(strlen(ptr[0]) + 1);
	strcpy_s(name, strlen(ptr[0]) + 1, ptr[0]);

	return name;
}
void System_PartitionOutPath(char *path, char **dir, char **name, char **extension)
{
	char *temp = System_ExtractNameFromPath(path);
	char *ptr = strrchr(path, '.');

	if (ptr)
	{
		*extension = malloc(strlen(ptr + 1) + 1);
		strcpy_s(*extension, strlen(ptr + 1) + 1, ptr + 1);
	}
	else
	{
		*extension = malloc(1);
		(*extension)[0] = 0;
	}
	free(temp);
	temp = System_ExtractNameFromPath(path);

	*dir = System_ExtractOutDirFromPath(path);
	*name = System_StripExtension(temp);

	free(temp);
}

int System_CompareFiles(int num_files, char **filename)
{
	image_t image[3];
	image_t image_ssim[4];
	int ret;
	int width;
	int height;
	int i;
	double ssim_range[4][2];
	int is_decoded = 0;
	int dimensions[2];
	wchar_t output_name[2048];

	ilInit();
	iluInit();
	ilutInit();
	ilutRenderer(ILUT_WIN32);

	ilEnable(IL_ORIGIN_SET);
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);

	for (i = 0; i < num_files + 1; i++)
	{
		ret = System_LoadFile(&image[i], filename[i]);
		if (ret < 0)
		{
			printf("Error reading file \"%s\"\n", filename[i]);
			return -1;
		}
		if (!g_minimal)
			printf("Loaded \"%s\" format %s\n", filename[i], System_GLFormatAsString(image[i].dst_format));
	}

	
	dimensions[0] = image[0].src_width[0];
	dimensions[1] = image[0].src_height[0];

	for (i = 1; i < num_files + 1; i++)

	if (image[i].src_width[0] != dimensions[0] || image[i].src_height[0] != dimensions[1])
	{
		printf("Images have different resolutions (%i x %i and %i x %i)\n", image[i].src_width[0], image[i].src_height[0], dimensions[0], dimensions[1]);
		return -1;
	}

	width = dimensions[0];
	height = dimensions[1];

	if (num_files == 2)
	{
		double higher;
		double lower;
		double same;
		double upper_error;
		double lower_error;
		Image_CompareBlocks(g_minimal, &image[0], &image[1], &image[2], &higher, &same, &lower, &upper_error, &lower_error);
		printf("Higher quality blocks in image 0: %3.2f%% (%.2f square error)\n", 100.0 * higher, upper_error);
		printf("Same quality blocks:              %3.2f%%\n", 100.0 * same);
		printf("Lower quality blocks in image 0:  %3.2f%% (%.2f square error)\n", 100.0 * lower, lower_error);
	}

	if (num_files == 2)
	{
		double psnr_rgb[2];
		double psnr_alpha[2];

		Image_PSNR(&psnr_rgb[0], &psnr_alpha[0], &image[0], &image[1]);
		Image_PSNR(&psnr_rgb[1], &psnr_alpha[1], &image[0], &image[2]);
		printf("PSNR RGB/Alpha for \"%s\" vs \"%s\":\n\t%.5f dB / %.5f dB,  %.5f dB / %.5f dB (%.5f dB / %.5f dB)\n", filename[1], filename[2], psnr_rgb[0], psnr_alpha[0], psnr_rgb[1], psnr_alpha[1], psnr_rgb[0] - psnr_rgb[1], psnr_alpha[0] - psnr_alpha[1]);
	}
	else
		for (i = 1; i < num_files + 1; i++)
		{
			double psnr_rgb;
			double psnr_alpha;
			Image_PSNR(&psnr_rgb, &psnr_alpha, &image[0], &image[i]);
			printf("PSNR RGB/Alpha for \"%s\": %.5f dB / %.5f dB\n", filename[i], psnr_rgb, psnr_alpha);
		}

	if (g_decode)
	{
		int j;
		char *dir;
		float *temp_data = malloc(width * height * 4 * sizeof(float));
		ILuint t;
		char *name;
		char *extension;

		for (i = 0; i < num_files + 1; i++)
		{
			System_PartitionOutPath(filename[i], &dir, &name, &extension);

			swprintf(output_name, 2048, L"%SCompare_input%i_%S_Decoded.tga", dir, i, name);
			if (!g_minimal)
				printf("Saving file \"%ls\"\n", output_name);
			ilGenImages(1, &t);
			ilBindImage(t);
			for (j = 0; j < height; j++)
				memcpy(&temp_data[(height - j - 1) * width * 4], &image[i].src_rgba_0_255[0][j * width * 4], width * 4 * sizeof(float));
			for (j = 0; j < width * height * 4; j++)
				temp_data[j] /= 255.0;
			ilTexImage(width, height, 1, 4, IL_RGBA, IL_FLOAT, temp_data);
			ilSaveImage(output_name);
		}

		free(temp_data);
	}

	if (g_output_ssim)
	{
		char *dir;
		float *temp_data = malloc(width * height * 4 * sizeof(float));
		ILuint t;
		char *name;
		char *extension;

		ilEnable(IL_FILE_OVERWRITE);

		for (i = 1; i < num_files + 1; i++)
		{
			int j;

			System_PartitionOutPath(filename[i], &dir, &name, &extension);

			Image_SSIM(g_ssim_literal, g_window, &image_ssim[0], &image_ssim[1], &image_ssim[2], &image_ssim[3], &image[0], &image[i], ssim_range[0], ssim_range[1], ssim_range[2], ssim_range[3], g_ssim_lower, g_ssim_upper);

			printf("SSIM LX range for \"%s\": %.10f - %.10f\n", filename[i], ssim_range[0][0], ssim_range[0][1]);
			printf("SSIM CX range for \"%s\": %.10f - %.10f\n", filename[i], ssim_range[1][0], ssim_range[1][1]);
			printf("SSIM SX range for \"%s\": %.10f - %.10f\n", filename[i], ssim_range[2][0], ssim_range[2][1]);
			printf("SSIM range for \"%s\": %.10f - %.10f\n", filename[i], ssim_range[3][0], ssim_range[3][1]);

			Image_SSIM_MapColours(&image_ssim[0]);
			Image_SSIM_MapColours(&image_ssim[1]);
			Image_SSIM_MapColours(&image_ssim[2]);
			Image_SSIM_MapColours(&image_ssim[3]);

			swprintf(output_name, 2048, L"%SCompare_input%i_%S_SSIM_LX.tga", dir, i, name);
			if (!g_minimal)
				printf("Saving file \"%ls\"\n", output_name);
			ilGenImages(1, &t);
			ilBindImage(t);
			for (j = 0; j < height; j++)
				memcpy(&temp_data[(height - j - 1) * width * 4], &image_ssim[0].src_rgba_0_255[0][j * width * 4], width * 4 * sizeof(float));
			ilTexImage(width, height, 1, 4, IL_RGBA, IL_FLOAT, temp_data);
			ilSaveImage(output_name);

			swprintf(output_name, 2048, L"%SCompare_input%i_%S_SSIM_CX.tga", dir, i, name);
			if (!g_minimal)
				printf("Saving file \"%ls\"\n", output_name);
			ilGenImages(1, &t);
			ilBindImage(t);
			for (j = 0; j < height; j++)
				memcpy(&temp_data[(height - j - 1) * width * 4], &image_ssim[1].src_rgba_0_255[0][j * width * 4], width * 4 * sizeof(float));
			ilTexImage(width, height, 1, 4, IL_RGBA, IL_FLOAT, temp_data);
			ilSaveImage(output_name);

			swprintf(output_name, 2048, L"%SCompare_input%i_%S_SSIM_SX.tga", dir, i, name);
			if (!g_minimal)
				printf("Saving file \"%ls\"\n", output_name);
			ilGenImages(1, &t);
			ilBindImage(t);
			for (j = 0; j < height; j++)
				memcpy(&temp_data[(height - j - 1) * width * 4], &image_ssim[2].src_rgba_0_255[0][j * width * 4], width * 4 * sizeof(float));
			ilTexImage(width, height, 1, 4, IL_RGBA, IL_FLOAT, temp_data);
			ilSaveImage(output_name);

			swprintf(output_name, 2048, L"%SCompare_input%i_%S_SSIM.tga", dir, i, name);
			if (!g_minimal)
				printf("Saving file \"%ls\"\n", output_name);
			ilGenImages(1, &t);
			ilBindImage(t);
			for (j = 0; j < height; j++)
				memcpy(&temp_data[(height - j - 1) * width * 4], &image_ssim[3].src_rgba_0_255[0][j * width * 4], width * 4 * sizeof(float));
			ilTexImage(width, height, 1, 4, IL_RGBA, IL_FLOAT, temp_data);
			ilSaveImage(output_name);

			free(dir);
			free(name);
			free(extension);
		}

		free(temp_data);
	}

	return 0;
}
int main(int argc, char **argvv)
{
	int t;
	int i;
	int error = 0;
	char *argv[2048];
	HWND consolehwnd = GetConsoleWindow();
	DWORD dwprocessid;

	for (i = 0; i < argc; i++)
		argv[i] = argvv[i];

	if (argc == 1)
	{
		System_PrintUsage();
		goto MAIN_EXIT;
	}

	t = System_ParseOptions(argc, argv);

	if (t)
	{
		System_PrintUsage();
		printf("**********************************\n");
		printf("Error parsing command line options\n");
		printf("**********************************\n");
		error = 1;
		goto MAIN_EXIT;
	}
	if (g_compare[0] == 0)
	{
		System_PrintUsage();
		printf("***************************\n");
		printf("No reference file specified\n");
		printf("***************************\n");
		error = 1;
		goto MAIN_EXIT;
	}
	if (g_num_files == 0)
	{
		System_PrintUsage();
		printf("****************************\n");
		printf("No comparison file specified\n");
		printf("****************************\n");
		error = 1;
		goto MAIN_EXIT;
	}

	error = System_CompareFiles(g_num_files, g_compare);

MAIN_EXIT:

	GetWindowThreadProcessId(consolehwnd, &dwprocessid);
	if (GetCurrentProcessId() == dwprocessid)
	{
		printf("\n\nPress any key to exit.\n");
		_getch();
	}

	return error;
}