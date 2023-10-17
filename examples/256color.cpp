

#include "libcodehappy.h"

int main(int argc, char* argv[]) {
	int ncol = 256;
	if (argc < 2) {
		printf("Usage: 256color [image file] {opt. # colors}\n");
		return(1);
	}
	SBitmap* bmp = SBitmap::load_bmp(argv[1]);
	if (nullptr == bmp) {
		printf("Unable to load bitmap %s\n", argv[1]);
		return 1;
	}
	if (argc >= 3) {
		ncol = atoi(argv[2]);
		ncol = CLAMP(ncol, 2, 256);
		printf("Will quantize bitmap to %d colors.\n", ncol);
	}
	SBitmap *bmp256 = quantize_bmp_greedy(bmp, ncol, nullptr, dither_floyd_steinberg, colorspace_rgb);
	if (nullptr == bmp256) {
		printf("Error quantizing bitmap.\n");
		return 1;
	}
	bmp256->save_bmp("output.png");
	printf("Result output to output.png\n");
//	bmp256->save_bmp("output.gif");
//	printf("Result output to output.gif\n");
	return 0;
}
