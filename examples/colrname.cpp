/***

	colrname.cpp

	Native + SDL app that displays an image, and gives the names and RGB colors 
	of points moused over.

	Copyright (c) 2022 Chris Street.

***/
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>

#define MAX_WIDTH	1200
#define MAX_HEIGHT	600

void main_loop(Display* display, void* user_data) {
	Font font(&font_swansea_bold);
	static u32 sz = font.font_size_for_height(36);
	SBitmap* bmp = (SBitmap*) user_data;
	SCoord rect;
	int mx = display->mouse_x(), my = display->mouse_y();
	static bool first = true;
	bmp->rect_bitmap(&rect);

	if (first) {
		codehappy_window_title("libcodehappy color picker");
		first = false;
	}

	display->bitmap()->clear();
	bmp->blit(display->bitmap(), SPoint(0, 0));
	if (rect.Contains(mx, my)) {
		RGBColor c = bmp->get_pixel(mx, my);
		const char* clrname;
		u32 dist;

		clrname = ClosestNameToRGBColor(c, &dist);
		if (!is_null(clrname)) {
			char str[256];
			sprintf(str, "(%d, %d) %s (#%02X%02X%02X, dist. %d)",
					mx, my,
					clrname,
					RGB_RED(c), RGB_GREEN(c), RGB_BLUE(c),
					dist);
			display->bitmap()->render_text(str, SCoord(0, bmp->height(), display->bitmap()->width() - 1, display->bitmap()->height() - 1), &font, C_WHITE, sz, CENTERED_BOTH);
		}
	}
}

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: colrname [image file]\n";
		return 1;
	}
	SBitmap* bmp = SBitmap::load_bmp(app_argv(1));
	if (is_null(bmp) || bmp->height() < 1) {
		std::cout << "Error loading bitmap " << app_argv(1) << std::endl;
		return 2;
	}
	if (bmp->height() > MAX_HEIGHT) {
		std::cout << "Bitmap too high, rescaling to height " << MAX_HEIGHT << " pixels.\n";
		if (!bmp->resize_and_replace(0, MAX_HEIGHT)) {
			std::cout << "Resize failed.\n";
			return 3;
		}
	}
	if (bmp->width() > MAX_WIDTH) {
		std::cout << "Bitmap too wide, rescaling to width " << MAX_WIDTH << " pixels.\n";
		if (!bmp->resize_and_replace(MAX_WIDTH, 0)) {
			std::cout << "Resize failed.\n";
			return 4;
		}
	}
	codehappy_main(main_loop, (void *)bmp, bmp->width(), bmp->height() + 40);
	return 0;
}

/*** end colrname.cpp ***/
