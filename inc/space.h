/***

	space.h

	Color-space conversions.

	Copyright (c) 2002-2022 C. M. Street

***/
#ifndef SPACE_H
#define SPACE_H

/*** RGB <-> HSV ***/
extern void RGB_HSV(int r, int g, int b, int *h, int *s, int *v);
extern void HSV_RGB(int h, int s, int v, int *r, int *g, int *b);

/*** RGB <-> YIQ ***/
extern void RGB_YIQ(int r, int g, int b, int *y, int *i, int *q);
extern void YIQ_RGB(int y, int i, int q, int *r, int *g, int *b);

/*** RGB <-> YCbCr 601 ***/
extern void RGB_YCbCr_601(int r, int g, int b, int *y, int *cb, int *cr);
extern void YCbCr_601_RGB(int y, int cb, int cr, int *r, int *g, int *b, int brightness_adjust);

/***
	Interpolate between two colors.
	This uses integer arithmetic; the interpolation variable
		interp should be from 0 to 1000.
	0 means "entirely color 2", 1000 means "entirely color 1" --
	so interp can be thought of as the proportion of color 1
	(in thousandths) used in the color mixture.

	Though the variables are named r/g/b, this could just as
	easily be used to interpolate in other color spaces as well.
***/
extern void interpolate_color(int r1, int g1, int b1,
			int r2, int g2, int b2,
			int *ri, int *gi, int *bi,
			int interp);


/*** as above for RGBColor, so you don't have to break into individual components ***/
extern void interpolate_rgbcolor(RGBColor rgb1, RGBColor rgb2, RGBColor* rgb_out, int interp);

#endif  // SPACE_H
// end space.h
