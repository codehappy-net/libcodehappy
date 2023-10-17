#include "libcodehappy.h"

// GIF Saver
// original public domain code written Paul Bartrum
// retooled for use with 32-bit bitmaps by adding the cp-hash, Chris Street, 2005
// further rewritten for use in C, Chris Street

// TODO: add my 1996 implementation of GIFSave, including GIF89a support?

struct LZW_STRING {
	short base;
	unsigned char _new;
};

struct BUFFER {
	int pos;
	int bit_pos;
	unsigned char data[255];
};

#define	COLHASH_SIZE	1122419 	/* previously 9173 */

// could use stretchy buffers here as well, but I haven't tested those in C++ yet
darray(cp) coltable;
#define	coltablei(i)	darray_item(coltable, i)
int *__colhash = NULL;
FILE* file_handle = nullptr;

#define HASHCOL(r, g, b)		(((1777 * r) + (1931 * g) + (2593 * b)) % COLHASH_SIZE)

static FILE *pack_fopen(const char *filename) {
	return fopen(filename, "wb");
}

static void pack_putc(int c, FILE *f) {
	fputc(c, f);
}

static void pack_fwrite(void *p, int size, FILE *f) {
	fwrite(p, 1, size, f);
}

static void pack_mputl(int i, FILE *f) {
	fputc(i >> 24, f);
	fputc((i >> 16) & 0xff, f);
	fputc((i >> 8) & 0xff, f);
	fputc(i & 0xff, f);
}

static void pack_mputw(int i, FILE *f) {
	fputc((i >> 8) & 0xff, f);
	fputc(i & 0xff, f);
}

static void pack_iputw(int i, FILE *f) {
	fputc(i & 0xff, f);
	fputc((i >> 8) & 0xff, f);
}

static void pack_fclose(FILE *f) {
	fclose(f);
}

static void clear_speed_buffer(short *speed_buffer) {
	int i;
	for (i=0; i < 256 * 4096; ++i)
		speed_buffer[i] = -1;
}

static void dump_buffer(BUFFER *b) {
	int size;

	size = b->pos;
	if(b->bit_pos != 0)
		size ++;

	pack_putc(size, file_handle);
	pack_fwrite(b->data, size, file_handle);
}

static void output(BUFFER *b, int bit_size, int code) {
	int shift;

	// pack the code into the buffer
	shift = b->bit_pos;
	do {
		if(shift >= 0) {
			if(b->bit_pos != 0)
				b->data[b->pos] = (unsigned char)((code << shift) | b->data[b->pos]);
			else
				b->data[b->pos] = (unsigned char)(code << shift);
		} else {
			if(b->bit_pos != 0)
				b->data[b->pos] = (unsigned char)((code >> -shift) | b->data[b->pos]);
			else
				b->data[b->pos] = (unsigned char)(code >> -shift);
		}
		if(bit_size + shift > 7) {
			b->bit_pos = 0;
			b->pos ++;
			shift -= 8;
			if(b->pos == 255) {
				dump_buffer(b);
				b->pos = 0;
				b->bit_pos = 0;
			}
			if(bit_size + shift <= 0)
				break;
		} else {
			b->bit_pos = bit_size + shift;
			break;
		}
	} while(1);
}

void getpixelbmp_components(SBitmap* bmp, int x, int y, int* r, int* g, int* b) {
	RGBColor rgb = bmp->get_pixel(x, y);
	*r = RGB_RED(rgb);
	*g = RGB_GREEN(rgb);
	*b = RGB_BLUE(rgb);
	return;
}

static int __gif_getpixel(SBitmap* bmp, int x, int y) {
	const int w = bmp->width();
	const int h = bmp->height();
	const BitmapType bt = bmp->type();
	int hash;
	cp col;

	if (x >= w || y >= h || x < 0 || y < 0)
		return -1;

	getpixelbmp_components(bmp, x, y, &col.r, &col.g, &col.b);
	hash = HASHCOL(col.r, col.g, col.b);

	while (__colhash[hash] != -1) {
		if (coltablei(__colhash[hash]).r == col.r &&
			coltablei(__colhash[hash]).g == col.g &&
			coltablei(__colhash[hash]).b == col.b) {
			// keep a population count (for the quantization functions)
			coltablei(__colhash[hash]).count++;
			break;
		}
		hash++;
		if (hash == COLHASH_SIZE)
			hash = 0;
	}

	return (__colhash[hash]);
}

void construct_coltable(SBitmap* bmp) {
	// this now keeps a population count in the coltable, for quantize.cpp
	const int w = bmp->width();
	const int h = bmp->height();
	const BitmapType bt = bmp->type();
	int e, f;
	cp col;
	int hash;

	if (darray_size(coltable) > 0) {
		darray_free(coltable);
		darray_init(coltable);
	}

	if (is_null(__colhash)) {
		__colhash = NEW_ARRAY(int, COLHASH_SIZE);
		check_mem_or_die(__colhash);
	}

	for (e = 0; e < COLHASH_SIZE; ++e)
		__colhash[e] = -1;

	for (f=0; f<h; ++f) {
		for (e=0; e<w; ++e) {
			if (__gif_getpixel(bmp, e, f) == -1) {
				getpixelbmp_components(bmp, e, f, &col.r, &col.g, &col.b);
				hash = HASHCOL(col.r, col.g, col.b);
				col.count = 1;
				while (__colhash[hash] != -1) {
					hash++;
					if (hash == COLHASH_SIZE)
						hash = 0;
				}
				darray_push(coltable, col);
				__colhash[hash] = darray_size(coltable) - 1;
			}
		}
	}
}

int save_gif(SBitmap *bmp, const char* filename) {
	const int bw = bmp->width();
	const int bh = bmp->height();
	const BitmapType bt = bmp->type();
	int i, bpp, bit_size;
	LZW_STRING string_table[4096];
	int prefix;
	int input_pos = 0;
	int c;												// current character
	int empty_string;
	BUFFER buffer;
	short *speed_buffer;

	construct_coltable(bmp);
	if (darray_size(coltable) > 256)
//		return -7;	// too bad
		{
		// We'll support this now by quantizing the output.
		SBitmap* bmpquant;
		bmpquant = quantize_bmp_greedy(bmp, 256, NULL, dither_sierra, colorspace_rgb);
		NOT_NULL_OR_RETURN(bmpquant, -7);
		i = save_gif(bmpquant, filename);
		delete bmpquant;
		return(i);
		}

	file_handle = pack_fopen(filename);
	if (!file_handle)
		return errno;

	// TODO: this presupposes little-endian architecture
	pack_mputl(0x47494638, file_handle);		// GIF8
	pack_mputw(0x3761, file_handle);				// 7a
	pack_iputw(bw, file_handle);				// width
	pack_iputw(bh, file_handle);				// height
	pack_putc(215, file_handle);						// packed fields
	pack_putc(0, file_handle);							// background colour
	pack_putc(0, file_handle);							// pixel aspect ratio

	// global colour table
	for (i=0; i<darray_size(coltable); ++i) {
		pack_putc(coltablei(i).r, file_handle);
		pack_putc(coltablei(i).g, file_handle);
		pack_putc(coltablei(i).b, file_handle);
	}
	for (i=darray_size(coltable); i < 256; ++i) {
		pack_putc(0, file_handle);
		pack_putc(0, file_handle);
		pack_putc(0, file_handle);
	}
 
	pack_putc(0x2c, file_handle);						// image separator
	pack_iputw(0, file_handle);							// x offset
	pack_iputw(0, file_handle);							// y offset
	pack_iputw(bw, file_handle);				// width
	pack_iputw(bh, file_handle);				// height
	pack_putc(0, file_handle);							// packed fields

	// Image data starts here
	bpp = 8;
	pack_putc(bpp, file_handle);			// initial code size

	// initialize string table
	for(i = 0; i < 1 << bpp; i ++) {
		string_table[i].base = -1;
		string_table[i]._new = i;
	}
	for(; i < (1 << bpp) + 2; i ++) {
		string_table[i].base = -1;
		string_table[i]._new = -1;
	}
	empty_string = (1 << bpp) + 2;

	prefix = -1;

	bit_size = bpp + 1;

	buffer.pos = 0;
	buffer.bit_pos = 0;

	output(&buffer, bit_size, 1 << bpp);					// clear code

	speed_buffer = NEW_ARRAY(short, 256 * 4096 * 2);
	clear_speed_buffer(speed_buffer);

	while (1) {
		if((c = __gif_getpixel(bmp, input_pos % bw, input_pos / bw)) == -1) {
			output(&buffer, bit_size, prefix);
			output(&buffer, bit_size, (1 << bpp) + 1);		// end of information
			dump_buffer(&buffer);
			pack_putc(0, file_handle);		// no more data blocks
			break;
		}
		input_pos ++;

		if(prefix == -1)
			i = c;
		else
			i = speed_buffer[prefix * 256 + c];
		if (i != -1) {
			prefix = i;
		} else {
			// add prefix + c to string table
			string_table[empty_string].base = prefix;
			string_table[empty_string]._new = c;

			//if(prefix < 512)
				speed_buffer[prefix * 256 + c] = empty_string;

			empty_string++;

			// output code for prefix
			output(&buffer, bit_size, prefix);

			if(empty_string == (1 << bit_size) + 1)
				bit_size ++;

			// make sure string table doesn't overflow
			if(empty_string == 4095) {
				output(&buffer, bit_size, 1 << bpp); // clear code
				empty_string = (1 << bpp) + 2;
				bit_size = bpp + 1;

				clear_speed_buffer(speed_buffer);
			}

			// set prefix to c
			prefix = c;
		}
	}

	delete [] speed_buffer;

	pack_putc(0x3b, file_handle);						// trailer (end of gif)
	pack_fclose(file_handle);
	return errno;
}

// end gif.cpp
