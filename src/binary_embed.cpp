/***

	binary_embed.cpp

	Transform a binary file into a C source file that can be compiled into an executable.

	Copyright (c) 2014-2022, C. M. Street

***/
#include "libcodehappy.h"

#define	BIN_ERROR	1

u32 flength(const char* pname) {
	FILE *f;
	u32 ret;
	f = fopen(pname, "rb");
	ret = filelen(f);
	fclose(f);
	return ret;
}

int binary_to_c_src(const char* infile, const char* outfile) {
	int32_t flen;
	FILE* f;
	FILE* fout;
	unsigned char* cbuf;
	int e;
	int cline;

	flen = flength(infile);
	if (flen <= 0)
		return(BIN_ERROR);

	cbuf = new [unsigned char] flen;
	if (NULL == cbuf)
		return(BIN_ERROR);

	f = fopen(infile, "rb");
	fread(cbuf, sizeof(unsigned char), flen, f);
	fclose(f);

	fout = fopen(outfile, "w");

	fprintf(fout, "const unsigned char __data[%d] = {\n\t", flen);
	for (cline = 0, e = 0; e < flen; ++e) {
		fprintf(fout, "0x%X, ", cbuf[e]);
		++cline;
		if ((cline & 0xf) == 0)
			fprintf(fout, "\n\t");
	}
	fprintf(fout, "\n};\n");

	fclose(fout);

	delete [] cbuf;

	return(0);
}

/* end binary_embed.cpp */