/***

	palette.cpp

	Functions for creating and manipulating palettes, palettizing images, etc.

	Copyright (c) 2014-2022, Chris M. Street

***/
#include "libcodehappy.h"

/* Make a 256-level grayscale palette. */
void fill_palette_grayscale(SPalette* pal) {
	int e;

	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;

	pal->ncolors = 256;

	for (e = 0; e < 256; ++e)
		pal->clrs[e] = RGB_NO_CHECK(e, e, e);
}

/* Make a 1-bit black & white palette. */
void fill_palette_bw(SPalette* pal) {
	NOT_NULL_OR_RETURN_VOID(pal);
	
	pal->ncolors = 2;
	pal->clrs[0] = C_BLACK;
	pal->clrs[1] = C_WHITE;
}

/* Make a 256-hue gradient palette, given saturation and value */
void fill_palette_hue(SPalette* pal, u32 saturation, u32 value) {
	u32 hue;

	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);

	pal->ncolors = 256;
	for (hue = 0; hue < 256; ++hue) {
		int r, g, b;
		HSV_RGB(hue, saturation, value, &r, &g, &b);
		pal->clrs[hue] = RGB_NO_CHECK(r, g, b);
	}
}

/* Make a 256-saturation gradient palette, given hue and value */
void fill_palette_saturation(SPalette* pal, u32 hue, u32 value) {
	u32 saturation;

	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);

	pal->ncolors = 256;
	for (saturation = 0; saturation < 256; ++saturation) {
		int r, g, b;
		HSV_RGB(hue, saturation, value, &r, &g, &b);
		pal->clrs[hue] = RGB_NO_CHECK(r, g, b);
	}
}

void perturb_color_randomly(RGBColor *c) {
	u32 r, g, b;

	r = RGB_RED(*c);
	g = RGB_GREEN(*c);
	b = RGB_BLUE(*c);

	r += (randint() % 9) - 4;
	g += (randint() % 9) - 4;
	b += (randint() % 9) - 4;
	
	r = CLAMP(r, 0, 255);
	g = CLAMP(g, 0, 255);
	b = CLAMP(b, 0, 255);

	(*c) = RGB_NO_CHECK(r, g, b);
}

/*** Make a 256-color palette based on a given color by adding random offsets to the colors. ***/
void fill_palette_color_random_perturbation(SPalette* pal, RGBColor c) {
	int e;
	
	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);
	pal->ncolors = 256;
	pal->clrs[0] = c;
	for (e = 1; e < 256; ++e) {
		pal->clrs[e] = pal->clrs[e - 1];
		perturb_color_randomly(&pal->clrs[e]);
	}
}

/*** Make a 218-color palette by evenly stepping through HSV color space. ***/
void fill_palette_hsv_stepping(SPalette* pal) {
	int h, s, v;
	int c;
	
	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);
	pal->ncolors = 218;

	/* first two colors are black and white */
	pal->clrs[0] = C_BLACK;
	pal->clrs[1] = C_WHITE;
	c = 2;

	/* and the last 216 are created by evenly stepping through HSV space */
	for (v = 0; v < 256; v += 51)
		for (s = 0; s < 256; s += 51)
			for (h = 0; h < 256; h += 51) {
				int r, g, b;
				HSV_RGB(h, s, v, &r, &g, &b);
				pal->clrs[c++] = RGB_NO_CHECK(r, g, b);
			}
}

/*** Fill the nc colors starting at index i in the passed palette with random colors. ***/
void fill_palette_random(SPalette* pal, u32 nc, u32 i) {
	u32 e;
	
	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);

	for (e = 0; e < nc && i + e < pal->ncolors; ++e)
		pal->clrs[i + e] = RGB_NO_CHECK(randint() & 0xff, randint() & 0xff, randint() & 0xff);
}

/* Convert the passed-in palette to grayscale. */
void make_palette_grayscale(SPalette* pal) {
	u32 e;

	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;

	for (e = 0; e < pal->ncolors; ++e) {
		u32 gray = RGBColorGrayscaleLevel(pal->clrs[e]);
		pal->clrs[e] = RGB_NO_CHECK(gray, gray, gray);
	}
}

/* Filter the palette. Each color component is bitwise-ANDed with its mask. Can be used to convert a grayscale image to all-red, etc. */
void rgb_filter_palette(SPalette* pal, u32 red_mask, u32 green_mask, u32 blue_mask) {
	u32 e;

	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;
	
	for (e = 0; e < pal->ncolors; ++e) {
		u32 r, g, b;

		r = RGB_RED(pal->clrs[e]);
		g = RGB_GREEN(pal->clrs[e]);
		b = RGB_BLUE(pal->clrs[e]);

		r &= red_mask;
		g &= green_mask;
		b &= blue_mask;
		
		pal->clrs[e] = RGB_NO_CHECK(r, g, b);
	}
}

/* Create a "safety palette" ala Windows. First color is white, last color is black, colors 20 - 235 are filled
	with 216 (6^3) nice even stepping colors, first 20 and last 20 are reserved. */
void fill_safety_palette(SPalette* pal) {
	int i;
	u32 r, g, b;
	
	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;

	pal->ncolors = 256;

	// color 0 must be white, color 255 must be black
	pal->clrs[0] = RGB_NO_CHECK(255, 255, 255);
	pal->clrs[255] = RGB_NO_CHECK(0, 0, 0);

	// now fill with 216 safety colors, evenly situated in the color space
	i = 20;
	for (r = 0; r < 256; r += 51)
		for (g = 0; g < 256; g += 51)
			for (b = 0; b < 256; b += 51)
				pal->clrs[i++] = RGB_NO_CHECK(r, g, b);

	// Leave the other colors untouched -- they might have already been set up.
}

/* Create all 64 colors of the EGA palette. */
void fill_ega_palette(SPalette* pal) {
	u32 i;
	
	NOT_NULL_OR_RETURN_VOID(pal);
	NOT_NULL_OR_RETURN_VOID(pal->clrs);

	pal->ncolors = 64;

	for (i = 0; i < 64; ++i) {
		int r, g, b;
		r = 0;
		g = 0;
		b = 0;
		if (truth(i & 32))
			r = 85;
		if (truth(i & 16))
			g = 85;
		if (truth(i & 8))
			b = 85;
		if (truth(i & 4))
			r += 170;
		if (truth(i & 2))
			g += 170;
		if (truth(i & 1))
			b += 170;

		pal->clrs[i] = RGB_NO_CHECK(r, g, b);
	}
}

const RGBColor __ega_palette[16] =
{
	EGA_00_BLACK,
	EGA_01_DK_BLUE,
	EGA_02_DK_GREEN,
	EGA_03_CYAN,
	EGA_04_DK_RED,
	EGA_05_PURPLE,
	EGA_06_BROWN,
	EGA_07_WHITE,
	EGA_08_GRAY,
	EGA_09_BRT_BLUE,
	EGA_10_BRT_GREEN,
	EGA_11_BRT_CYAN,
	EGA_12_BRT_RED,
	EGA_13_BRT_PURPLE,
	EGA_14_YELLOW,
	EGA_15_BRT_WHITE
};
/* Create the default 256-color VGA palette. */
void fill_vga_palette(SPalette* pal) {
	int e;
	u32 i;
	const u32 gs1[] = {0x00, 0x14, 0x20, 0x2C, 0x38, 0x45, 0x51, 0x61,
					 0x71, 0x82, 0x92, 0xA2, 0xB6, 0xCB, 0xE3, 0xFF};
	const RGBColor rainbow[] =
		{
		RGB(0,0,0xff),
		RGB(0x41, 0, 0xff),
		RGB(0x7d, 0, 0xff),
		RGB(0xbe, 0, 0xff),
		RGB(0xff, 0, 0xff),
		RGB(0xff, 0, 0xbe),
		RGB(0xff, 0, 0x7d),
		RGB(0xff, 0, 0x41),
		RGB(0xff, 0, 0),
		RGB(0xff, 0x41, 0),
		RGB(0xff, 0x7d, 0),
		RGB(0xff, 0xbe, 0),
		RGB(0xff, 0xff, 0),
		RGB(0xbe, 0xff, 0),
		RGB(0x7d, 0xff, 0),
		RGB(0x41, 0xff, 0),
		RGB(0, 0xff, 0),
		RGB(0, 0xff, 0x41),
		RGB(0, 0xff, 0x7d),
		RGB(0, 0xff, 0xbe),
		RGB(0, 0xff, 0xff),
		RGB(0, 0xbe, 0xff),
		RGB(0, 0x7d, 0xff),
		RGB(0, 0x41, 0xff)
		};
	RGBColor mix;
	
	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;

	pal->ncolors = 256;

	// first 16 colors come from the default EGA palette
	for (i = 0, e = 0; e < 16; ++e)
		pal->clrs[i++] = __ega_palette[e];
	// next 16 are grayscale
	for (e = 0; e < 16; ++e)
		pal->clrs[i++] = RGB_NO_CHECK(gs1[e], gs1[e], gs1[e]);
	// next 24 are a nice rainbow
	for (e = 0; e < 24; ++e)
		pal->clrs[i++] = rainbow[e];
	// next 24 are 51% rainbow, 49% white
	for (e = 0; e < 24; ++e) {
		interpolate_rgbcolor(rainbow[e], C_WHITE, &mix, 510);
		pal->clrs[i++] = mix;
	}
	// next 24 are 28% rainbow, 72% white
	for (e = 0; e < 24; ++e) {
		interpolate_rgbcolor(rainbow[e], C_WHITE, &mix, 280);
		pal->clrs[i++] = mix;
	}
	// next 72 are a mix of 44% (colors 32-103), 56% black
	for (e = 0; e < 72; ++e) {
		interpolate_rgbcolor(pal->clrs[30 + e], C_BLACK, &mix, 440);
		pal->clrs[i++] = mix;
	}
	// next 72 are a mix of 25% (colors 32-103), 75% black
	for (e = 0; e < 72; ++e) {
		interpolate_rgbcolor(pal->clrs[30 + e], C_BLACK, &mix, 250);
		pal->clrs[i++] = mix;
	}
	// last 8 colors are black, because I guess the palette designer gave up at this point
	for (e = 0; e < 8; ++e)
		pal->clrs[i++] = C_BLACK;
	// done!
}

/*** Fill with the Apple ][ 16 color palette (actually 15 different colors, since gray was used twice.) ***/
void fill_apple_ii_palette(SPalette* pal) {
	if (unlikely(is_null(pal) || is_null(pal->clrs)))
		return;

	pal->ncolors = 16;

	pal->clrs[0] = APPLE_II_00_BLACK;
	pal->clrs[1] = APPLE_II_01_DEEP_RED;
	pal->clrs[2] = APPLE_II_02_DARK_BLUE;
	pal->clrs[3] = APPLE_II_03_PURPLE;
	pal->clrs[4] = APPLE_II_04_DARK_GREEN;
	pal->clrs[5] = APPLE_II_05_GRAY_1;
	pal->clrs[6] = APPLE_II_06_MEDIUM_BLUE;
	pal->clrs[7] = APPLE_II_07_LIGHT_BLUE;
	pal->clrs[8] = APPLE_II_08_BROWN;
	pal->clrs[9] = APPLE_II_09_ORANGE;
	pal->clrs[10] = APPLE_II_10_GRAY_2;
	pal->clrs[11] = APPLE_II_11_PINK;
	pal->clrs[12] = APPLE_II_12_LIGHT_GREEN;
	pal->clrs[13] = APPLE_II_13_YELLOW;
	pal->clrs[14] = APPLE_II_14_AQUAMARINE;
	pal->clrs[15] = APPLE_II_15_WHITE;
}

/*** Re-use the palettization code I wrote for __gif.c. ***/

#if 0
struct cp {
	int r;
	int g;
	int b;
	int count;
};
#endif

extern void construct_coltable(SBitmap* bmp);

// to avoid tripping up syntax parsing in IDEs
#define	_XX_	darray(cp)
extern _XX_ coltable;
#define	coltablei(i)	darray_item(coltable, i)

/* Create a palettized image with at most 256 colors from the source bitmap. Currently fails
	if the src_bmp contains more than 256 different RGB colors. */
// TODO: auto-quantize the image if it has too many colors?
SBitmap* create_palettized_image(SBitmap* src_bmp) {
	int e;
	SBitmap* out_bmp;
	int x, y;

	// first, construct a palette composed of all unique colors in the image
	construct_coltable(src_bmp);

	if (darray_size(coltable) > 256)
		return(NULL);

	out_bmp = new SBitmap(src_bmp->width(), src_bmp->height(), BITMAP_PALETTE);
	if (unlikely(is_null(out_bmp)))
		return(NULL);
	if (unlikely(is_null(out_bmp->palette()) || is_null(out_bmp->palette()->clrs))) {
		delete out_bmp;
		return(NULL);
	}

	// now copy that data to our new bitmap
	for (e = 0; e < darray_size(coltable); ++e)
		out_bmp->palette()->clrs[e] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
	out_bmp->palette()->ncolors = e;

	// and create the palettized pixel data.
	for (y = 0; y < out_bmp->height(); ++y) {
		for (x = 0; x < out_bmp->width(); ++x) {
			RGBColor cc;
			cc = src_bmp->get_pixel(x, y);
			// putpixelbmp() will auto-match the RGB color to our palette.
			out_bmp->put_pixel(x, y, cc);
		}
	}

	return(out_bmp);
}

/*** Rotates the palette nentries to the left. Useful for palettized color animations. ***/
void rotate_palette(SPalette* pal, u32 nentries) {
	RGBColor cp[256];
	int e = 0;

	NOT_NULL_OR_RETURN_VOID(pal);
	if (pal->ncolors < 2)
		return;
	if (nentries >= pal->ncolors)
		nentries %= pal->ncolors;
	if (nentries > 256)
		return;	// ?
	if (iszero(nentries))
		return;

	OUR_MEMCPY(cp, pal->clrs, sizeof(RGBColor) * nentries);
	forever {
		if (e + nentries >= pal->ncolors)
			break;
		pal->clrs[e] = pal->clrs[e + nentries];
		++e;
	}
	OUR_MEMCPY(&pal->clrs[e], cp, sizeof(RGBColor) * nentries);
}

/*** Returns an allocated copy of the specified palette. ***/
SPalette* copy_palette(SPalette* pal) {
	SPalette* palret;
	u32 nalloc;
	
	palret = NEW(SPalette);
	NOT_NULL_OR_RETURN(palret, NULL);
	palret->ncolors = pal->ncolors;
	nalloc = max_int(256, pal->ncolors);
	palret->clrs = NEW_ARRAY(RGBColor, nalloc);
	if (unlikely(is_null(palret->clrs)))
		{
		delete palret;
		return(NULL);
		}
	OUR_MEMCPY(palret->clrs, pal->clrs, pal->ncolors * sizeof(RGBColor));
	return(palret);
}

/*** Create a new palette that can contain up to max_colors colors. ***/
SPalette* new_palette(u32 max_colors) {
	SPalette* palret;

	if (max_colors < 1)
		return(NULL);

	palret = NEW(SPalette);
	NOT_NULL_OR_RETURN(palret, NULL);
	palret->ncolors = max_colors;
	palret->clrs = NEW_ARRAY(RGBColor, max_colors);
	if (is_null(palret->clrs)) {
		delete palret;
		return(NULL);
	}

	return(palret);
}

static int __rgb_comp(const void* v1, const void* v2) {
	RGBColor c1 = *(RGBColor*)v1;
	RGBColor c2 = *(RGBColor*)v2;
	u32 d1, d2;

	d1 = distance_squared_3d(0, 0, 0, RGB_RED(c1), RGB_GREEN(c1), RGB_BLUE(c1));
	d2 = distance_squared_3d(0, 0, 0, RGB_RED(c2), RGB_GREEN(c2), RGB_BLUE(c2));

	return(d1 - d2);
}

/*** Sort the palette by color intensity ***/
void sort_palette(SPalette *pal) {
	qsort(pal->clrs, pal->ncolors, sizeof(pal->clrs[0]), __rgb_comp);
}

u32 palette_index_from_rgb(SPalette* pal, RGBColor c) {
	u32 best_i, best_dist;
	u32 e;
	u32 dist;
	const i32 rr = RGB_RED(c);
	const i32 gg = RGB_GREEN(c);
	const i32 bb = RGB_BLUE(c);
	
	if (unlikely(is_null(pal) || is_null(pal->clrs) || pal->ncolors == 0))
		return(PALETTE_INVALID);
	
	best_i = 0;
	best_dist = 0xFFFFFFFFUL;

	for (e = 0; e < pal->ncolors; ++e) {
		i32 r, g, b;

		r = RGB_RED(pal->clrs[e]);
		g = RGB_GREEN(pal->clrs[e]);
		b = RGB_BLUE(pal->clrs[e]);
		
		dist = distance_squared_3d(rr, gg, bb, r, g, b);
		if (dist < best_dist) {
			if (0 == dist)
				return(e);	/* can't do better */
			best_i = e;
			best_dist = dist;
		}
	}

	return(best_i);
}

/*** Returns the two palette indices that best match the specified color, with their error. ***/
void palette_index_from_rgb_2(SPalette* pal, RGBColor c, u32* c1, u32* c2, u32* e1, u32* e2) {
	u32 best_i[2], best_dist[2];
	u32 e;
	u32 dist;
	const i32 rr = RGB_RED(c);
	const i32 gg = RGB_GREEN(c);
	const i32 bb = RGB_BLUE(c);
	
	if (unlikely(is_null(pal) || is_null(pal->clrs) || pal->ncolors == 0))
		return;
	
	best_i[0] = 0;
	best_dist[0] = 0xFFFFFFFFUL;
	best_i[1] = 0;
	best_dist[1] = 0xFFFFFFFFUL;

	for (e = 0; e < pal->ncolors; ++e) {
		i32 r, g, b;

		r = RGB_RED(pal->clrs[e]);
		g = RGB_GREEN(pal->clrs[e]);
		b = RGB_BLUE(pal->clrs[e]);
		
		dist = distance_squared_3d(rr, gg, bb, r, g, b);
		if (dist < best_dist[0]) {
			best_dist[1] = best_dist[0];
			best_i[1] = best_i[0];
			best_dist[0] = dist;
			best_i[0] = e;
		} else if (dist < best_dist[1]) {
			best_dist[1] = dist;
			best_i[1] = e;
		}
	}

	if (not_null(c1))
		*c1 = best_i[0];
	if (not_null(e1))
		*e1 = isqrt(best_dist[0]);
	if (not_null(c2))
		*c2 = best_i[1];
	if (not_null(e2))
		*e2 = isqrt(best_dist[1]);

	return;
}

/*** end __palette.c ***/
