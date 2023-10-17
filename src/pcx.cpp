/***

	pcx.cpp

	Support for PC Paintbrush .PCX image files

	A practically obsolete file format nowadays, but they are occasionally encountered.

	Copyright (c) 1997-2022 Chris M. Street.
	
***/
#include "libcodehappy.h"

/***

	This is ported to C from BMP2PCX.EX and PCX.EX, an implementation I wrote in Euphoria in 1997. -- CMS

***/


/***
	Save the image bmp as a .PCX named filename.
***/
int save_pcx(SBitmap* bmp, const char* filename) {
	RamFile rf;
	const unsigned char hdr1[] =
		{ 10, 5, 1, 8, 0, 0, 0, 0 };	/* We only write Paintbrush 3.0+ format PCX files. */
	const unsigned char hdr2[] = {
		64, 1, 200, 0, 0, 0, 0, 216, 152, 56, 120, 116, 4, 112, 108, 4, 236,
		172, 76, 248, 196, 128, 64, 36, 36, 36, 40, 20, 248,
		188, 104, 212, 144, 156, 60, 36, 36, 116, 112, 8,
		120, 116, 8, 124, 120, 8, 52, 48, 4, 240, 196, 136,
		0, 1, 64, 1, 0, 0, 
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
	};
	int e, f;

	/*** image must be palettized first. ***/
	if (bmp->type() != BITMAP_PALETTE)
		return(1);

	rf.open(filename, RAMFILE_DEFAULT);

	rf.putmem((char *)hdr1, sizeof(hdr1));
	rf.putc(bmp->width() & 0xff);
	rf.putc((bmp->width() >> 8) & 0xff);
	rf.putc(bmp->height() & 0xff);
	rf.putc((bmp->height() >> 8) & 0xff);
	rf.putmem((char *)hdr2, sizeof(hdr2));

	// RLE the image
	for (e = 0; e < bmp->height(); ++e) {	
		int t;
		t = 0;
		while (t < bmp->width()) {
			int i;
			u32 c;
			i = 0;

			c = (u32)bmp->get_pixel_palette(t, e);

			while (t + i + 1 < bmp->width() &&
				c == bmp->get_pixel_palette(t + i + 1, e) && i <= 62)
				++i;

			if (i > 0) {
				rf.putc(i | 192);
				rf.putc(c);
				t += i;
 			} else {
				if ((c & 192) > 0)
					rf.putc(193);
				rf.putc(c);
				++t;
			}
		}
	}

	// write the image palette
	rf.putc(12);
	for (e = 0; e < bmp->palette()->ncolors; ++e) {
		rf.putc(RGB_RED(bmp->palette()->clrs[e]));
		rf.putc(RGB_GREEN(bmp->palette()->clrs[e]));
		rf.putc(RGB_BLUE(bmp->palette()->clrs[e]));
	}
	for (; e < 256; ++e) {
		rf.putc(0);
		rf.putc(0);
		rf.putc(0);
	}
	
	rf.close();

	return(0);
}

/***
	Load the named .PCX file into an image.
	Currently only supports 256-color files.
***/
SBitmap* load_pcx(const char* filename) {
	SBitmap* bmp;
	RamFile rf;
	u32 flen;
	u32 xdim, ydim;
	u32 x, y;
	int c;
	int p;

	rf.open(filename, RAMFILE_READONLY);
	flen = rf.length();

	rf.seek(8);
	xdim = rf.getc();
	xdim += (rf.getc() << 8);
	ydim = rf.getc();
	ydim += (rf.getc() << 8);

	/* create a palettized (8-bit) xdim by ydim bitmap */
	bmp = new SBitmap(xdim, ydim, BITMAP_PALETTE);

	/* read in the palette */
	rf.seek_from_end(768);
	for (p = 0; p < 256; ++p) {
		u32 r, g, b;
		r = rf.getc();
		g = rf.getc();
		b = rf.getc();
		bmp->palette()->clrs[p] = RGB(r, g, b);
	}

	/* read the PCX image data */
	rf.seek(128);

	x = 0UL;
	y = 0UL;

	while (y < ydim) {
		c = rf.getc();
		if (c < 0) {
			delete bmp;
			return(NULL);
		}
		if (c > 192) {
			int interim = c - 192;
			c = rf.getc();
			for (p = interim; p > 0; --p) {
				if (x >= xdim) {
					x = 0UL;
					++y;
				}
				bmp->put_pixel_palette(x, y, c);
				++x;
			}
		} else {
			if (x >= xdim) {
				x = 0UL;
				++y;
			}
			bmp->put_pixel_palette(x, y, c);
			++x;
		}
	}

	rf.close();
	
	return(bmp);
}

/*** end pcx.cpp ***/
