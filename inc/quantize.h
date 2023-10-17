/***

	quantize.h

	Reduce the number of colors in a bitmap. Includes also bitmap-dithering routines.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __QUANTIZE_H__
#define __QUANTIZE_H__

enum dithertype {
	dither_none = 0,
	dither_floyd_steinberg = 1,
	dither_sierra = 2,
	dither_burkes = 3,
	dither_atkinson = 4,
	dither_random = 5,
	/* should be the value of the largest valid dither type */
	dither_max = 5,			
};

enum colorspace {
	colorspace_rgb = 0,
	colorspace_hsv = 1,
	colorspace_yiq = 2,
	colorspace_ycbcr = 3,
	/* should be the value of the largest valid colorspace */
	colorspace_max = 3,		
};

extern const char* colorspace_name(colorspace cs);

/*** Returns a new palettized bitmap with the desired number of colors representing the passed-in bitmap. ***/
/*** High quality, but will probably take a few seconds and it eats some RAM. ***/
/*** If initial_palette is non-NULL, those colors will be used as the starting colors in quantization. ***/
/*** If dither is non-zero, matches colors using specified dithering algorithm once the palette is constructed. ***/
/*** Color-matching is done in the space specified by matchspace. ***/
extern SBitmap* quantize_bmp_greedy(SBitmap* bmp, u32 desired_num_colors, SPalette* initial_palette, dithertype dither, colorspace matchspace);

/*** Lower quality but faster and lower RAM use version of the above. ***/
extern SBitmap* quantize_bmp_quick_and_dirty(SBitmap* bmp, u32 desired_num_colors, dithertype dither);

/*** Convert a color from RGB color space to the specified color space. ***/
extern RGBColor rgb_to_colorspace(RGBColor rgb, colorspace cspace);

/*** Convert a color from the specified color space to RGB. ***/
extern RGBColor colorspace_to_rgb(RGBColor rgb, colorspace cspace);

/*** As above, but convert a bitmap (in place) to or from the given colorspace. ***/
extern void bitmap_to_colorspace(SBitmap* bmp_in, colorspace cspace);
extern void bitmap_from_colorspace(SBitmap* bmp_in, colorspace cspace);

/*** Returns a count of unique colors that actually appear in the bitmap. ***/
extern u32 count_unique_colors_bmp(SBitmap *bmp);

/*** Returns an allocated SPalette containing every unique color in the image. ***/
extern SPalette* create_palette_bmp(SBitmap* bmp);

/* Create a representation of the image bmp_in using the palette pal_in, using Floyd-Steinberg dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
extern SBitmap* floyd_steinberg_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out);

/* Create a representation of the image bmp_in using the palette pal_in, using Sierra dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
extern SBitmap* sierra_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out);

/* Create a representation of the image bmp_in using the palette pal_in, using Burkes dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
extern SBitmap* burkes_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out);

/* Create a representation of the image bmp_in using the palette pal_in, using Atkinson dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
extern SBitmap* atkinson_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out);

/* Create a representation of the image bmp_in using the palette pal_in, using random dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. In certain cases, like
	a large color gradient, random dithering gives better results than error-diffusion methods. */
extern SBitmap* random_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out);

#endif  // __QUANTIZE_H__
/* end __quantize.h */
