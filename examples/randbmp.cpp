#define CODEHAPPY_NATIVE
#include <libcodehappy.h>


/* Increasing direction, columns not inverted. */
static void comet1(SBitmap* bmp) {
	const int x = bmp->width();
	const int y = bmp->height();
	bmp->put_pixel(0, 0, RandColor());
	for (int e = 1; e < (x - 1) + y; ++e) {
		for (int row = 0; row < y; ++row) {
			RGBColor cr[2];
			int col = (e - row);
			int c = 0;
			if (!pixel_ok(bmp, col, row))
				continue;
			if (pixel_ok(bmp, col, row - 1)) {
				cr[c++] = bmp->get_pixel(col, row - 1);
			}
			if (pixel_ok(bmp, col - 1, row)) {
				cr[c++] = bmp->get_pixel(col - 1, row);
			}
			if (0 == c)
				continue;
			if (1 == c) {
				no_op;
			} else {
				cr[0] = MAKE_RGB((RGB_RED(cr[0]) + RGB_RED(cr[1])) / 2,
						 (RGB_GREEN(cr[0]) + RGB_GREEN(cr[1])) / 2,
						 (RGB_BLUE(cr[0]) + RGB_BLUE(cr[1])) / 2);
			}
			perturb_color_randomly_even(&cr[0]);
			bmp->put_pixel(col, row, cr[0]);
		}
	}
}

/* Increasing direction, columns inverted. */
static void comet2(SBitmap* bmp) {
	const int x = bmp->width();
	const int y = bmp->height();
	bmp->put_pixel(x - 1, 0, RandColor());
	for (int e = 1; e < (x - 1) + y; ++e) {
		for (int row = 0; row < y; ++row) {
			RGBColor cr[2];
			int col = (x - 1) - (e - row);
			int c = 0;
			if (!pixel_ok(bmp, col, row))
				continue;
			if (pixel_ok(bmp, col, row - 1)) {
				cr[c++] = bmp->get_pixel(col, row - 1);
			}
			if (pixel_ok(bmp, col + 1, row)) {
				cr[c++] = bmp->get_pixel(col + 1, row);
			}
			if (0 == c)
				continue;
			if (1 == c) {
				no_op;
			} else {
				cr[0] = MAKE_RGB((RGB_RED(cr[0]) + RGB_RED(cr[1])) / 2,
						 (RGB_GREEN(cr[0]) + RGB_GREEN(cr[1])) / 2,
						 (RGB_BLUE(cr[0]) + RGB_BLUE(cr[1])) / 2);
			}
			perturb_color_randomly_even(&cr[0]);
			bmp->put_pixel(col, row, cr[0]);
		}
	}
}

/* Decreasing direction, columns not inverted. */
static void comet3(SBitmap* bmp) {
	const int x = bmp->width();
	const int y = bmp->height();
	bmp->put_pixel(x - 1, y - 1, RandColor());
	for (int e = (x - 1) + (y - 1) - 1; e >= 0; --e) {
		for (int row = 0; row < y; ++row) {
			RGBColor cr[2];
			int col = (e - row);
			int c = 0;
			if (!pixel_ok(bmp, col, row))
				continue;
			if (pixel_ok(bmp, col, row + 1)) {
				cr[c++] = bmp->get_pixel(col, row + 1);
			}
			if (pixel_ok(bmp, col + 1, row)) {
				cr[c++] = bmp->get_pixel(col + 1, row);
			}
			if (0 == c)
				continue;
			if (1 == c) {
				no_op;
			} else {
				cr[0] = MAKE_RGB((RGB_RED(cr[0]) + RGB_RED(cr[1])) / 2,
						 (RGB_GREEN(cr[0]) + RGB_GREEN(cr[1])) / 2,
						 (RGB_BLUE(cr[0]) + RGB_BLUE(cr[1])) / 2);
			}
			perturb_color_randomly_even(&cr[0]);
			bmp->put_pixel(col, row, cr[0]);
		}
	}
}

/* Decreasing direction, columns inverted. */
static void comet4(SBitmap* bmp) {
	const int x = bmp->width();
	const int y = bmp->height();
	bmp->put_pixel(0, y - 1, RandColor());
	for (int e = (x - 1) + (y - 1) - 1; e >= 0; --e) {
		for (int row = 0; row < y; ++row) {
			RGBColor cr[2];
			int col = (x - 1) - (e - row);
			int c = 0;
			if (!pixel_ok(bmp, col, row))
				continue;
			if (pixel_ok(bmp, col, row + 1)) {
				cr[c++] = bmp->get_pixel(col, row + 1);
			}
			if (pixel_ok(bmp, col - 1, row)) {
				cr[c++] = bmp->get_pixel(col - 1, row);
			}
			if (0 == c)
				continue;
			if (1 == c) {
				no_op;
			} else {
				cr[0] = MAKE_RGB((RGB_RED(cr[0]) + RGB_RED(cr[1])) / 2,
						 (RGB_GREEN(cr[0]) + RGB_GREEN(cr[1])) / 2,
						 (RGB_BLUE(cr[0]) + RGB_BLUE(cr[1])) / 2);
			}
			perturb_color_randomly_even(&cr[0]);
			bmp->put_pixel(col, row, cr[0]);
		}
	}
}

void comet_plasma(SBitmap* bmp_in, bool merge, bool dir, bool rowcol) {
	SBitmap* bmp = new SBitmap(bmp_in->width(), bmp_in->height());
	bmp->clear();	// fills alpha channel
	if (dir) {
		if (rowcol) {
			comet1(bmp);
		} else {
			comet2(bmp);
		}
	} else {
		if (rowcol) {
			comet3(bmp);
		} else {
			comet4(bmp);
		}
	}
	if (merge) {
		int shift_x, shift_y;
		int xx, yy;
		shift_x = 0; // (int)RandU16();
		shift_y = 0; // (int)RandU16();
		for (int y = 0; y < bmp_in->height(); ++y) {
			for (int x = 0; x < bmp_in->width(); ++x) {
				RGBColor cr[2];
				cr[0] = bmp_in->get_pixel(x, y);
				xx = (x + shift_x) % bmp_in->width();
				yy = (y + shift_y) % bmp_in->height();
				cr[1] = bmp->get_pixel(xx, yy);
				cr[0] = MAKE_RGB((RGB_RED(cr[0]) + RGB_RED(cr[1])) / 2,
						 (RGB_GREEN(cr[0]) + RGB_GREEN(cr[1])) / 2,
						 (RGB_BLUE(cr[0]) + RGB_BLUE(cr[1])) / 2);
				bmp_in->put_pixel(x, y, cr[0]);
			}
		}
	} else {
		bmp->blit(bmp_in, SPoint(0, 0));
		delete bmp;
	}
}

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: randbmp [width] {height}\n";
		std::cout << "Specifying only width outputs a square bitmap.\n";
		return 1;
	}

	int x, y;
	x = atoi(app_argv(1));
	if (app_argc() >= 3)
		y = atoi(app_argv(2));
	else
		y = x;
	std::cout << "Outputting " << x << " x " << y << " random color gradient image to 'out.png'.\n";

	SBitmap* bmp = new SBitmap(x, y);
#ifdef TILES
	SBitmap* tile = new SBitmap(20, 20);
	int tx, ty;
	for (ty = 0; ty < y; ty += 20) {
		for (tx = 0; tx < x; tx += 20) {
			comet_plasma(tile, false, RandBool(), RandBool());
			comet_plasma(tile, true, RandBool(), RandBool());
			tile->blit(bmp, SPoint(tx, ty));
		}
	}
#else
	comet_plasma(bmp, false, RandBool(), RandBool());
	comet_plasma(bmp, true, RandBool(), RandBool());
#endif

	bmp->save_bmp("out.png");

	return 0;
}

/* end randbmp.cpp */