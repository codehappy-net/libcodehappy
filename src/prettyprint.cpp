/***

	prettyprint.cpp

	Pretty-printing functions.

	Copyright (c) 2010-2022 C. M. Street.

***/
#include "libcodehappy.h"

/* Print buf to output stream f with a column width of col_width. */
void PrettyPrintF(char *buf, uint32_t col_width, FILE* f) {
	char *w;	// start of line
	char *w2;	// end of word
	char *w3;	// last position

	w = buf;
	w2 = buf;
	w3 = buf;
	while (1)
		{
		w2++;
		if (isspace((unsigned char)*w2) || *w2 == 0)
			{
			// a word
			if (w2 - w < (col_width - 1))
				w3 = w2;
			else
				{
				*w3 = '\000';
				fprintf(f, "%s\n", w);
				w = w3 + 1;
				}
			}
		if (!(*w2))
			break;
		}
	if (*w)
		fprintf(f, "%s\n", w);
}

void PrettyPrint(char *buf, uint32_t col_width) {
	PrettyPrintF(buf, col_width, stdout);
}

/* end prettyprint.cpp */