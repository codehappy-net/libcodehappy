/***

	rotate.cpp

	Rotate by a specified angle. Floating
	point and integer arithmetic versions
	available.

	Copyright (c) 2014-2022, C. M. Street

***/
#include "libcodehappy.h"
#include <math.h>

/***
	Rotate an integer point (x, y) counter-clockwise about the origin
	by theta radians. Uses floating point arithmetic.

	Simple linear algebra:

	Basis vectors (1, 0) and (0, 1) rotate to (cos theta, sin theta), (-sin theta, cos theta).
	Convert to the new basis by the matrix multiplication:

		|  cos theta	-sin theta  | | x |
		|  sin theta	 cos theta  | | y |
***/
void rotate_xy(int x_in, int y_in, int* x_out, int* y_out, double theta) {
	*x_out = (int)floor(cos(theta) * x_in - sin(theta) * y_in + 0.5);
	*y_out = (int)floor(sin(theta) * x_in + cos(theta) * y_in + 0.5);
}

// TODO: integer versions, using the fast trig lookups

/***
	Generate a sprite that is rotated (about the bitmap's center) theta radians clockwise.
	(This is rotated_sprite, not rotated_bmp, because any pixels that come from "outside" the original bitmap
	will be filled with full-transparency.)
***/
SBitmap* rotated_sprite(SBitmap* bmp_in, double theta) {
	SBitmap* bmpret;
	int x, y;
	int rx, ry;
	int xh, yh;

	bmpret = new SBitmap(bmp_in->width(), bmp_in->height(), BITMAP_DEFAULT);
	bmpret->fill_alpha_channel(ALPHA_TRANSPARENT);

	xh = bmp_in->width() >> 1;
	yh = bmp_in->height() >> 1;
	
	for (y = 0; y < bmp_in->height(); ++y) {
		for (x = 0; x < bmp_in->width(); ++x) {
			// calculate the coordinates relative to the center of the bitmap
			int xc, yc;

			xc = (x - xh);
			yc = (bmp_in->height() - (y + 1));
			yc = (yc - yh);

			// rotate
			rotate_xy(xc, yc, &rx, &ry, theta);

			// now re-translate the coordinates
			rx += xh;
			ry += yh;
			ry = (bmp_in->height() - (ry + 1));

			if (pixel_ok(bmp_in, rx, ry))
				bmpret->put_pixel(x, y, bmp_in->get_pixel(rx, ry));
		}
	}

	return(bmpret);
}

/*** end rotate.c ***/
