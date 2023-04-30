typedef struct image_s
{
	int		dst_format;
	int		dst_block_size;
	int		num_mipmaps;
	int		*src_width;
	int		*src_height;
	float	**src_rgba_0_255; // num_mipmaps * src_width[i] * src_height[i]
	uint8_t	**dst_encoded;
	int		num_channels;
}image_t;

image_t Image_Create(int width, int height, int mipmaps, int encoded_format);
void Image_Destroy(image_t *image);
void Image_PSNR(double *psnr_rgb, double *psnr_alpha, image_t *ref_image, image_t *compare_image);
void Image_SSIM(int is_literal, int window, image_t *ssim_lx, image_t *ssim_cx, image_t *ssim_sx, image_t *ssim, image_t *ref_image, image_t *compare_image, double *lx_range, double *cx_range, double *sx_range, double *ssim_range, double view_range_min, double view_range_max);
void Image_SSIM_MapColours(image_t *ssim);
double Image_CompareBlocks(int options, image_t *reference, image_t *image0, image_t *image1, double *higher_quality, double *same_quality, double *lower_quality, double *error_upper, double *error_lower);