#include "libcodehappy.h"

/*** SVG rasterization/rendering ***/
#include "external/nanosvg.h"
#include "external/nanosvgrast.h"

int main(int argc, char* argv[])
{
	struct NSVGimage* image;
	struct NSVGrasterizer* rast = nsvgCreateRasterizer();
	SBitmap* bmp;
	float size;

	if (argc < 3) {
		puts("Please provide an SVG image and a size on the command line.\n");
		return(1);
	}

	size = atof(argv[2]);
	if (size <= 0.) {
		puts("Invalid size?");
		return(2);
	}

	image = nsvgParseFromFile(argv[1], "px", size);

	float scale = size / (float)image->width;
	u32 nw, nh;
	nw = (u32)floor(image->width * scale + 0.5);
	nh = (u32)floor(image->height * scale + 0.5);

	bmp = new SBitmap(nw, nh);

	nsvgRasterize(rast, image, 0, 0, scale, bmp->pixel_loc(0, 0), nw, nh, bmp->width() * 4);

	bmp->save_bmp("svgout.png");
	delete bmp;

	return(0);
}
