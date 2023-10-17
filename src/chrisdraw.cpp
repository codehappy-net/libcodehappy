/***

	chrisdraw.cpp

	Functions for reading images created/saved in the "ChrisDrw" (ChrisDraw) format.
	ChrisDrw was an old drawing program I wrote that allowed me to create pictures
	for programs. Only supported EGA mode 16 colors, 640 x 350 resolution, default
	palette.

	Copyright (c) 2014-2022 C. M. Street.

***/

const RGBColor ega_palette[16] = {
	EGA_00_BLACK,
	EGA_01_DK_BLUE,
	EGA_02_DK_GREEN,
	EGA_03_CYAN,
	EGA_04_DK_RED,
	EGA_05_PURPLE,
	EGA_06_BROWN,
	EGA_07_WHITE,
	EGA_08_GRAY,
	EGA_09_BRT_BLUE,
	EGA_10_BRT_GREEN,
	EGA_11_BRT_CYAN,
	EGA_12_BRT_RED,
	EGA_13_BRT_PURPLE,
	EGA_14_YELLOW,
	EGA_15_BRT_WHITE
};

#define	SCREEN_12_X	640
#define SCREEN_12_Y	350

static u32 __read_chrisdrw_arg(u32 nd, char** w) {
	char st[4] = { 0 };
	u32 ct = 0;

	while (*(*w) && *(*w) == '-') {
		++(*w);
		++ct;
	}
	if (*(*w) == '\000')
		return(0UL);
	ct %= nd;
	strncpy(st, *w, nd - ct);
	*w += (nd - ct);
	return (atoi(st));
}

SBitmap* load_chrisdraw(const char* filename) {
	SBitmap* bmp = NULL;
	RamFile rf;
	Scratchpad sp;

	bmp = new SBitmap(SCREEN_12_X, SCREEN_12_Y, BITMAP_DEFAULT);
	if (unlikely(is_null(bmp)))
		goto LErr;
	
	bmp->clear();

	rf.open(filename, RAMFILE_READONLY);

	until (rf.getline(sp) < 0) {
		char* line = (char*)sp.c_str();
		char* w;
		u32 x1, y1, x2, y2, clr_idx;

		w = line + 3;
		if (!strncmp(line, "LIN", 3)) {
			// draw a line
			clr_idx = __read_chrisdrw_arg(2, &w);
			x1 = __read_chrisdrw_arg(3, &w);
			y1 = __read_chrisdrw_arg(3, &w);
			x2 = __read_chrisdrw_arg(3, &w);
			y2 = __read_chrisdrw_arg(3, &w);
			bmp->line(x1, y1, x2, y2, ega_palette[clr_idx]);
		} else if (!strncmp(line, "BOX", 3)) {
			// draw a rectangle
			clr_idx = __read_chrisdrw_arg(2, &w);
			x1 = __read_chrisdrw_arg(3, &w);
			y1 = __read_chrisdrw_arg(3, &w);
			x2 = __read_chrisdrw_arg(3, &w);
			y2 = __read_chrisdrw_arg(3, &w);
			bmp->rect(x1, y1, x2, y2, ega_palette[clr_idx]);
		} else if (!strncmp(line, "CIR", 3)) {
			// draw a circle
			clr_idx = __read_chrisdrw_arg(2, &w);
			x1 = __read_chrisdrw_arg(3, &w);
			y1 = __read_chrisdrw_arg(3, &w);
			x2 = __read_chrisdrw_arg(3, &w);	// this is actually the radius
			bmp->circle(x1, y1, x2, ega_palette[clr_idx]);
		} else if (!strncmp(line, "PAN", 3)) {
			// paint (flood-fill)
			clr_idx = __read_chrisdrw_arg(2, &w);
			x1 = __read_chrisdrw_arg(3, &w);
			y1 = __read_chrisdrw_arg(3, &w);
			bmp->floodfill(x1, y1, ega_palette[clr_idx]);
		}
	}

	rf.close();
	bmp->alpha_opaque();
	return(bmp);

LErr:
	if (not_null(bmp))
		delete bmp;
	
	return nullptr;
}

// end chrisdraw.cpp