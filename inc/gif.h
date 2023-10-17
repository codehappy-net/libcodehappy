/***
	gif.h

	GIF save function

	Copyright (c) 1998-2022 C. M. Street
***/
#ifndef GIFSAVE_H
#define GIFSAVE_H

/*** Save the specified bmp as a .GIF to the named file. Returns 0 on success. ***/
/*** Note: this now works with bitmaps containing more than 256 colors; it will quantize the image in that case. ***/
extern int save_gif(SBitmap *bmp, const char* filename);

/*** Get the R/G/B individual components for a pixel in bmp. ***/
extern void getpixelbmp_components(SBitmap* bmp, int x, int y, int* r, int* g, int* b);

/*** Construct a color table (with population count) for the passed bitmap. ***/
extern void construct_coltable(SBitmap* bmp);

/*** Color population structure. ***/
struct cp {
	int r;
	int g;
	int b;
	int count;
};

#endif  // GIFSAVE_H
/* end __gif.h */
