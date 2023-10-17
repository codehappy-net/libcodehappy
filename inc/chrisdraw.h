/***

	chrisdraw.h

	Support for ChrisDrw-format graphics files.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifndef CHRISDRAW_H
#define CHRISDRAW_H

/*** Load a Chris Draw format file. ***/	
extern SBitmap* load_chrisdraw(const char* filename);

/*** The default EGA palette. ***/
extern const RGBColor ega_palette[16];

#endif  // CHRISDRAW_H
/* end chrisdraw.h */