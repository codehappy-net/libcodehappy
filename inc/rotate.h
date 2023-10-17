/***

	rotate.h

	Rotate by a specified angle. Floating point and integer arithmetic versions
	available.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __ROTATE_H
#define	__ROTATE_H

/***
	Rotate an integer point (x, y) counter-clockwise about the origin
	by theta radians. Uses floating point arithmetic.
***/
extern void rotate_xy(int x_in, int y_in, int* x_out, int* y_out, double theta);

/***
	Generate a sprite that is rotated (about the bitmap's center) theta radians clockwise.
	(This is rotated_sprite, not rotated_bmp, because any pixels that come from "outside" the original bitmap
	will be filled with full-transparency.) Uses floating-point arithmetic.
***/
extern SBitmap* rotated_sprite(SBitmap* bmp_in, double theta);

#endif  // __ROTATE_H
/* end rotate.h */
