/***
	parser.h

	Functions that may be helpful for parsers, lexers, and any
	code that processes text data.

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef PARSER_H_CODEHAPPY
#define PARSER_H_CODEHAPPY

/*** advance a char pointer past any whitespace */
extern char* advance_past_whitespace(char* ptr);

/*** advance a char pointer to the next whitespace ***/
extern char* advance_to_whitespace(char* ptr);

/*** advance a char pointer past any digits */
extern char* advance_past_digits(char* ptr);

/*** advance a char pointer to the next word (separated by spaces) ***/
extern char* advance_to_next_word(char* ptr);

/*** advance a char pointer to the next new line -- handles \n, \n\r, \r\n ***/
extern char* advance_to_next_line(char* ptr);

/*** advance a char pointer to the next occurence of any of the specified chars ***/
extern char* advance_to_next_matching_char(char* ptr, const char* candidates);

/*** advance a char pointer to the next alphabetic character (A-Z, a-z) ***/
extern char* advance_to_next_alpha(char* ptr);

/*** parse a (32-bit) integer at the current pointer position and advance
	past it -- behavior undefined for integers larger than 32 bits ***/
extern char* advance_and_parse_integer(char* ptr, int* i);

/*** as above, but parses a 64 bit integer. ***/
extern char* advance_and_parse_int64(char* ptr, i64* i);

/*** parse a double at the current pointer position and advance past it ***/
extern char* advance_and_parse_double(char* ptr, double* d);

/*** decrement a char pointer to the first previous whitespace ***/
extern char* backup_to_whitespace(char* ptr, char* bufstart);

/*** Skip past HTML tags ***/
extern char *SkipTags(char *w);

/*** translate "&amp;" into "&" ***/
extern void ClearAmp(char *w);

/*** currency symbols ***/
extern bool IsCurrencySymbol(char* w);

/*** trim whitespace from the end of a string ***/
extern void TrimEnd(char* text);

/*** double percent signs in a string -- note dest should be at least
	twice as long as src ***/
extern void DoubleEscapePercentSigns(char* src, char* dest);

/*** like atof() but ignores commas in the input ***/
extern double atof_ignore_commas(char* w);

/*** like atof() but ignores commas and currency symbols in the input ***/
extern double atof_currency(char *w);

/*** like atoi(), but ignores commas in input ***/
extern int atoi_ignore_commas(char *w);

/*** converts a float to a string, attempts to avoid round-off creating
	ugly formatting ***/
extern char *SmarterFloatFormat(float val);

/*** Are the inputs roughly the same order of magnitude?
	Note that inputs of mixed signs will return false. ***/
extern bool OrderMag(double v1, double v2);

/*** Return a properly URL-escaped version of the input string ***/
extern char *URLized(char *name);

/*** is the character i (0 - 255) valid in a URL? ***/
extern bool valid_URL_char(int i);

/*** is the extension a known HTML (or HTML-like) extension? ***/
extern bool is_HTML_extension(const char* n);

#endif  // PARSER_H_CODEHAPPY
/* end __parser.h */
