/***

	prettyprint.h
	
	Pretty-printing functions.

	See also libtable, which lives in extern.cpp.	

	Copyright (c) 2010-2022 C. M. Street.

***/
#ifndef PRETTYPRINT_H
#define PRETTYPRINT_H

/* Print buf to output stream f with a column width of col_width. */
extern void PrettyPrintF(char *buf, uint32_t col_width, FILE* f);

/* Print buf to standard output with a column width of col_width. */
extern void PrettyPrint(char *buf, uint32_t col_width);

#endif  // PRETTYPRINT_H
// end prettyprint.h