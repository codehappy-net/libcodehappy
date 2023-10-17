
#include "libcodehappy.h"


int main(void) {
	Font font(font_swansea);
	SBitmap* render[3] = {
		font.render_cstr("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 16, false, nullptr),
		font.render_cstr("abcdefghijklmnopqrstuvwxyz", 16, false, nullptr),
		font.render_cstr("0123456789!&*@%^()#$-_+=:;", 16, false, nullptr)
	};
	u32 h = render[0]->height() + render[1]->height() + render[2]->height() + 12;
	u32 w = std::max(std::max(render[0]->width(), render[1]->width()), render[2]->width());
	SBitmap* bout = new SBitmap(w, h);
	// The clear() function sets the alpha channel to opaque, which is important for PNGs.
	bout->clear(C_WHITE);
	u32 y = 0;
	for (u32 e = 0; e < 3; ++e) {
		Font::blit(render[e], bout, 0, y, C_BLACK);
		y += render[e]->height();
		y += 6;
	}
	bout->save_bmp("output.png");
	return 0;
}