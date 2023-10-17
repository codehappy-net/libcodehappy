/***

	testhttp.cpp

	An example application using the libcodehappy functionality.

***/

#include "libcodehappy.h"

volatile bool wait = false;
char display_text[128];
Font font(font_oregon);

void fetch_success(RamFile* rf) {
	u8* buf = rf->buffer();
	wait = true;
	strncpy(display_text, (char*)buf, 127);
	display_text[127] = '\000';
	wait = false;
	delete rf;
}

void fetch_failure(RamFile* rf) {
	strcpy(display_text, "Whoops!");
}

void main_loop(Display* display, void* user_data) {
	static bool first = true;
	if (first) {
		first = false;
//		codehappy_URI_fetch_async("console.html", fetch_success, fetch_failure);
		codehappy_URI_fetch_async("../xword/puzzle.cgi?op=0", fetch_success, fetch_failure);
	}
	display->bitmap()->clear();
	if (strlen(display_text) > 0 && !wait) {
		SBitmap* blt = font.render_cstr(display_text, 16, false, nullptr);
		Font::blit(blt, display->bitmap(), 0, 0, C_WHITE);
	}
}

int main(void) {
	display_text[0] = '\000';
	codehappy_main(main_loop, nullptr, 800, 512);
	return 0;
}
