/***

	colors.h

	Useful color definitions.

	For color space conversion functions, check out space.cpp.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __COLORS_H
#define __COLORS_H

/* create an RGB 32-bit color, each component 8 bits, blue LSB, (zero) alpha MSB */
#ifndef RGB
#define	RGB(r, g, b)	((((r) & 0xff) << 16) + (((g) & 0xff) << 8) + ((b) & 0xff))
#endif

/* As above, with the alpha channel set too */
#ifndef RGBA
#define	RGBA(r, g, b, a)	((RGB(r, g, b)) | ((a) << 24))
#endif

/* add an alpha channel value to an RGB color */
#ifndef ADD_ALPHA
#define	ADD_ALPHA(rgb, a)	((rgb) | ((a) << 24))
#endif

/* indicates an error */
#define	INVALID_COLOR	((RGBColor)(-1))

/* get a string name for a color, if we know it */
extern const char* NameFromRGBColor(RGBColor rgb);

/* get the name of the closest-matching color, and return the distance in d_out if non-null */
extern const char* ClosestNameToRGBColor(RGBColor rgb, u32* d_out);

/* get an RGBColor from a string name */
extern RGBColor RGBColorFromName(const char* name);

/* make an RGBColor lighter or darker */
extern RGBColor RGBColorDarkenBrighten(RGBColor rgb, double factor);
extern RGBColor RGBColorDarkenBrightenRational(RGBColor rgb, u32 num, u32 den);

/* extract the grayscale level of the given color */
extern u32 RGBColorGrayscaleLevel(RGBColor rgb);

/* alternate grayscale level function -- weighted red 33%, green 56%, blue 11%. */
extern u32 RGBColorGrayscaleLevelWeighted(RGBColor rgb);

/* convert an RGB color to gray scale */
extern u32 RGBColorToGrayscale(RGBColor rgb);

/* Alternate grayscale conversion -- weighted red 33%, green 56%, blue 11%. */
extern u32 RGBColorToGrayscaleWeighted(RGBColor rgb);

/* check that the array of color names is properly sorted */
extern void debug_verify_color_name_sort(void);

/*** Returns the color for a given temperature (in Kelvin). The most interesting range for imaging purposes is
	about [1500 K, 15000 K]. ***/
extern RGBColor temperaturecolor(u32 kelvin);

/*** Adjust the color so that the highest-intensity component of the color so it is equal to amount (0-255). Preserves alpha channel. ***/
extern RGBColor modify_intensity(RGBColor c, uint amount);

/*** As modify_intensity, but changes the color to maximum intensity ***/
extern RGBColor full_intensity(RGBColor c);

/*** Ensure that maximum intensity of the color c is at most max_intensity; scale components down evenly if necessary. ***/
extern RGBColor fade_intensity(RGBColor c, uint max_intensity);

/* our array of known color names */
struct ColorNames {
	const char* name;
	RGBColor rgb;
};

extern const ColorNames rgbcolor_names[];
extern const int number_named_colors;

#endif  // __COLORS_H
// end __colors.h
