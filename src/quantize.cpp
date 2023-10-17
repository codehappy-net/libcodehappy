/***

	quantize.cpp

	Reduce the number of colors in a bitmap.

	Operates using a greedy algorithm: first a population count is made on the bitmap, and
	the highest-population colors are selected for the palette. (How many are chosen depends
	on the user arguments.) The palette is filled out by repeatedly determining the remaining color with
	the maximum population count * distance from the palette (where distance from the
	palette means the minimum distance between that color and any color already in
	the palette.)

	Includes accessories like count unique colors/generate color palette for a bitmap, and
	deluxe dithering functions.
	
	High quality and reasonable speed.

	Copyright (c) 2014-2022 C. M. Street

***/

#include "libcodehappy.h"
#include <algorithm>

// TODO: population count can fail on images with more than COLHASH_SIZE (~1 million) distinct colors -- realloc colhash as needed for those

// TODO: separate population count code from GIF save code?

static bool __cp_comp(const cp& v1, const cp& v2) {
	return v1.count > v2.count;
}

// Helper functions to use during nonRGB-space color matching.

/*** Convert a color from RGB color space to the specified color space. ***/
RGBColor rgb_to_colorspace(RGBColor rgb, colorspace cspace) {
	int s1, s2, s3;
	switch (cspace)
		{
	case colorspace_rgb:
		return(rgb);
	case colorspace_hsv:
		RGB_HSV(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3);
		break;
	case colorspace_yiq:
		RGB_YIQ(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3);
		break;
	case colorspace_ycbcr:
		RGB_YCbCr_601(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3);
		break;
		}
	return RGB_NO_CHECK(s1, s2, s3);
}

/*** Convert a color from the specified color space to RGB. ***/
RGBColor colorspace_to_rgb(RGBColor rgb, colorspace cspace) {
	int s1, s2, s3;
	switch (cspace)
		{
	case colorspace_rgb:
		return(rgb);
	case colorspace_hsv:
		HSV_RGB(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3);
		break;
	case colorspace_yiq:
		YIQ_RGB(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3);
		break;
	case colorspace_ycbcr:
		YCbCr_601_RGB(RGB_RED(rgb), RGB_GREEN(rgb), RGB_BLUE(rgb), &s1, &s2, &s3, 0);
		break;
		}
	return RGB_NO_CHECK(s1, s2, s3);
}

void bitmap_to_colorspace(SBitmap* bmp_in, colorspace cspace) {
	uint c;
	int y, x;
	if (bmp_in->type() == BITMAP_PALETTE) {
		for (c = 0; c < bmp_in->palette()->ncolors; ++c)
			bmp_in->palette()->clrs[c] = rgb_to_colorspace(bmp_in->palette()->clrs[c], cspace);
		return;
	}

	for (y = 0; y < bmp_in->height(); ++y)
		for (x = 0; x < bmp_in->width(); ++x)
			bmp_in->put_pixel(x, y, rgb_to_colorspace(bmp_in->get_pixel(x, y), cspace));
}

void bitmap_from_colorspace(SBitmap* bmp_in, colorspace cspace) {
	if (bmp_in->type() == BITMAP_PALETTE) {
		uint c;
		for (c = 0; c < bmp_in->palette()->ncolors; ++c)
			bmp_in->palette()->clrs[c] = colorspace_to_rgb(bmp_in->palette()->clrs[c], cspace);
	} else {
		int y, x;
		for (y = 0; y < bmp_in->height(); ++y)
			for (x = 0; x < bmp_in->width(); ++x)
				bmp_in->put_pixel(x, y, colorspace_to_rgb(bmp_in->get_pixel(x, y), cspace));
	}
}

static const char* colorspace_names[] = {
	"RGB", "HSV", "YIQ", "YCbCr"
};

const char* colorspace_name(colorspace cs) {
	if (cs > colorspace_max)
		return "(null)";
	return colorspace_names[(u32)cs];
}

// #define this for a somewhat faster greedy quantization that eats tons of RAM.
#undef	FULL_DISTANCE_CACHE

/*** Returns a new palettized bitmap with the desired number of colors representing the passed-in bitmap. ***/
/*** If initial_palette is non-NULL, those colors will be used as the starting colors in quantization. ***/
/*** If dither is non-zero, matches colors using specified dithering algorithm once the palette is constructed. ***/
/*** Color-matching is done in the space specified by matchspace. ***/
SBitmap* quantize_bmp_greedy(SBitmap* bmp, u32 desired_num_colors, SPalette* initial_palette, dithertype dither, colorspace matchspace) {
	u32 ncp;
	SBitmap *bmpret;
	SBitmap *bmpsav = NULL;
	u32 e;
	u32 lv;
	int sz = -1;
#ifdef FULL_DISTANCE_CACHE
	u32 **distance_cache;
#else
	u32 *distance_cache;
#endif

	/* sanity checks on input */
	if (unlikely(is_null(bmp) || desired_num_colors == 0UL))
		return(NULL);

	/* matchspace is a no-op for monochrome/grayscale bitmaps. */
	if (bmp->type() == BITMAP_MONO || bmp->type() == BITMAP_GRAYSCALE)
		matchspace = colorspace_rgb;

	if (matchspace != colorspace_rgb) {
		bmpsav = bmp;
		bmp = bmpsav->copy();
		bitmap_to_colorspace(bmp, matchspace);
	}

	/* set up the return bitmap */
	// TODO: this won't work if we want more than 256 colors, since only 8bpp are saved in BITMAP_PALETTE type
	bmpret = new SBitmap(bmp->width(), bmp->height(), BITMAP_PALETTE);
	bmpret->clear();
	NOT_NULL_OR_RETURN(bmpret, NULL);
	if (desired_num_colors > 256) {
		delete [] bmpret->palette()->clrs;
		bmpret->palette()->clrs = NEW_ARRAY(RGBColor, desired_num_colors);
		if (unlikely(is_null(bmpret->palette()->clrs))) {
			delete bmpret;
			if (not_null(bmpsav))
				delete bmp;
			return(NULL);
		}
	}
	bmpret->palette()->ncolors = desired_num_colors;
	
	/* First, generate the population count. */
	construct_coltable(bmp);
	sz = coltable.size();

	/* If there are fewer (or an equal number of) colors in the image than in our palette, well this is easy. */
	if (darray_size(coltable) <= desired_num_colors) {
		for (e = 0; e < sz; ++e) {
			bmpret->palette()->clrs[e] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
		}
		bmpret->palette()->ncolors = darray_size(coltable);
		goto LCopyPixels;
	}

	/* Else we have work to do. Use initial_palette if it's present, else fill part of the palette with the most
		frequent colors by population count (ignoring colors that are just too close.) */
	// TODO: shouldn't we check initial_palette before filling the palette when there are few colors in the image? We want to use those colors.
	if (not_null(initial_palette)) {
		for (e = 0; e < initial_palette->ncolors && e < desired_num_colors; ++e)
			bmpret->palette()->clrs[e] = initial_palette->clrs[e];
	} else {
		u32 cc;
		
//		qsort(coltable.item, darray_size(coltable), sizeof(cp), __cp_comp);
		std::sort(coltable.begin(), coltable.end(), __cp_comp);
		ncp = desired_num_colors >> 2;
		if (ncp < 1)
			ncp = 1;
		if (ncp > desired_num_colors)
			ncp = desired_num_colors;
		if (ncp > 16)
			ncp = 16;
//		std::vector<cp>::iterator& it = coltable.begin();
		for (e = 0, cc = 0; cc < ncp && e < sz; ++e)
			{
			u32 f;
			u32 least_distance = 0xFFFFFFFFUL;
			const u32 min_min_distance = 4800ul;	/* +/-40 difference in each component = 4,800 distance-squared */
			for (f = 0; f < e; ++f) {
				u32 distance;
				if (coltablei(f).count >= 0)
					continue;
				distance = distance_squared_3d(coltablei(e).r, coltablei(e).g, coltablei(e).b,
												coltablei(f).r, coltablei(f).g, coltablei(f).b);
				if (distance < least_distance)
					least_distance = distance;				
			}
			if (least_distance < min_min_distance)
				// too close
				continue;
			bmpret->palette()->clrs[cc] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
			++cc;
			coltablei(e).count = -e;
		}
	}

	/*** Initialize the distance cache, to allow short-circuiting substantial number of calculations. ***/
#ifdef FULL_DISTANCE_CACHE
	distance_cache = NEW_ARRAY(u32 *, darray_size(coltable));
	if (unlikely(is_null(distance_cache)))
		{
LFree:
//		delete [] fitness_compute;
		delete [] distance_cache;
		delete bmpret;
		return(NULL);
		}
	for (e = 0; e < sz; ++e)
		{
		u32 f;
		distance_cache[e] = NEW_ARRAY(u32, desired_num_colors + 1);
		if (unlikely(is_null(distance_cache[e])))
			{
			for (f = 0; f < e; ++f)
				delete distance_cache[f];
			goto LFree;
			}
		zeromem(distance_cache[e], sizeof(u32) * desired_num_colors);
		distance_cache[e][desired_num_colors] = 0xFFFFFFFF;
		}
#else	// !FULL_DISTANCE_CACHE
	distance_cache = NEW_ARRAY(u32, darray_size(coltable));
	if (unlikely(is_null(distance_cache)))
		{
		delete bmpret;
		return(NULL);
		}
	for (e = 0; e < sz; ++e)
		distance_cache[e] = 0xFFFFFFFFUL;
#endif

	lv = ncp - 1;

	/* While the palette isn't full, find the next color by best fitness function. */
	while (ncp < desired_num_colors)
		{
		u64 fitness;
		u32 besti;

		besti = 0;
		fitness = 0;
		for (e = 0; e < sz; ++e)
			{
			u64 our_fitness;
			u32 min_distance;
			u32 f;
			
			if (coltablei(e).count <= 0)
				continue;	// already selected

#if 0
			if (fitness_compute[e] < fitness)
				break;
#endif

#ifdef	FULL_DISTANCE_CACHE
			if ((u64)(distance_cache[e][desired_num_colors]) * coltablei(e).count < fitness)
				continue;
#else
			if ((u64)(distance_cache[e]) * coltablei(e).count < fitness)
				continue;
#endif

			min_distance = 0xFFFFFFFFUL;
			for (f = 0; f <= lv && f < sz; ++f)
				{
				u32 distance;
				
				if (coltablei(f).count > 0)
					continue;

#ifdef FULL_DISTANCE_CACHE
				if (distance_cache[e][-coltablei(f).count] == 0)
					{
					distance_cache[e][-coltablei(f).count] = distance_squared_3d(coltablei(f).r, coltablei(f).g, coltablei(f).b, 
																coltablei(e).r, coltablei(e).g, coltablei(e).b);
					/* cache the minimum distance as well */
					if (distance_cache[e][-coltablei(f).count] < distance_cache[e][desired_num_colors])
						distance_cache[e][desired_num_colors] = distance_cache[e][-coltablei(f).count];
					}
				distance = distance_cache[e][-coltablei(f).count];
#else
				distance = distance_squared_3d(coltablei(f).r, coltablei(f).g, coltablei(f).b, 
												coltablei(e).r, coltablei(e).g, coltablei(e).b);
				if (distance < distance_cache[e])
					distance_cache[e] = distance;
#endif
				if (distance < min_distance)
					min_distance = distance;
			}

			our_fitness = coltablei(e).count * min_distance;
			if (our_fitness > fitness)
				{
				besti = e;
				fitness = our_fitness;
				}
			}

		if (besti > lv)
			lv = besti;

		bmpret->palette()->clrs[ncp] = RGB_NO_CHECK(coltablei(besti).r, coltablei(besti).g, coltablei(besti).b);
		coltablei(besti).count = -ncp;
		ncp++;
		}

#if 0
	delete [] fitness_compute;
#endif

#ifdef FULL_DISTANCE_CACHE
	for (e = 0; e < sz; ++e)
		delete distance_cache[e];
#endif
	delete [] distance_cache;

LCopyPixels:
	/* The palette has been generated. Set the pixels in the quantized bitmap. */
	switch (dither)
		{
	case dither_none:
		bmp->blit(0, 0, bmp->width() - 1, bmp->height() - 1, bmpret, 0, 0);
		break;
	case dither_floyd_steinberg:
		floyd_steinberg_dither_bmp(bmp, bmpret->palette(), bmpret);
		break;
	case dither_sierra:
		sierra_dither_bmp(bmp, bmpret->palette(), bmpret);
		break;
	case dither_burkes:
		burkes_dither_bmp(bmp, bmpret->palette(), bmpret);
		break;
	case dither_atkinson:
		atkinson_dither_bmp(bmp, bmpret->palette(), bmpret);
		break;
	case dither_random:
		random_dither_bmp(bmp, bmpret->palette(), bmpret);
		break;
		}

	if (matchspace != colorspace_rgb)
		bitmap_from_colorspace(bmpret, matchspace);
	if (not_null(bmpsav))
		delete bmp;

	return(bmpret);
}


/*** Lower quality but faster and low-RAM version of the above. ***/
SBitmap* quantize_bmp_quick_and_dirty(SBitmap* bmp, u32 desired_num_colors, dithertype dither)
{
		SBitmap *bmpret;
		u32 e;
	
		/* sanity checks on input */
		if (unlikely(is_null(bmp) || desired_num_colors == 0UL))
			return(NULL);
	
		/* set up the return bitmap */
		bmpret = new SBitmap(bmp->width(), bmp->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmpret, NULL);
		if (desired_num_colors > 256) {
			delete [] bmpret->palette()->clrs;
			bmpret->palette()->clrs = NEW_ARRAY(RGBColor, desired_num_colors);
			if (unlikely(is_null(bmpret->palette()->clrs))) {
				delete bmpret;
				return(NULL);
			}
		}
		bmpret->palette()->ncolors = desired_num_colors;
		
		/* First, generate the population count. */
		construct_coltable(bmp);
	
		/* If there are fewer (or an equal number of) colors in the image than in our palette, well this is easy. */
		if (darray_size(coltable) <= desired_num_colors) {
			for (e = 0; e < darray_size(coltable); ++e)
				bmpret->palette()->clrs[e] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
			goto LCopyPixels;
		}

		/* Just copy the desired_num_colors top colors to the palette. */
		for (e = 0; e < desired_num_colors; ++e)
			bmpret->palette()->clrs[e] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
	
LCopyPixels:
		/* The palette has been generated. Set the pixels in the quantized bitmap. */
		switch (dither) {
		case dither_none:
			bmp->blit(0, 0, bmp->width() - 1, bmp->height() - 1, bmpret, 0, 0);
			break;
		case dither_floyd_steinberg:
			floyd_steinberg_dither_bmp(bmp, bmpret->palette(), bmpret);
			break;
		case dither_sierra:
			sierra_dither_bmp(bmp, bmpret->palette(), bmpret);
			break;
		case dither_burkes:
			burkes_dither_bmp(bmp, bmpret->palette(), bmpret);
			break;
		case dither_atkinson:
			atkinson_dither_bmp(bmp, bmpret->palette(), bmpret);
			break;
		case dither_random:
			random_dither_bmp(bmp, bmpret->palette(), bmpret);
			break;
		}
	
		return(bmpret);
}

/*** Returns a count of unique colors that actually appear in the bitmap. ***/
u32 count_unique_colors_bmp(SBitmap *bmp) {
	construct_coltable(bmp);
	return((u32)darray_size(coltable));
}

/*** Returns an allocated SPalette containing every unique color in the image. ***/
SPalette* create_palette_bmp(SBitmap* bmp) {
	SPalette* palret;
	int e;
	
	// check for the easy case first
	if (bmp->type() == BITMAP_PALETTE)
		return(copy_palette(bmp->palette()));

	// create the palette from the image.
	construct_coltable(bmp);
	palret = NEW(SPalette);
	NOT_NULL_OR_RETURN(palret, NULL);
	palret->ncolors = (u32)darray_size(coltable);
	palret->clrs = NEW_ARRAY(RGBColor, palret->ncolors);
	if (unlikely(is_null(palret->clrs))) {
		delete palret;
		return(NULL);
	}

	for (e = 0; e < palret->ncolors; ++e) {
		palret->clrs[e] = RGB_NO_CHECK(coltablei(e).r, coltablei(e).g, coltablei(e).b);
	}

	return(palret);
}

// TODO: more different dithering algorithms?

#define	BIDIRECTIONAL_DITHERING

#define	error_diffuse(xd, yd, mul, totmul)		\
			if (pixel_ok(bmp_in, x + (xd), y + (yd))) \
				{ \
				PIX_Error(x + (xd), y + (yd), 0) += (error_r * (mul)); \
				PIX_Error(x + (xd), y + (yd), 1) += (error_g * (mul)); \
				PIX_Error(x + (xd), y + (yd), 2) += (error_b * (mul)); \
				}


/*** For images with a small number of colors, we modify the Floyd-Steinberg, Sierra, and Burkes dithering algorithms
	to do error reduction, like Atkison dithering. Atkinson only adds 3/4 of the total quantization error
	to neightboring pixels; for a small number of colors, the other dithering algorithms do the same. Generally,
	this helps to avoid streaking or "worm pattern" artifacts. When quantizing to at least 64 colors, we turn
	error reduction off for higher accuracy. ***/
#define	USE_ERROR_REDUCTION(pal)	((pal)->ncolors < 64)

/* Create a representation of the image bmp_in using the palette pal_in, using Floyd-Steinberg dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
SBitmap* floyd_steinberg_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out) {
	i32* errors = NULL;
	bool allocbmp = is_null(bmp_out);
	i32 y, x;

	/***
		In this version of the Floyd-Steinberg algorithm, we alternate our x sweep direction each row
		from left-to-right to right-to-left and back. The rationale is that, if we always sweep left-to-
		right, there may be visible artifacts in which the color matching errors spread down and
		to the right. If we alternate sweep directions, these artifacts may be less noticeable.
		Improving the quality of the output is always nice.
	***/
	
	if (is_null(bmp_out)) {
		bmp_out = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmp_out, NULL);
		bmp_out->set_palette(pal_in);
		if (is_null(bmp_out->palette()))
			goto LFree;
	}

	/* we only need to allocate two rows to contain the errors */
#define	PIX_Error(x_, y_, z_)		errors[ARRAY_3D_INDEX((x_), ((y_) - y), (z_), bmp_in->width(), 2)]
	errors = NEW_ARRAY_3D(i32, bmp_in->width(), 2, 3);
	if (unlikely(is_null(errors))) {
LFree:
		delete [] errors;
		if (allocbmp)
			delete bmp_out;
		return(NULL);
	}

	zeromem(errors, sizeof(i32) * bmp_in->width() * 2 * 3);

	for (y = 0; y < bmp_in->height(); ++y) {
		i32 match_r, match_g, match_b;
		i32 error_r, error_g, error_b;
		i32 r, g, b;
		u32 idx;
		for (x = 0; x < bmp_in->width(); ++x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/16ths of component delta)
			match_r *= 16;
			match_g *= 16;
			match_b *= 16;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 64;
				match_g += 3 * PIX_Error(x, y, 1) / 64;
				match_b += 3 * PIX_Error(x, y, 2) / 64;
			} else {
				match_r += PIX_Error(x, y, 0) / 16;
				match_g += PIX_Error(x, y, 1) / 16;
				match_b += PIX_Error(x, y, 2) / 16;
			}

			r = (match_r + 7) / 16;
			g = (match_g + 7) / 16;
			b = (match_b + 7) / 16;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 16;
			g = RGB_GREEN(pal_in->clrs[idx]) * 16;
			b = RGB_BLUE(pal_in->clrs[idx]) * 16;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			// 7/16ths to the pixel on the right
			error_diffuse(1, 0, 7, 16);
			// 3/16ths to the pixel down and to the left
			error_diffuse(-1, 1, 3, 16);
			// 5/16ths to the pixel immediately below
			error_diffuse(0, 1, 5, 16);
			// and 1/16th to the pixel below and to the right
			error_diffuse(1, 1, 1, 16);
		}

		// update the errors array
		if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#ifdef BIDIRECTIONAL_DITHERING
		/* now go right-to-left */
		++y;
		if (y >= bmp_in->height())
			break;

		for (x = bmp_in->width() - 1; x >= 0; --x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/16ths of component delta)
			match_r *= 16;
			match_g *= 16;
			match_b *= 16;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 64;
				match_g += 3 * PIX_Error(x, y, 1) / 64;
				match_b += 3 * PIX_Error(x, y, 2) / 64;
			} else {
				match_r += PIX_Error(x, y, 0) / 16;
				match_g += PIX_Error(x, y, 1) / 16;
				match_b += PIX_Error(x, y, 2) / 16;
			}

			r = (match_r + 7) / 16;
			g = (match_g + 7) / 16;
			b = (match_b + 7) / 16;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 16;
			g = RGB_GREEN(pal_in->clrs[idx]) * 16;
			b = RGB_BLUE(pal_in->clrs[idx]) * 16;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			/*** these are mirrored versions of the error distributions in the above pass ***/
			error_diffuse(-1, 0, 7, 16);
			error_diffuse(1, 1, 3, 16);
			error_diffuse(0, 1, 5, 16);
			error_diffuse(-1, 1, 1, 16);
		}

		if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#endif  // BIDIRECTIONAL_DITHERING
	}

#undef	PIX_Error
	delete [] errors;
	return(bmp_out);
}

/* Create a representation of the image bmp_in using the palette pal_in, using Sierra dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
SBitmap* sierra_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out) {
	i32* errors = NULL;
	bool allocbmp = is_null(bmp_out);
	i32 y, x;

	if (is_null(bmp_out)) {
		bmp_out = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmp_out, NULL);
		bmp_out->set_palette(pal_in);
		if (is_null(bmp_out->palette()))
			goto LFree;
	}

	/* we only need to allocate three rows to contain the errors */
#define	PIX_Error(x_, y_, z_)		errors[ARRAY_3D_INDEX((x_), ((y_) - y), (z_), bmp_in->width(), 3)]
	errors = NEW_ARRAY_3D(i32, bmp_in->width(), 3, 3);
	if (unlikely(is_null(errors))) {
LFree:
		delete [] errors;
		if (allocbmp)
			delete bmp_out;
		return(NULL);
	}

	zeromem(errors, sizeof(i32) * bmp_in->width() * 3 * 3);

	for (y = 0; y < bmp_in->height(); ++y) {
		i32 match_r, match_g, match_b;
		i32 error_r, error_g, error_b;
		i32 r, g, b;
		u32 idx;
		for (x = 0; x < bmp_in->width(); ++x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/32nds of component delta)
			match_r *= 32;
			match_g *= 32;
			match_b *= 32;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 128;
				match_g += 3 * PIX_Error(x, y, 1) / 128;
				match_b += 3 * PIX_Error(x, y, 2) / 128;
			} else {
				match_r += PIX_Error(x, y, 0) / 32;
				match_g += PIX_Error(x, y, 1) / 32;
				match_b += PIX_Error(x, y, 2) / 32;
			}

			r = (match_r + 15) / 32;
			g = (match_g + 15) / 32;
			b = (match_b + 15) / 32;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 32;
			g = RGB_GREEN(pal_in->clrs[idx]) * 32;
			b = RGB_BLUE(pal_in->clrs[idx]) * 32;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			// 5/32nds to the pixel on the right
			error_diffuse(1, 0, 5, 32);
			// 3/32nds two pixels to the right
			error_diffuse(2, 0, 3, 32);
			// 2/32nds two pixels to the left and one down
			error_diffuse(-2, 1, 2, 32);
			// 4/32nds one pixel to the left and one down
			error_diffuse(-1, 1, 4, 32);
			// 5/32nds one pixel down
			error_diffuse(0, 1, 5, 32);
			// 4/32nds one pixel  down and to the right
			error_diffuse(1, 1, 4, 32);
			// 2/32nds two pixels right and one down
			error_diffuse(2, 1, 2, 32);
			// 2/32nds one pixel left and two down
			error_diffuse(-1, 2, 2, 32);
			// 3/32nds two pixels down
			error_diffuse(0, 2, 3, 32);
			// 2/32nds one pixel right and two down
			error_diffuse(1, 2, 2, 32);
		}

		// update the errors array
		if (y + 2 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = PIX_Error(x, y + 2, 0);
				PIX_Error(x, y + 1, 1) = PIX_Error(x, y + 2, 1);
				PIX_Error(x, y + 1, 2) = PIX_Error(x, y + 2, 2);
				PIX_Error(x, y + 2, 0) = 0;
				PIX_Error(x, y + 2, 1) = 0;
				PIX_Error(x, y + 2, 2) = 0;
			}
		else if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#ifdef BIDIRECTIONAL_DITHERING
		/* now go right-to-left */
		++y;
		if (y >= bmp_in->height())
			break;

		for (x = bmp_in->width() - 1; x >= 0; --x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/32nds of component delta)
			match_r *= 32;
			match_g *= 32;
			match_b *= 32;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 128;
				match_g += 3 * PIX_Error(x, y, 1) / 128;
				match_b += 3 * PIX_Error(x, y, 2) / 128;
			} else {
				match_r += PIX_Error(x, y, 0) / 32;
				match_g += PIX_Error(x, y, 1) / 32;
				match_b += PIX_Error(x, y, 2) / 32;
			}

			r = (match_r + 15) / 32;
			g = (match_g + 15) / 32;
			b = (match_b + 15) / 32;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 32;
			g = RGB_GREEN(pal_in->clrs[idx]) * 32;
			b = RGB_BLUE(pal_in->clrs[idx]) * 32;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			/*** these are mirrored versions of the error distributions in the above pass ***/
			error_diffuse(-1, 0, 5, 32);
			error_diffuse(-2, 0, 3, 32);
			error_diffuse(2, 1, 2, 32);
			error_diffuse(1, 1, 4, 32);
			error_diffuse(0, 1, 5, 32);
			error_diffuse(-1, 1, 4, 32);
			error_diffuse(-2, 1, 2, 32);
			error_diffuse(1, 2, 2, 32);
			error_diffuse(0, 2, 3, 32);
			error_diffuse(-1, 2, 2, 32);
		}

	// update the errors array
	if (y + 2 < bmp_in->height())
		for (x = 0; x < bmp_in->width(); ++x) {
			PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
			PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
			PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
			PIX_Error(x, y + 1, 0) = PIX_Error(x, y + 2, 0);
			PIX_Error(x, y + 1, 1) = PIX_Error(x, y + 2, 1);
			PIX_Error(x, y + 1, 2) = PIX_Error(x, y + 2, 2);
			PIX_Error(x, y + 2, 0) = 0;
			PIX_Error(x, y + 2, 1) = 0;
			PIX_Error(x, y + 2, 2) = 0;
		}
	else if (y + 1 < bmp_in->height())
		for (x = 0; x < bmp_in->width(); ++x) {
			PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
			PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
			PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
			PIX_Error(x, y + 1, 0) = 0;
			PIX_Error(x, y + 1, 1) = 0;
			PIX_Error(x, y + 1, 2) = 0;
		}
#endif  // BIDIRECTIONAL_DITHERING
	}

#undef	PIX_Error
	delete [] errors;
	return(bmp_out);
}

/* Create a representation of the image bmp_in using the palette pal_in, using Burkes dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
SBitmap* burkes_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out) {
	i32* errors = NULL;
	bool allocbmp = is_null(bmp_out);
	i32 y, x;
	
	if (is_null(bmp_out)) {
		bmp_out = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmp_out, NULL);
		bmp_out->set_palette(pal_in);
		if (is_null(bmp_out->palette()))
			goto LFree;
	}

	/* we only need to allocate two rows to contain the errors */
#define	PIX_Error(x_, y_, z_)		errors[ARRAY_3D_INDEX((x_), ((y_) - y), (z_), bmp_in->width(), 2)]
	errors = NEW_ARRAY_3D(i32, bmp_in->width(), 2, 3);
	if (unlikely(is_null(errors)))
		{
LFree:
		delete [] errors;
		if (allocbmp)
			delete bmp_out;
		return(NULL);
		}

	zeromem(errors, sizeof(i32) * bmp_in->width() * 2 * 3);

	for (y = 0; y < bmp_in->height(); ++y) {
		i32 match_r, match_g, match_b;
		i32 error_r, error_g, error_b;
		i32 r, g, b;
		u32 idx;
		for (x = 0; x < bmp_in->width(); ++x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error 
			match_r *= 32;
			match_g *= 32;
			match_b *= 32;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 128;
				match_g += 3 * PIX_Error(x, y, 1) / 128;
				match_b += 3 * PIX_Error(x, y, 2) / 128;
			} else {
				match_r += PIX_Error(x, y, 0) / 32;
				match_g += PIX_Error(x, y, 1) / 32;
				match_b += PIX_Error(x, y, 2) / 32;
			}

			r = (match_r + 15) / 32;
			g = (match_g + 15) / 32;
			b = (match_b + 15) / 32;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 32;
			g = RGB_GREEN(pal_in->clrs[idx]) * 32;
			b = RGB_BLUE(pal_in->clrs[idx]) * 32;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;

			error_diffuse(1, 0, 8, 32);
			error_diffuse(2, 0, 4, 32);
			error_diffuse(0, 1, 8, 32);
			error_diffuse(1, 1, 4, 32);
			error_diffuse(-1, 1, 4, 32);
			error_diffuse(2, 1, 2, 32);
			error_diffuse(-2, 1, 2, 32);
		}

		// update the errors array
		if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#ifdef BIDIRECTIONAL_DITHERING
		/* now go right-to-left */
		++y;
		if (y >= bmp_in->height())
			break;

		for (x = bmp_in->width() - 1; x >= 0; --x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error 
			match_r *= 32;
			match_g *= 32;
			match_b *= 32;
			if (USE_ERROR_REDUCTION(pal_in)) {
				match_r += 3 * PIX_Error(x, y, 0) / 128;
				match_g += 3 * PIX_Error(x, y, 1) / 128;
				match_b += 3 * PIX_Error(x, y, 2) / 128;
			} else {
				match_r += PIX_Error(x, y, 0) / 32;
				match_g += PIX_Error(x, y, 1) / 32;
				match_b += PIX_Error(x, y, 2) / 32;
			}

			r = (match_r + 15) / 32;
			g = (match_g + 15) / 32;
			b = (match_b + 15) / 32;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 32;
			g = RGB_GREEN(pal_in->clrs[idx]) * 32;
			b = RGB_BLUE(pal_in->clrs[idx]) * 32;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			/*** these are mirrored versions of the error distributions in the above pass ***/
			error_diffuse(-1, 0, 8, 32);
			error_diffuse(-2, 0, 4, 32);
			error_diffuse(0, 1, 8, 32);
			error_diffuse(-1, 1, 4, 32);
			error_diffuse(1, 1, 4, 32);
			error_diffuse(-2, 1, 2, 32);
			error_diffuse(2, 1, 2, 32);
		}

		if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#endif  // BIDIRECTIONAL_DITHERING
	}

#undef	PIX_Error
	delete [] errors;
	return(bmp_out);
}

/* Create a representation of the image bmp_in using the palette pal_in, using Atkinson dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. */
SBitmap* atkinson_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out) {
	i32* errors = NULL;
	bool allocbmp = is_null(bmp_out);
	i32 y, x;
	
	if (is_null(bmp_out)) {
		bmp_out = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmp_out, NULL);
		bmp_out->set_palette(pal_in);
		if (is_null(bmp_out->palette()))
			goto LFree;
	}

	/* we only need to allocate two rows to contain the errors */
#define	PIX_Error(x_, y_, z_)		errors[ARRAY_3D_INDEX((x_), ((y_) - y), (z_), bmp_in->width(), 3)]
	errors = NEW_ARRAY_3D(i32, bmp_in->width(), 3, 3);
	if (unlikely(is_null(errors))) {
LFree:
		delete [] errors;
		if (allocbmp)
			delete bmp_out;
		return(NULL);
	}

	zeromem(errors, sizeof(i32) * bmp_in->width() * 3 * 3);

	for (y = 0; y < bmp_in->height(); ++y) {
		i32 match_r, match_g, match_b;
		i32 error_r, error_g, error_b;
		i32 r, g, b;
		u32 idx;
		for (x = 0; x < bmp_in->width(); ++x) {
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/8ths of component delta)
			match_r *= 8;
			match_g *= 8;
			match_b *= 8;
			match_r += PIX_Error(x, y, 0) / 8;
			match_g += PIX_Error(x, y, 1) / 8;
			match_b += PIX_Error(x, y, 2) / 8;

			r = (match_r + 3) / 8;
			g = (match_g + 3) / 8;
			b = (match_b + 3) / 8;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 8;
			g = RGB_GREEN(pal_in->clrs[idx]) * 8;
			b = RGB_BLUE(pal_in->clrs[idx]) * 8;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;

			error_diffuse(1, 0, 1, 8);
			error_diffuse(2, 0, 1, 8);
			error_diffuse(-1, 1, 1, 8);
			error_diffuse(0, 1, 1, 8);
			error_diffuse(1, 1, 1, 8);
			error_diffuse(0, 2, 1, 8)
		}

		// update the errors array
		if (y + 2 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = PIX_Error(x, y + 2, 0);
				PIX_Error(x, y + 1, 1) = PIX_Error(x, y + 2, 1);
				PIX_Error(x, y + 1, 2) = PIX_Error(x, y + 2, 2);
				PIX_Error(x, y + 2, 0) = 0;
				PIX_Error(x, y + 2, 1) = 0;
				PIX_Error(x, y + 2, 2) = 0;
			}
		else if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#ifdef BIDIRECTIONAL_DITHERING
		/* now go right-to-left */
		++y;
		if (y >= bmp_in->height())
			break;

		for (x = bmp_in->width() - 1; x >= 0; --x)
			{
			getpixelbmp_components(bmp_in, x, y, &match_r, &match_g, &match_b);

			// add in the error (measured in 1/8ths of component delta)
			match_r *= 8;
			match_g *= 8;
			match_b *= 8;
			match_r += PIX_Error(x, y, 0) / 8;
			match_g += PIX_Error(x, y, 1) / 8;
			match_b += PIX_Error(x, y, 2) / 8;

			r = (match_r + 3) / 8;
			g = (match_g + 3) / 8;
			b = (match_b + 3) / 8;
			r = COMPONENT_RANGE(r);
			g = COMPONENT_RANGE(g);
			b = COMPONENT_RANGE(b);

			idx = palette_index_from_rgb(pal_in, RGB_NO_CHECK(r, g, b));

			bmp_out->put_pixel_palette(x, y, idx);

			// now determine the error per component, and distribute to neighboring pixels
			r = RGB_RED(pal_in->clrs[idx]) * 8;
			g = RGB_GREEN(pal_in->clrs[idx]) * 8;
			b = RGB_BLUE(pal_in->clrs[idx]) * 8;

			error_r = match_r - r;
			error_g = match_g - g;
			error_b = match_b - b;
			
			/*** these are mirrored versions of the error distributions in the above pass ***/
			error_diffuse(-1, 0, 1, 8);
			error_diffuse(-2, 0, 1, 8);
			error_diffuse(1, 1, 1, 8);
			error_diffuse(0, 1, 1, 8);
			error_diffuse(-1, 1, 1, 8);
			error_diffuse(0, 2, 1, 8)
		}

		// update the errors array
		if (y + 2 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = PIX_Error(x, y + 2, 0);
				PIX_Error(x, y + 1, 1) = PIX_Error(x, y + 2, 1);
				PIX_Error(x, y + 1, 2) = PIX_Error(x, y + 2, 2);
				PIX_Error(x, y + 2, 0) = 0;
				PIX_Error(x, y + 2, 1) = 0;
				PIX_Error(x, y + 2, 2) = 0;
			}
		else if (y + 1 < bmp_in->height())
			for (x = 0; x < bmp_in->width(); ++x) {
				PIX_Error(x, y, 0) = PIX_Error(x, y + 1, 0);
				PIX_Error(x, y, 1) = PIX_Error(x, y + 1, 1);
				PIX_Error(x, y, 2) = PIX_Error(x, y + 1, 2);
				PIX_Error(x, y + 1, 0) = 0;
				PIX_Error(x, y + 1, 1) = 0;
				PIX_Error(x, y + 1, 2) = 0;
			}
#endif  // BIDIRECTIONAL_DITHERING
	}

#undef	PIX_Error
	delete [] errors;
	return(bmp_out);
}

/* Create a representation of the image bmp_in using the palette pal_in, using random dithering
	to reduce quantization error. If bmp_out is NULL, will create the bitmap. In certain cases, like
	a large color gradient, random dithering may give better results than error-diffusion methods. */
SBitmap* random_dither_bmp(SBitmap* bmp_in, SPalette* pal_in, SBitmap* bmp_out) {
	int x, y;
	u32 c1, c2, e1, e2;
	
	if (is_null(bmp_out)) {
		bmp_out = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_PALETTE);
		NOT_NULL_OR_RETURN(bmp_out, NULL);
		bmp_out->set_palette(pal_in);
	}

	for (y = 0; y < bmp_in->height(); ++y)
		for (x = 0; x < bmp_in->width(); ++x) {
			RGBColor c = bmp_in->get_pixel(x, y);
			palette_index_from_rgb_2(pal_in, c, &c1, &c2, &e1, &e2);
			if (iszero(e1)) {
				/* exact match. */
				bmp_out->put_pixel_palette(x, y, c1);
			} else {
				/* There is an e1/(e1 + e2) chance that we use c2 instead of c1. */
				if ((randint() % (e1 + e2)) < e1)
					bmp_out->put_pixel_palette(x, y, c2);
				else
					bmp_out->put_pixel_palette(x, y, c1);					
			}
		}

	return(bmp_out);
}

/* end quantize.cpp */
