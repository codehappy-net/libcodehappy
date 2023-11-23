/***

	ttfembed.cpp

	Converts a TrueType font into a C++ file that can be added to the library.

	Free-to-use fonts can thus be included as built in fonts.

	C. M. Street

***/

#define	CODEHAPPY_NATIVE
#include <libcodehappy.h>

extern int load_ttf_font(const char* filename, ttfont* font_out);

int main(int argc, char* argv[]) {
	u32 len, e;
	char fname[256];
	int i = (int) RandU32Range(1000000, 9999999);

	if (argc < 2) {
		printf("Usage: ttfembed [input file]\n");
		return(0);
	}

	FILE* f = fopen(argv[1], "rb");
	len = filelen(f);
	fclose(f);

	Font cfont(argv[1]);
	ttfont* font = ttf_from_font(&cfont);

	sprintf(fname, "%s.cpp", (char *)strip_filename_extension(argv[1]));
	f = fopen(fname, "w");

	fprintf(f, "static unsigned char __fontdata%d[] = {\n", i);

	for (e = 0; e < len; ++e) {
		fprintf(f, "0x%02X, ", font->data[e]);
		if ((e & 63) == 63)
			fprintf(f, "\n");
	}

	fprintf(f, "\n};\n\n");

	fprintf(f, "ttfont font_%s = {\n", (char *)strip_filename_extension(argv[1]));
	fprintf(f, " { NULL, __fontdata%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d }, ",
			i,
			font->info.fontstart,
			font->info.numGlyphs,
			font->info.loca,
			font->info.head,
			font->info.glyf,
			font->info.hhea,
			font->info.hmtx,
			font->info.kern,
			font->info.index_map,
			font->info.indexToLocFormat);
	fprintf(f, "\n   __fontdata%d };\n\n\n", i);
	fclose(f);

	return 0;
}

