/***

	pcx.h

	Support for PC Paintbrush .PCX image files

	A practically obsolete file format nowadays, but
	they are occasionally encountered.

	Copyright (c) 1997-2022 C. M. Street
	
***/
#ifndef _PCX_H
#define _PCX_H

/***
	Save the image bmp as a .PCX named filename.

	Requires that the SBitmap be palettized first.
***/
// TODO: allow save_pcx to quantize images with > 256 colors.
extern int save_pcx(SBitmap* bmp, const char* filename);

/***
	Load the named .PCX file into an image.
	Currently only supports 256-color files.
***/
extern SBitmap* load_pcx(const char* filename);

#endif  // _PCX_H
