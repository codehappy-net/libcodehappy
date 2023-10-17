/***

	stb_wrappers.c

	Functions to manipulate SBitmaps using STB library code.
	Implemented in a separate file so code that doesn't use these
	functions does not need to pull in the STB. (With link time
	optimizations, depending on your toolchain, this may not matter.)

	C. M. Street

***/

#include "libcodehappy.h"

#define	FORMAT_UNK	(-1)
#define	FORMAT_BMP	0
#define	FORMAT_PNG	1
#define	FORMAT_TGA	2
#define	FORMAT_GIF	3
#define	FORMAT_PCX	4
#define	FORMAT_RAW	5
#define FORMAT_JPG	6

// TODO: more formats?
/***
	Saves an SBitmap to file. The file type is determined from the extension.
	Supported formats: .BMP, .TGA, .PNG, .GIF, .PCX, .RAW, .JPG
	Note that .GIF/.PCX are 8-bit (or fewer) color formats.
	You can save a 32-bit bitmap to .GIF -- it will be palettized if it has
	a small number of colors, and quantized if too many colors.
	Returns 0 on success.
***/
u32 savebmp(SBitmap* bmp, const char* fname)
{
	SBitmap* bmp_use;
	u32 fmt = FORMAT_UNK;
	int bpp = 4;
	int ret;
	
	if (has_extension(fname, "bmp"))
		fmt = FORMAT_BMP;
	else if (has_extension(fname, "png"))
		fmt = FORMAT_PNG;
	else if (has_extension(fname, "tga"))
		fmt = FORMAT_TGA;
	else if (has_extension(fname, "gif"))
		fmt = FORMAT_GIF;
	else if (has_extension(fname, "pcx"))
		fmt = FORMAT_PCX;
	else if (has_extension(fname, "jpg") || has_extension(fname, "jpeg"))
		fmt = FORMAT_JPG;
	else
		fmt = FORMAT_RAW;

	ret = 0;
	bmp_use = bmp;

	switch (bmp->btype)
		{
	default:
		assert(false);
		return(1);
		
	case BITMAP_DEFAULT:
	case WINDOWS_DIB_COMPATIBLE:
		break;	

	case BITMAP_GRAYSCALE:
		bpp = 1;
		break;

	case BITMAP_MONO:
	case BITMAP_PALETTE:
		if (FORMAT_GIF != fmt && FORMAT_PCX != fmt)
			{
			bmp_use = new_bmp(bmp->w, bmp->h, BITMAP_DEFAULT);
			copy_and_translate_bmp(bmp, bmp_use);
			fill_alpha_channel_bmp(bmp_use, ALPHA_OPAQUE);
			}
		break;
		}

	switch (fmt)
		{
	case FORMAT_UNK:
		// unknown file type?
LErr:
		if (bmp_use != bmp)
			free_bmp(bmp_use);
		return(1);
		
	case FORMAT_TGA:
		if (!stbi_write_tga(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits))
			goto LErr;
		break;

	case FORMAT_PNG:
		if (!stbi_write_png(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits, bpp * bmp_use->w))
			goto LErr;
		break;

	case FORMAT_BMP:
		if (!stbi_write_bmp(fname, bmp_use->w, bmp_use->h, bpp, bmp_use->bits))
			goto LErr;
		break;

	case FORMAT_GIF:
		ret = save_gif(bmp_use, fname);
		break;

	case FORMAT_PCX:
		ret = save_pcx(bmp_use, fname);
		break;

	case FORMAT_RAW:
		ret = save_raw(bmp_use, fname);
		break;

	case FORMAT_JPG:
		ret = !tje_encode_to_file_at_quality(fname, 3, bmp_use->w, bmp_use->h, bpp, (const unsigned char*)bmp_use->bits);
		break;
		}

	if (bmp_use != bmp)
		free_bmp(bmp_use);
	return(ret);
}

/***
	Loads an SBitmap from file. The file type is automatically determined.

	Supported formats: .BMP, .TGA, .PNG, .GIF, .JPG, .PSD, .HDR, .PIC, .PNM, .PCX, .SVG, .RAW

	Returns the bitmap on success, or NULL on failure.
***/
SBitmap* loadbmp(const char* fname)
{
	int x, y;
	int comp;
	unsigned char* pixel_data;
	SBitmap* bmp_ret;

	if (has_extension(fname, "pcx"))
		{
		bmp_ret = load_pcx(fname);
		return(bmp_ret);
		}

	if (has_extension(fname, "svg"))
		{
		bmp_ret = load_svg(fname, 96.);
		return(bmp_ret);
		}

	if (has_extension(fname, "raw"))
		{
		bmp_ret = load_raw(fname);
		return(bmp_ret);
		}
	
	pixel_data = stbi_load(fname, &x, &y, &comp, 4);
	if (is_null(pixel_data))
		return(NULL);
	if (comp < 3)
		{
LErr:
		stbi_image_free(pixel_data);
		return(NULL);
		}

	bmp_ret = new_bmp(x, y, STB_COMPATIBLE);
	if (is_null(bmp_ret))
		goto LErr;
	memcpy(bmp_ret->bits, pixel_data, x * y * 4);

	stbi_image_free(pixel_data);

	return(bmp_ret);
}


/*** Wrappers for the TrueType code below. ***/

// TODO: gradient-shaded font!

int load_ttf_font(const char* filename, ttfont* font_out)
{
	u32 flen;
	FILE *f;

	f = fopen(filename, "rb");
	if (is_null(f))
		return(1);
	flen = filelen(f);
	
	font_out->data = NEW_ARRAY(unsigned char, flen);
	if (is_null(font_out->data))
		{
		fclose(f);
		return(1);
		}

	fread(font_out->data, 1, flen, f);
	fclose(f);

	stbtt_InitFont(&font_out->info, font_out->data, stbtt_GetFontOffsetForIndex(font_out->data, 0));

	return(0);
}

SBitmap* ttf_codepoint_bmp(ttfont* font, int codepoint, int size)
{
	unsigned char* bits, *px;
	int w, h;
	int e, f;
	SBitmap *bmp;
	
	bits = stbtt_GetCodepointBitmap(&font->info,
						0,
						stbtt_ScaleForPixelHeight(&font->info, size),
						codepoint,
						&w,
						&h,
						0,
						0);

	bmp = new_bmp(w, h, BITMAP_GRAYSCALE);
	px = bits;
	for (f = 0; f < h; ++f)
		{
		for (e = 0; e < w; ++e)
			{
			fast_putpixelbmp(bmp, e, f, RGB_NO_CHECK(*px, *px, *px));
			++px;
			}
		}
	free(bits);	// STBTT_free

	return(bmp);
}

SBitmap* sprite_from_ttf_codepoint(ttfont *font, int codepoint, int size, RGBColor c)
{
	SBitmap* sprite;
	unsigned char* bits, *px;
	int w, h;
	int e, f;
	
	bits = stbtt_GetCodepointBitmap(&font->info,
						0,
						stbtt_ScaleForPixelHeight(&font->info, size),
						codepoint,
						&w,
						&h,
						0,
						0);

	sprite = new_bmp(w, h, BITMAP_DEFAULT);
	px = bits;
	for (f = 0; f < h; ++f)
		{
		for (e = 0; e < w; ++e)
			{
			fast_putpixelbmp(sprite, e, f, ADD_ALPHA(c, *px));
			++px;
			}
		}
	free(bits);	// STBTT_free

	return(sprite);
}

/*** as above, but allows user-specified callback to determine the pixel colors in the sprite ***/
SBitmap* sprite_from_ttf_codepoint_pattern(ttfont *font, int codepoint, int size, PatternCallback pattern_callback, void* args)
{
	SBitmap* sprite;
	unsigned char* bits, *px;
	int w, h;
	int e, f;
	
	bits = stbtt_GetCodepointBitmap(&font->info,
						0,
						stbtt_ScaleForPixelHeight(&font->info, size),
						codepoint,
						&w,
						&h,
						0,
						0);

	sprite = new_bmp(w, h, BITMAP_DEFAULT);
	px = bits;
	for (f = 0; f < h; ++f)
		{
		for (e = 0; e < w; ++e)
			{
			fast_putpixelbmp(sprite, e, f, ADD_ALPHA(pattern_callback(e, f, args), *px));
			++px;
			}
		}
	free(bits);	// STBTT_free

	return(sprite);
}

static void __ttf_bmp_char_blt(SBitmap* src, SBitmap *dest_bmp, int x, int y)
{
	int e, f;

	for (f = 0; f <= src->h; ++f)
		{
		for (e = 0; e <= src->w; ++e)
			{
			int xx = x + e;
			int yy = y + f;

			if (!pixel_ok(dest_bmp, xx, yy))
				continue;
			
			RGBColor c = fast_getpixelbmp(src, e, f);
			RGBColor cd;
			u32 gray = (c & 0xff);
			u32 r;
			if (c == C_BLACK)
				continue;
			if (c == C_WHITE)
				{
				putpixelbmp(dest_bmp, xx, yy, C_WHITE);
				continue;
				}
			// blend case
			cd = fast_getpixelbmp(dest_bmp, xx, yy);
			
			r = (RGB_RED(c) * gray) + (RGB_RED(cd) * (255 - gray));
			r /= 255;

			putpixelbmp(dest_bmp, xx, yy, RGB_NO_CHECK(r, r, r));			
			}
		}
	
}

#undef		DEBUG_OUTPUT

SBitmap* ttf_bmp_cstr(ttfont* font, const char* str, int size, bool single_character_blend, u16** char_index_to_x_pos)
{
	/*** translate to Unicode string and render ***/
	ustring ustr;
	SBitmap* bmpret;
	
	ustr = cstr2ustr(str);
	NOT_NULL_OR_RETURN(ustr, NULL);

	bmpret = ttf_bmp_ustr(font, ustr, size, single_character_blend, char_index_to_x_pos);
	DELETE(ustr);

	return(bmpret);
}

/*** Helper function: Trim a text bitmap produced by blt_ttf_bmp(). Note that this frees the passed-in bitmap. ***/
static SBitmap* trim_text_bmp(SBitmap* bmp, bool left_space_ok)
{
	int x1, x2, y1, y2, xs, ys;
	SBitmap* bmpret;
	for (y1 = 0; y1 < bmp->h; ++y1)
		for (x1 = 0; x1 < bmp->w; ++x1)
			{
			if (getpixelbmp(bmp, x1, y1) != C_BLACK)
				{
				goto Pass2;
				}
			}
Pass2:
	for (xs = 0; xs < bmp->w; ++xs)
		for (ys = 0; ys < bmp->h; ++ys)
			{
			if (getpixelbmp(bmp, xs, ys) != C_BLACK)
				{
				x1 = min_int(x1, xs);
				y1 = min_int(y1, ys);
				goto Pass3;
				}
			}
Pass3:
	for (y2 = bmp->h - 1; y2 >= 0; --y2)
		for (x2 = bmp->w - 1; x2 >= 0; --x2)
			{
			if (getpixelbmp(bmp, x2, y2) != C_BLACK)
				{
				goto Pass4;
				}
			}
Pass4:
	for (xs = bmp->w - 1; xs >= 0; --xs)
		for (ys = bmp->h - 1; ys >= 0; --ys)
			{
			if (getpixelbmp(bmp, xs, ys) != C_BLACK)
				{
				x2 = max_int(x2, xs);
				y2 = max_int(y2, ys);
				goto LDone;
				}
			}
LDone:
	SORT2(x1, x2, int);
	SORT2(y1, y2, int);
	x1 = CLAMP(x1, 0, bmp->w - 1);
	x2 = CLAMP(x2, 0, bmp->w - 1);
	y1 = CLAMP(y1, 0, bmp->h - 1);
	y2 = CLAMP(y2, 0, bmp->h - 1);
	if (left_space_ok)
		x1 = 0;
	bmpret = new_bmp(x2 - x1 + 1, y2 - y1 + 1, bmp->btype);
	blitbmp(bmp, x1, y1, x2, y2, bmpret, 0, 0);
	freebmp(bmp);
	return (bmpret);
}

#define	TAB_SPACES	4

SBitmap* ttf_bmp_ustr(ttfont* font, ustring str, int size, bool single_character_blend, u16** char_index_to_x_pos)
{
	float scale = stbtt_ScaleForPixelHeight(&font->info, size);
	int ascent;
	int baseline;
	int advance;
	int lsb;
	int x;
	float xpos;
	const uch* w;
	SBitmap *outbmp;
	SBitmap* cbmp;
	int cx, cy;
	u32 len;
	int ix;
#ifdef DEBUG_OUTPUT
	FILE *f;

	f = fopen("fontrender.txt", "w");
#endif

	if (not_null(char_index_to_x_pos))
		{
		len = ustrlen(str);
		*char_index_to_x_pos = NEW_ARRAY(u16, len + 1);
		}

	stbtt_GetFontVMetrics(&font->info, &ascent, 0, 0);
	baseline = (int) (ascent * scale);

	cx = 128;
	cy = 128;

	x = 20;	/* left-padding */
	w = str;
	while (*w)
		{
		if (*w == '\t')
			stbtt_GetCodepointHMetrics(&font->info, ' ', &advance, &lsb);
		else
			stbtt_GetCodepointHMetrics(&font->info, *w, &advance, &lsb);
		if ((int)floor(advance * scale + 0.5) > cx)
			{
			cx = (int)floor(advance * scale + 0.5);
			cx += (cx >> 1);
			}
		if (*w == '\t')
			x += (int)floor(advance * scale + 0.5) * TAB_SPACES;
		else
			x += (int)floor(advance * scale + 0.5);
		++w;
		}

	// probably bigger than we need, but that's O.K. 
	outbmp = new_bmp(x, size * 3, BITMAP_GRAYSCALE);
	fillrectbmp(outbmp, 0, 0, outbmp->w - 1, outbmp->h - 1, C_BLACK);

	if (outbmp->h > cy / 2)
		cy = outbmp->h * 2;

#ifdef DEBUG_OUTPUT
	fprintf(f, "Allocated a bitmap %d x %d for text rendering.\n", outbmp->w, outbmp->h);
#endif
	xpos = 2.;
	w = str;
	ix = 0;
	while (*w)
		{
		int x0, y0, x1, y1;
		float x_shift = xpos - (float) floor(xpos);
		int e, n = 1;

		if (*w == '\t')
			{
			n = 4;
			stbtt_GetCodepointHMetrics(&font->info, ' ', &advance, &lsb);
			stbtt_GetCodepointBitmapBoxSubpixel(&font->info, ' ', scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
			}
		else
			{
			n = 1;
			stbtt_GetCodepointHMetrics(&font->info, *w, &advance, &lsb);
			stbtt_GetCodepointBitmapBoxSubpixel(&font->info, *w, scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);
			}

#ifdef DEBUG_OUTPUT
		fprintf(f, "Rendering character '%c'\n", *w);
		fprintf(f, "  (x0, y0): (%d, %d), (x1, y1): (%d, %d)\n",
			x0, y0, x1, y1);
		fprintf(f, "  pixel pos: (%d, %d) to (%d, %d)\n",
			((int)xpos + x0), baseline + y0,
			((int)xpos + x1), (baseline + y1));
#endif

		if (not_null(char_index_to_x_pos))
			{
			(*char_index_to_x_pos)[ix] = xpos + x0;
			++ix;
			}

		for (e = 0; e < n; ++e)
		{
		if (!single_character_blend)
			{
			// Faster but slightly-lower quality render
			if (pixel_ok(outbmp, ((int)xpos + x0), baseline + y0 + 4) &&
				pixel_ok(outbmp, ((int)xpos + x1), (baseline + y1 + 4)))
				stbtt_MakeCodepointBitmapSubpixel(&font->info,
						(unsigned char*)&outbmp->bits[(baseline + y0 + 4) * outbmp->w + ((int)xpos + x0)],
						x1 - x0,
						y1 - y0,
						outbmp->w,
						scale,
						scale,
						x_shift,
						0,
						*w);
			}
		else
			{
			// Use ttf_codepoint_bmp() to generate the single-character SBitmap, then blend into the image.
			int xx, yy;
			if (*w == '\t')
				cbmp = ttf_codepoint_bmp(font, ' ', size);
			else
				cbmp = ttf_codepoint_bmp(font, *w, size);
			if (not_null(cbmp))
				{
				for (yy = 0; yy < cbmp->h; ++yy)
					for (xx = 0; xx < cbmp->w; ++xx)
						{
						RGBColor cc, cb;
						int xd, yd;
						u32 g1, g2;
						
						xd = (int)xpos + x0 + xx;
						yd = baseline + y0 + 4 + yy;
						
						cc = fast_getpixelbmp(cbmp, xx, yy);
						if (cc == C_BLACK)
							continue;
						if (cc == C_WHITE)
							{
							putpixelbmp(outbmp, xd, yd, C_WHITE);
							continue;
							}
						cb = fast_getpixelbmp(outbmp, xd, yd);
						g1 = RGB_RED(cb);
						g2 = RGB_RED(cc);
						g1 += g2;
						g1 >>= 1;
						putpixelbmp(outbmp, xd, yd, RGB(g1, g1, g1));
						}
				free_bmp(cbmp);
				}
			}

		xpos += (advance * scale);
		}	// end for-loop
		++w;

		if (*w && *(w - 1) != '\t')
			xpos += scale * stbtt_GetCodepointKernAdvance(&font->info, *(w - 1), *w);
		}

	if (not_null(char_index_to_x_pos))
		(*char_index_to_x_pos)[ix] = xpos;

#ifdef DEBUG_OUTPUT
	fclose(f);
#endif

	return(trim_text_bmp(outbmp, true));
}

void blt_ttf_bmp(SBitmap* ttf_bmp, SBitmap* dest_bmp, int x, int y, RGBColor text_clr)
{
	int x0, x1, y0, y1;
	int xx, yy;
	int e, f;
	int sx, sy;

	x0 = x;
	y0 = y;
	x1 = x + ttf_bmp->w - 1;
	y1 = y + ttf_bmp->h - 1;

	if ((x > 0 && x >= dest_bmp->w) || (y > 0 && y >= dest_bmp->h))
		return;

	VALID_RECT(dest_bmp, x0, y0, x1, y1);

	xx = x1 - x0;
	yy = y1 - y0;

	sx = (x0 - x);
	sy = (y0 - y);

	for (f = 0; f <= yy; ++f)
		{
		for (e = 0; e <= xx; ++e)
			{
			RGBColor c = fast_getpixelbmp(ttf_bmp, sx + e, sy + f);
			RGBColor cd;
			u32 gray = (c & 0xff);
			u32 r, g, b;
			if (c == C_BLACK)
				continue;
			if (c == C_WHITE)
				{
				fast_putpixelbmp(dest_bmp, x0 + e, y0 + f, text_clr);
				continue;
				}
			// OK, easy cases are out of the way: we need to do some blending
			cd = fast_getpixelbmp(dest_bmp, x0 + e, y0 + f);
			
			r = (RGB_RED(text_clr) * gray) + (RGB_RED(cd) * (255 - gray));
			g = (RGB_GREEN(text_clr) * gray) + (RGB_GREEN(cd) * (255 - gray));
			b = (RGB_BLUE(text_clr) * gray) + (RGB_BLUE(cd) * (255 - gray));
			r /= 255;
			g /= 255;
			b /= 255;

			fast_putpixelbmp(dest_bmp, x0 + e, y0 + f, RGB_NO_CHECK(r, g, b));			
			}
		}
	
}
void free_ttf_font(ttfont* font)
{
	DELETE(font->data);
	font->data = NULL;
}

/*** Bitmap resizing and scaling ***/

SBitmap* resizebmp(SBitmap* bmp_in, int new_width, int new_height)
{
	SBitmap* bmpret;
	int bpp;
	double ratio;
	SBitmap* bmpuse;

	if (unlikely(is_null(bmp_in)))
		return(NULL);

	bmpuse = bmp_in;
	switch (bmp_in->btype)
		{
	case BITMAP_DEFAULT:
	case WINDOWS_DIB_COMPATIBLE:
		bpp = 4;
		break;
		
	case BITMAP_GRAYSCALE:
		bpp = 1;
		break;

	case BITMAP_MONO:
	case BITMAP_PALETTE:
	default:
		// support these by creating a temporary bitmap
		bmpuse = new_bmp(bmp_in->w, bmp_in->h, BITMAP_DEFAULT);
		NOT_NULL_OR_RETURN(bmpuse, NULL);
		blitbmp(bmp_in, 0, 0, bmp_in->w, bmp_in->h, bmpuse, 0, 0);
		bpp = 4;
		}

	if (new_width == 0)
		{
		/* set the width according to aspect ratio */
		if (unlikely(new_height == 0))
			return(NULL);
		ratio = (double)(bmp_in->w) / (double)(bmp_in->h);
		ratio *= (double)new_height;
		new_width = ROUND_FLOAT_TO_INT(ratio);
		}
	else if (new_height == 0)
		{
		/* set the height acccording to aspect ratio */
		ratio = (double)(bmp_in->h)/ (double)(bmp_in->w);
		ratio *= (double)new_width;
		new_height = ROUND_FLOAT_TO_INT(ratio);
		}

	bmpret = new_bmp(new_width, new_height, bmpuse->btype);
	if (unlikely(is_null(bmpret)))
		return(NULL);

	if (!stbir_resize_uint8((unsigned char*)bmpuse->bits, bmpuse->w, bmpuse->h, 0, (unsigned char*)bmpret->bits, bmpret->w, bmpret->h, 0, bpp))
		{
		free_bmp(bmpret);
		if (bmpuse != bmp_in)
			free_bmp(bmpuse);
		return(NULL);
		}

	if (bmpuse != bmp_in)
		free_bmp(bmpuse);

	return(bmpret);
}

SBitmap* scalebmp_rational(SBitmap* bmp_in, u32 num, u32 den)
{
	i32 nw, nh;

	if (unlikely(is_null(bmp_in)))
		return(NULL);

	nw = (i32)((i32)(bmp_in->w * num) / den);
	nh = (i32)((i32)(bmp_in->h * num) / den);

	return (resizebmp(bmp_in, nw, nh));
}

SBitmap* scalebmp(SBitmap* bmp_in, double scale_factor)
{
	i32 nw, nh;

	if (unlikely(is_null(bmp_in)))
		return(NULL);

	nw = ROUND_FLOAT_TO_INT((double)bmp_in->w * scale_factor);
	nh = ROUND_FLOAT_TO_INT((double)bmp_in->h * scale_factor);

	return (resizebmp(bmp_in, nw, nh));
}

/*** Reading and rasterizing vector graphics in SVG format ***/

SBitmap* load_svg(const char* fname, float size)
{
	struct NSVGimage* image;
	static struct NSVGrasterizer* rast = NULL;
	SBitmap* bmp;

	if (is_null(rast))
		{
		rast = nsvgCreateRasterizer();
		NOT_NULL_OR_RETURN(rast, NULL);
 		}

	image = nsvgParseFromFile(fname, "px", size);
	bmp = new_bmp(image->width, image->height, BITMAP_DEFAULT);

	nsvgRasterize(rast, image, 0, 0, 1, (unsigned char*)bmp->bits, image->width, image->height, bmp->w * 4);

	return(bmp);
}

/*** Implementation of the Draw class (SBitmap wrapper). The implementation code was moved here since it
	pulls in STB code, and for compilers w/o link-time optimization we want to avoid pulling in the extra code unless
	it's used. ***/
#ifdef CODEHAPPY_CPP

// TODO: remove dependence on these functions
extern void __putstringbmp(char *bits, char *str, int x, int y, int w, int h, int s, int c, BitmapType bt, int bg);
extern void __putnumberstringbmp(char *bits, int num, int x, int y, int w, int h, int s, int c, BitmapType bt, int bg);
extern void __putnumberfontbmp(char *bits, char *str, int x, int y, int w, int h, int s, int c, BitmapType bt, int bg);

Draw::Draw()
{
	this->bmp_ = NULL;
	_loginterp = false;
}

Draw::Draw(const char* filename)
{
	this->bmp_ = loadbmp(filename);
	_loginterp = false;
}

Draw::~Draw()
{
	Destroy();
}

void Draw::Create(int x, int y, BitmapType bt)
{
	if (bmp_ != NULL)
		Destroy();

	bmp_ = new_bmp(x, y, bt);

	Fill(C_WHITE);
}

int Draw::Color(int r, int g, int b) const
{
	r = COMPONENT_RANGE(r);
	g = COMPONENT_RANGE(g);
	b = COMPONENT_RANGE(b);
	return RGB_NO_CHECK(r, g, b);
}

int Draw::Width(void) const
{
	NOT_NULL_OR_RETURN(bmp_, 0);
	return(bmp_->w);
}

int Draw::Height(void) const
{
	NOT_NULL_OR_RETURN(bmp_, 0);
	return(bmp_->h);
}

void Draw::Pixel(int x, int y, int c)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	putpixelbmp(bmp_, x, y, (RGBColor)c);
}

int Draw::GetPixel(int x, int y)
{
	NOT_NULL_OR_RETURN(bmp_, -1);
	return (int)getpixelbmp(bmp_, x, y);
}
void Draw::Line(int x1, int y1, int x2, int y2, int c)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	drawlinebmp(bmp_, x1, y1, x2, y2, (RGBColor)c);
}

void Draw::VLine(int x, int y1, int y2, int c, bool dotted)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	vlinebmp(bmp_, x, y1, y2, (RGBColor)c, (dotted ? 1 : 0));
}

void Draw::HLine(int x1, int x2, int y, int c, bool dotted)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	hlinebmp(bmp_, x1, x2, y, (RGBColor)c, (dotted ? 1 : 0));
}

void Draw::Rect(int x1, int y1, int x2, int y2, int c)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	rectbmp(bmp_, x1, y1, x2, y2, (RGBColor)c);
}

void Draw::FillRect(int x1, int y1, int x2, int y2, int c)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	fillrectbmp(bmp_, x1, y1, x2, y2, (RGBColor)c);
}

void Draw::Fill(int c)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	fillbmp(bmp_, (RGBColor)c);
}

void Draw::Text(char *text, int x, int y, int forecolor, int backcolor, bool centerhorz, bool centervert, int size)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	if (centerhorz)
		x -= (3 * strlen(text) * size);

	if (centervert)
		y -= ((5 * size) / 2);

	__putstringbmp(bmp_->bits, text, x, y, bmp_->w, bmp_->h, size, forecolor, bmp_->btype, backcolor);
}

void Draw::NumberText(int num, int x, int y, int forecolor, int backcolor, bool centerhorz, bool centervert, int size)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	// treating all numbers as 2 digits in this implementation
	if (centerhorz)
		x -= (4 * 2 * size);

	if (centervert)
		y -= ((8 * size) / 2);

	__putnumberstringbmp(bmp_->bits, num, x, y, bmp_->w, bmp_->h, size, forecolor, bmp_->btype, backcolor);
}

void Draw::BetterText(char *text, int x, int y, int forecolor, int backcolor, bool centerhorz, bool centervert, int size)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	// treating all numbers as 2 digits in this implementation
	if (centerhorz)
		x -= (3 * strlen(text) * size);

	if (centervert)
		y -= ((8 * size) / 2);

	__putnumberfontbmp(bmp_->bits, text, x, y, bmp_->w, bmp_->h, size, forecolor, bmp_->btype, backcolor);
}

void Draw::Save(const char *fname)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	savebmp(bmp_, fname);
}

void Draw::Destroy(void)
{
	if (not_null(bmp_))
		free_bmp(bmp_);
	bmp_ = NULL;
}

void Draw::SubBitmap(Draw *dsub, int x1, int y1, int x2, int y2)
{
	int x;
	int y;

	if (x2 < x1)
		{
		x = x2;
		x2 = x1;
		x1 = x;
		}
	if (y2 < y1)
		{
		y = y2;
		y2 = y1;
		y1 = y;
		}

	dsub->Create(x2 - x1 + 1, y2 - y1 + 1, bmp_->btype);
	dsub->Blit(this, 0, 0, x1, y1, x2, y2);
}

void Draw::Blit(Draw *dblit, int xdest, int ydest, int xsrc_start, int ysrc_start, int xsrc_end, int ysrc_end)
{
	int x;
	int y;

	if (xsrc_end < xsrc_start)
		{
		x = xsrc_end;
		xsrc_end = xsrc_start;
		xsrc_start = x;
		}
	if (ysrc_end < ysrc_start)
		{
		y = ysrc_end;
		ysrc_end = ysrc_start;
		ysrc_start = y;
		}

	for (y = ysrc_start; y <= ysrc_end; ++y)
		{
		for (x = xsrc_start; x <= xsrc_end; ++x)
			{
			Pixel(x - xsrc_start + xdest, y - ysrc_start + ydest, dblit->GetPixel(x, y));
			}
		}
}

void Draw::SetAxes(double xmin, double xmax, double ymin, double ymax)
{
	xaxismin = xmin;
	xaxismax = xmax;
	yaxismin = ymin;
	yaxismax = ymax;
}

void Draw::SetLogYScale(const bool islog)
{
	this->_loginterp = islog;
}

void Draw::InterpolateAxes(double x, double y, int &xbmp, int &ybmp)
{
	x -= xaxismin;
	x /= (xaxismax - xaxismin);

	xbmp = (int)floor(((double)bmp_->w * x) + 0.5);

	if (!this->_loginterp)
		{
		y -= yaxismin;
		y /= (yaxismax - yaxismin);
		}
	else
		{
		y = log(y) - log(yaxismin);
		y /= (log(yaxismax) - log(yaxismin));
		}

	ybmp = bmp_->h - (int)floor(((double)bmp_->h * y) + 0.5);
}

void Draw::SetFromSBitmap(SBitmap *sbmp)
{
	Destroy();
	bmp_= copybmp(sbmp);
}

SBitmap* Draw::SetToSBitmap()
{
	NOT_NULL_OR_RETURN(bmp_, NULL);
	return copybmp(bmp_);
}

void Draw::FillAlphaChannel(const int val)
{
	NOT_NULL_OR_RETURN_VOID(bmp_);
	fill_alpha_channel_bmp(bmp_, val);
}

#endif  // CODEHAPPY_CPP

CODEHAPPY_SOURCE_BOTTOM
/*** end __stb_wrappers.c ***/
