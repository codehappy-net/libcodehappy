/***
	parser.cpp

	Functions that may be helpful for parsers, lexers, and any
	code that processes text data.

	Copyright (c) 2014-2022 Chris Street.

***/

#include "libcodehappy.h"

// TODO: Unicode versions

/*** advance a char pointer past any whitespace */
char* advance_past_whitespace(char* ptr) {
	while (*ptr && isspace(*((unsigned char*)ptr)))
		++ptr;
	return(ptr);
}

/*** advance a char pointer to the next whitespace ***/
char* advance_to_whitespace(char* ptr) {
	while (*ptr && !isspace(*((unsigned char*)ptr)))
		++ptr;
	return(ptr);
}

/*** advance a char pointer past any digits */
char* advance_past_digits(char* ptr) {
	while (*ptr && isdigit(*((unsigned char*)ptr)))
		++ptr;
	return(ptr);
}

/*** decrement a char pointer to the first previous whitespace ***/
char* backup_to_whitespace(char* ptr, char* bufstart) {
	while (ptr >= bufstart && !isspace(*((unsigned char*)ptr)))
		--ptr;
	return(ptr);
}

/*** advance a char pointer to the next word (separated by spaces) ***/
char* advance_to_next_word(char* ptr) {
	ptr = advance_to_whitespace(ptr);
	return (advance_past_whitespace(ptr));
}

/*** advance a char pointer to the next new line -- handles \n, \n\r, \r\n ***/
char* advance_to_next_line(char* ptr) {
	while (*ptr)
		{
		if (*ptr == '\n')
			{
			++ptr;
			if (*ptr == '\r')
				return(ptr + 1);
			return(ptr);
			}
		else if (*ptr == '\r')
			{
			++ptr;
			if (*ptr == '\n')
				return(ptr + 1);
			continue;
			}
		++ptr;
		}

	return(ptr);
}

/*** advance a char pointer to the next occurence of any of the specified chars ***/
char* advance_to_next_matching_char(char* ptr, const char* candidates) {
	while (*ptr)
		{
		if (strchr(candidates, *ptr) != NULL)
			return(ptr);
		++ptr;
		}
	return(ptr);
}

/*** advance a char pointer to the next alphabetic character (A-Z, a-z) ***/
char* advance_to_next_alpha(char* ptr) {
	while (*ptr && !isalpha(*((unsigned char*)ptr)))
		++ptr;
	return(ptr);
}

/*** parse a (32-bit) integer at the current pointer position and advance
	past it -- behavior undefined for integers larger than MAX_INT ***/
char* advance_and_parse_integer(char* ptr, int* i) {
	if (i != NULL)
		(*i) = atoi(ptr);
	ptr = advance_past_whitespace(ptr);
	if (*ptr == '+' || *ptr == '-')
		++ptr;
	while ((*ptr) >= '0' && (*ptr) <= '9')
		++ptr;
	return(ptr);
}

/*** as above, but parses a 64 bit integer. ***/
char* advance_and_parse_int64(char* ptr, i64* i) {
	if (i != NULL)
		(*i) = strtoll(ptr, NULL, 0);
	ptr = advance_past_whitespace(ptr);
	if (*ptr == '+' || *ptr == '-')
		++ptr;
	while ((*ptr) >= '0' && (*ptr) <= '9')
		++ptr;
	return(ptr);
}

/*** parse a double at the current pointer position and advance past it ***/
char* advance_and_parse_double(char* ptr, double* d) {
	if (d != NULL)
		(*d) = atof(ptr);
	ptr = advance_past_whitespace(ptr);
	if (*ptr == '+' || *ptr == '-')
		++ptr;
	while ((*ptr) >= '0' && (*ptr) <= '9')
		++ptr;
	if ((*ptr) == '.')
		++ptr;
	while ((*ptr) >= '0' && (*ptr) <= '9')
		++ptr;
	if (toupper(*ptr) == 'E' || toupper(*ptr) == 'F')
		{
		++ptr;
		if (*ptr == '+' || *ptr == '-')
			++ptr;
		while ((*ptr) >= '0' && (*ptr) <= '9')
			++ptr;
		}
	return(ptr);
}

/*** Skip past HTML tags ***/
char *SkipTags(char *w) {
	// TODO: this needs to handle quoted text between <> characters!
	char *wl;

	while (*w == '<')
		{
		wl = w;
		w = strchr(w, '>');
		if (w == NULL)
			return(wl);
		w++;
		while (*w && isspace(*w))
			++w;
		if (!(*w))
			return(w);
		}

	return(w);
}

/*** translate "&amp;" into "&" ***/
void ClearAmp(char *w) {
	char *x;

	x = strstr(w, "&amp;");
	while (x != NULL)
		{
		memmove(x + 1, x + 5, strlen(x + 4));
		x = strstr(w, "&amp;");
		}
}

/*** currency symbols ***/
bool IsCurrencySymbol(char* w) {
	// use u_iscurrency() for more general Unicode currency symbols
	switch ((unsigned char)*w)
		{
	case '$':
	case 0xA3: // '£'
	case 0x80: // '€':
	case 0xA5: // '¥':
		return(true);
		}

	return(false);
}

/*** trim whitespace from the end of a string ***/
void TrimEnd(char *text) {
	char *w;

	w = text + strlen(text);
	w--;
	while (isspace(*w) && w >= text)
		{
		*w = '\000';
		--w;
		}
}

/*** double percent signs in a string -- note d should be at least
	twice as long as u ***/
void DoubleEscapePercentSigns(char* u, char* d) {
	int e;
	int f;

	e = 0;
	f = 0;
	while (u[e])
		{
		d[f] = u[e];
		f++;
		if (u[e] == '%')
			{
			d[f] = u[e];
			f++;
			}
		e++;
		}
	d[f] = '\000';
}

/*** like atof() but ignores commas in the input ***/
double atof_ignore_commas(char* w) {
	char *w2;
	char buf[128];
	int i;

	w2 = w;
	i = 0;
	forever
		{
		if (i < 127)
			{
			if (*w2 >= '0' && *w2 <= '9')
				{
				buf[i++] = *w2;
				w2++;
				continue;
				}
			if (*w2 == '.')
				{
				buf[i++] = *w2;
				w2++;
				continue;
				}
			if (*w2 == ',')
				{
				w2++;
				continue;
				}
			}
		buf[i] = 0;
		break;
		}

	return atof(buf);
}

/*** like atof() but ignores commas and currency symbols in the input ***/
double atof_currency(char *w) {
	double mul;

	mul = 1.0;
	w = advance_past_whitespace(w);
	if (*w == '-')
		mul = -1.0;
	while (IsCurrencySymbol(w))
		w++;
	w = advance_past_whitespace(w);

	return atof_ignore_commas(w) * mul;
}

/*** like atoi(), but ignores commas in input ***/
int atoi_ignore_commas(char *w) {
	int dd;
	bool neg = false;

	if (*w == '-')
		{
		neg = true;
		w++;
		}
	dd = atoi(w);
	while (*w)
		{
		if (*w == ',')
			{
			dd *= 1000;
			++w;
			dd += atoi(w);
			continue;
			}
		else if (!isdigit(*w))
			break;
		++w;
		}		

	if (neg)
		dd = -dd;

	return(dd);
}

static void _IncrementIntString(char *s) {
	int i;
	char buf[256];

	i = atoi(s);
	sprintf(buf, "%d", (i + 1));
	strcpy(s, buf);
}

char __sffbuf[256];
char *SmarterFloatFormat(float val) {
	/* avoids silly formating like 14.3 -> 14.300003 and 3.8 -> 3.79998 */
	char *q;
	char *dp;
	int i;
	char buf1[16];
	char buf2[16];

	sprintf(__sffbuf, "%f", val);
	dp = strchr(__sffbuf, '.');
	if (!dp)
		return(__sffbuf);
	
	q = strstr(dp, "000");
	if (q != NULL)
		{
		i = atoi(q);
		if (i < 10)
			*q = 0;
		}
	q = strstr(dp, "001");
	if (q != NULL)
		{
		i = atoi(q);
		if (i == 1)
			*q = 0;
		}
	q = strstr(dp, "999");
	if (q != NULL)
		{
		i = atoi(q);
		sprintf(buf1, "%d", i);
		sprintf(buf2, "%d", i+9);
		if (strlen(buf2) == strlen(buf1) + 1)
			{
			*q = 0;
			if (q == dp + 1)
				{
				*dp = 0;
				_IncrementIntString(__sffbuf);
				}
			else
				{
				_IncrementIntString(dp + 1);
				}
			}
		}
	if (*(dp + 1) == 0)
		*dp = 0;

	return(__sffbuf);
}

/*** Are the inputs roughly the same order of magnitude?
	Note that inputs of mixed signs will return false. ***/
bool OrderMag(double v1, double v2) {
	// note, mixed signs will return false
	v1 /= v2;
	if (v1 < 0.5 || v1 > 2.0)
		return(false);
	return(true);
}

/*** Return a properly URL-escaped version of the input string ***/
// TODO: not thread-safe
static char _urlname[1024];
char *URLized(char *name) {
	int cn;
	int cc;
	const char hd[] = "0123456789ABCDEF";

	cn = 0;
	cc = 0;
	while (cc < 1020 && name[cn])
		{
		if ((name[cn] >= 'a' && name[cn] <= 'z') || (name[cn] >= 'A' && name[cn] <= 'Z') ||
				(name[cn] >= '0' && name[cn] <= '9'))
			{
			_urlname[cc++] = name[cn];
			}
		else if (name[cn] == ' ')
			{
			_urlname[cc++] = '+';
			}
		else if (name[cn] == '\t')
			{
			}
		else
			{
			_urlname[cc++] = '%';
			_urlname[cc++] = hd[(name[cn]) >> 4];
			_urlname[cc++] = hd[name[cn] & 0x0f];
			}
		cn++;
		}
	_urlname[cc] = '\000';
	
	return(_urlname);
}

bool valid_URL_char(int i) {
	if (i < 0 || i > 255)
			return false;
	
	if (i >= 'a' && i <= 'z')
			return true;
	
	if (i >= 'A' && i <= 'Z')
			return true;
	
	if (i >= '0' && i <= '9')
			return true;
	
	return (i == ':' || i == '/' || i == '.' || i == '_' || i == '-' ||
					i == '&' || i == '?' || i == '%' || i == '+' ||
					i == '=' || i == '~' || i == '!' || i == '@' ||
					i == '$' || i == ',' || i == ';');
}

bool is_HTML_extension(const char* n) {
	if (!__stricmp(n, "htm"))
			return true;
	if (!__stricmp(n, "html"))
			return true;
	if (!__stricmp(n, "shtml"))
			return true;
	if (!__stricmp(n, "xhtml"))
			return true;
	if (!__stricmp(n, "bml"))
			return true;
	if (!__stricmp(n, "php"))
			return true;
	if (!__stricmp(n, "asp"))
			return true;
	if (!__stricmp(n, "aspx"))
			return true;
	if (!__stricmp(n, "phtml"))
			return true;
	if (!__stricmp(n, "cgi"))
			return true;
	return false;
}

/* end parser.cpp */
