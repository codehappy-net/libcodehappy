/***

	stb_image_wrappers.c

	Implementation and wrappers for STB image libraries

***/

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION

/*** an STB-like header library for JPEG encoding! ***/
#define TJE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"
#include "stb_truetype.h"
#include "tiny_jpeg.h"

/*** Include NanoSVG, why not ***/
#define	NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define	NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

// end stb_image_wrappers.c