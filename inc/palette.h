/***

	palette.h

	Functions for manipulating palettes, palettizing images, etc.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __PALETTE_H
#define	__PALETTE_H

#define	PALETTE_INVALID	(0xffffffff)

/* Make a 256-level grayscale palette. */
extern void fill_palette_grayscale(SPalette* pal);

/* Convert the passed-in palette to grayscale. */
extern void make_palette_grayscale(SPalette* pal);

/* Fill with a "safety palette" of 216 evenly-spaced RGB colors. An OK default palette. */
extern void fill_safety_palette(SPalette* pal);

/* Create the default 256-color VGA palette. */
extern void fill_vga_palette(SPalette* pal);

/* Create all 64 colors of the EGA palette. */
extern void fill_ega_palette(SPalette* pal);

/* Fill with the Apple ][ 16 color palette (actually 15 different colors, since gray was used twice.) */
extern void fill_apple_ii_palette(SPalette* pal);

/* Filter the palette. Each color component is bitwise-ANDed with its mask. Can be used to convert a grayscale image to all-red, etc. */
extern void rgb_filter_palette(SPalette* pal, u32 red_mask, u32 green_mask, u32 blue_mask);

/* Create a palettized image with at most 256 colors from the source bitmap. Currently fails
	if the src_bmp contains more than 256 different RGB colors. */
extern SBitmap* create_palettized_image(SBitmap* src_bmp);

/*** Returns an allocated copy of the specified palette. ***/
extern SPalette* copy_palette(SPalette* pal);

/*** Rotates the palette nentries to the left. Useful for palettized color animations. ***/
extern void rotate_palette(SPalette* pal, u32 nentries);

/*** Create a new palette that can contain up to max_colors colors. ***/
extern SPalette* new_palette(u32 max_colors);

/*** Sort the palette by color intensity ***/
extern void sort_palette(SPalette *pal);

/*** Make a 256-hue gradient palette, given saturation and value ***/
extern void fill_palette_hue(SPalette* pal, u32 saturation, u32 value);

/*** Make a 256-saturation gradient palette, given hue and value ***/
extern void fill_palette_saturation(SPalette* pal, u32 hue, u32 value);

/*** Make a 256-color palette based on a given color by adding random offsets to the colors. ***/
extern void fill_palette_color_random_perturbation(SPalette* pal, RGBColor c);

/*** Make a 218-color palette by evenly stepping through HSV color space. ***/
extern void fill_palette_hsv_stepping(SPalette* pal);

/*** Fill the nc colors starting at index i in the passed palette with random colors. ***/
extern void fill_palette_random(SPalette* pal, u32 nc, u32 i);

/* Make a 1-bit black & white palette. */
extern void fill_palette_bw(SPalette* pal);

/*** Returns the palette index that best matches the specified color. ***/
extern u32 palette_index_from_rgb(SPalette* pal, RGBColor c);

/*** Returns the two palette indices that best match the specified color, with their error. ***/
extern void palette_index_from_rgb_2(SPalette* pal, RGBColor c, u32* c1, u32* c2, u32* e1, u32* e2);

/*** Perturb the passed-in color by a small amount. ***/
extern void perturb_color_randomly(RGBColor *c);

#endif  // PALETTE_H
// end palette.h
