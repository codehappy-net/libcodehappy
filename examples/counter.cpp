/***

	counter.cpp

	An SDL app that simply displays the value of the hardware counter.

	Copyright (c) 2022 Chris Street.

***/
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "libcodehappy.h"

void main_loop(Display* display, void* user_data) {
	Font font(&font_swansea_bold);
	u64 val;
	char d[64];
	SCoord rect;
	static bool first = true;

	if (first) {
		codehappy_window_title("Hardware Counter");
		first = false;
	}

	display->bitmap()->clear(C_WHITE);
	display->bitmap()->rect_bitmap(&rect);
	val = hardware_counter();
	sprintf(d, "%" PRIu64, val);
	display->bitmap()->render_text(d, rect, &font, C_BLACK, 80, CENTERED_VERT | ALIGN_LEFT);
}

int app_main() {
	Font font(&font_swansea_bold);
	u32 sz = font.font_size_for_height(80);
	SBitmap* test = font.render_cstr("80808080808000080", sz, false, nullptr);
	u32 w = test->width();
	delete test;
	codehappy_main(main_loop, nullptr, w + 10, 100);
	return 0;
}

/* end counter.cpp */