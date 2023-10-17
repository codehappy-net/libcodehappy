/***

	unicode.cpp

	Unicode implementation for libcodehappy. Includes format conversions, ctype functions, 
	anti-spoofing measures, etc.

	Copyright (c) 2014-2022 Chris Street

***/
#include <string.h>
#include <stdlib.h>
#include "libcodehappy.h"

u32 ustrlen(const ustring ustr) {
	ustring ustr_e = ustr;

	while (*ustr_e)
		++ustr_e;

	return(ustr_e - ustr);
}

u32 ustrcpy(ustring ustr_dest, ustring ustr_src, u32 max_chars_dest) {
	ustring ustr_e = ustr_dest + max_chars_dest;

	if (unlikely(max_chars_dest) == 0UL)
		return(0UL);

	while (ustr_dest < ustr_e) {
		*ustr_dest = *ustr_src;
		++ustr_dest;
		++ustr_src;
	}

	if (ustr_dest < ustr_e)
		*ustr_dest = 0;
	else
		*(ustr_dest - 1) = 0;

	return(ustr_e - ustr_dest);
}

u32 ustrncpy(ustring ustr_dest, ustring ustr_src, u32 nchars, u32 max_chars_dest) {
	if (max_chars_dest < nchars)
		return ustrcpy(ustr_dest, ustr_src, max_chars_dest);

	return ustrcpy(ustr_dest, ustr_src, nchars);
}

i32 ustrcmp(ustring ustr1, ustring ustr2) {
	forever {
		if (*ustr1 != *ustr2)
			return ((i32)(*ustr1) - (i32)(*ustr2));
		if (*ustr1 == 0)
			break;
		++ustr1;
		++ustr2;
	}

	return(0);
}

i32 ustricmp(ustring ustr1, ustring ustr2) {
	forever {
		uch l1, l2;
		l1 = u_tolower(*ustr1);
		l2 = u_tolower(*ustr2);
		if (l1 != l2)
			return ((i32)(l1) - (i32)(l2));
		if (*ustr1 == 0)
			break;
		++ustr1;
		++ustr2;
	}

	return(0);
}

i32 ustrncmp(ustring ustr1, ustring ustr2, u32 nchars) {
	const ustring ustr1_e = ustr1 + nchars;
	
	while (ustr1 < ustr1_e) {
		if (*ustr1 != *ustr2)
			return ((i32)(*ustr1) - (i32)(*ustr2));
		if (*ustr1 == 0)
			break;
		++ustr1;
		++ustr2;
	}

	return(0);
}

u32 ustrcat(ustring ustr, ustring ustr_cat, u32 max_chars_dest) {
	const u32 len1 = ustrlen(ustr);

	return ustrcpy(ustr + len1, ustr_cat, max_chars_dest - len1);	
}

ustring ustrdup(const ustring ustr) {
	const u32 len = ustrlen(ustr);
	ustring uret = NEW_USTR(len);
	NOT_NULL_OR_RETURN(uret, NULL);
	ustrcpy(uret, ustr, len + 1);
	return(uret);
}

ustring ustrstr(ustring haystack, const ustring needle) {
	const u32 lencomp = ustrlen(needle);

	while (*haystack) {
		if (!ustrncmp(haystack, needle, lencomp))
			return(haystack);
		++haystack;
	}

	return(NULL);
}

ustring ustrchr(ustring haystack, const uch needle) {
	while (*haystack)
		{
		if ((*haystack) == needle)
			return (haystack);
		++haystack;
		}

	return(NULL);
}

/*** Convert a C one-byte char string into a Unicode string. ***/
ustring cstr2ustr(const char* c_str) {
	const u32 len = strlen(c_str);
	ustring uret = NEW_USTR(len);
	char* w = (char *)c_str;
	uch* u;
	NOT_NULL_OR_RETURN(uret, NULL);

	u = uret;
	while (*w)
		{
		*u = (uch)*w;
		++w;
		++u;
		}
	*u = 0;
	
	return(uret);
}

/*** Convert a Unicode UTF-32 string into a 8-bit character C string. Replace any characters outside
	the 8-bit range with the passed in char "c" (or removed if c < 0). If replace is non-NULL, it will return how many characters
	outside the 8-bit range were encountered and replaced/removed. ***/
/* Note that this does preserve the non-ASCII characters from 0x80 to 0xFF. */
char* ustr2str(const ustring u_str, int c, u32* replace) {
	const u32 len = ustrlen(u_str);
	char* cret = NEW_STR(len);
	uch* u = (uch *)u_str;
	char* w;
	NOT_NULL_OR_RETURN(cret, NULL);

	if (not_null(replace))
		*replace = 0UL;

	w = cret;
	while (*u) {
		if (*u >= 0x100) {
			if (not_null(replace))
				(*replace)++;
			if (c >= 0) {
				*(unsigned char *)w = c;
			} else {
				++u;
				continue;
			}
		} else {
			*w = (*u);
		}
		++w;
		++u;
	}
	*w = 0;

	return(cret);
}

/*** Helper function: returns true iff the UTF-8 byte is a continuation byte. ***/
static bool is_continuation_byte(uchar utf8_ch) {
	return ((utf8_ch & HIGH_2) == HIGH_1);
}

/*** Helper function: returns the number of continuation bytes expected, from the
		first UTF-8 byte in an encoding. ***/
static uint expected_continuation_bytes(uchar utf8_ch) {
	if (iszero(utf8_ch & HIGH_1))
		return(0);
	if ((utf8_ch & HIGH_3) == HIGH_2)
		return(1);
	if ((utf8_ch & HIGH_4) == HIGH_3)
		return(2);
	if ((utf8_ch & HIGH_5) == HIGH_4)
		return(3);
#ifdef OLD_UTF8
	/* These were allowed in the original UTF-8 specification, however since
		2003 all UTF-8 encodings are expected to be at most 4 bytes long. */
	if ((utf8_ch & HIGH_6) == HIGH_5)
		return(4);
	if ((utf8_ch & HIGH_7) == HIGH_6)
		return(5);
#endif

	/* invalid encoding! */
	return(UTF_INVALID);
}

/*** Helper function: decode a 2-byte UTF-8 code. Returns UTF_INVALID for invalid encoding. ***/
static u32 utf8_2byte(uchar* w) {
	u32 ret;

	if (!is_continuation_byte(*(w + 1)))
		return(UTF_INVALID);
	ret = (((*w) & LOW_5) << 6) + ((*(w + 1)) & LOW_6);
	if (ret < 0x80)
		return(UTF_INVALID);
	return(ret);
}

/*** Helper function: decode a 3-byte UTF-8 code. Returns UTF_INVALID for invalid encoding. ***/
static u32 utf8_3byte(uchar* w) {
	u32 ret;
	if (!is_continuation_byte(*(w + 1)))
		return(UTF_INVALID);
	if (!is_continuation_byte(*(w + 2)))
		return(UTF_INVALID);
	ret = (((*w) & LOW_4) << 12) + (((*(w + 1)) & LOW_6) << 6) +
		((*(w + 2)) & LOW_6);
	if (ret < 0x800)
		return(UTF_INVALID);
	return(ret);
}

/*** Helper function: decode a 4-byte UTF-8 code. Returns UTF_INVALID for invalid encoding. ***/
static u32 utf8_4byte(uchar* w) {
	u32 ret;
	if (!is_continuation_byte(*(w + 1)))
		return(UTF_INVALID);
	if (!is_continuation_byte(*(w + 2)))
		return(UTF_INVALID);
	if (!is_continuation_byte(*(w + 3)))
		return(UTF_INVALID);
	ret = (((*w) & LOW_3) << 18) + (((*(w + 1)) & LOW_6) << 12) +
		 (((*(w + 2)) & LOW_6) << 6) + (((*(w + 3)) & LOW_6));
	if (ret < 0x10000)
		return(UTF_INVALID);
	return(ret);
}

/*** Is the C string passed in ASCII? ***/
bool is_ascii(const char* str) {
	const uchar* w = (const uchar*)str;
	until (iszero(*w))
		{
		if ((*w) > 0x7f)
			return(false);
		++w;
		}
	return(true);
}

/*** Is the C string passed in UTF-8? ***/
bool is_utf8(const char* str) {
	// TODO: this should probably only check the first "x" bytes, to allow a quick guess and avoid checking 
	// an entire million-character string
	return (utf8_strlen((const utf8string)str) != UTF_INVALID);
}

/*** Return the length (in codepoints) of a zero-terminated UTF-8 string.
		Return UTF_INVALID if the encoding is invalid. ***/
uint utf8_strlen(const utf8string str) {
	uint l = 0UL;
	uint ex;
	uint e;
	const uchar* w = (const uchar*)str;
	until (iszero(*w))
		{
		ex = expected_continuation_bytes(*w);
		if (ex == UTF_INVALID)
			return(UTF_INVALID);
		for (e = 0; e < ex; ++e)
			{
			++w;
			if (!is_continuation_byte(*w))
				return(UTF_INVALID);	// bogosity	
			}
		++w;
		++l;
		}
	return(l);
}

/*** Convert a zero-terminated UTF-8 encoded string into an allocated UTF-32 string (ustring).
		Returns NULL on failure (OOM or an invalid UTF-8 encoding.) ***/
ustring utf8_to_utf32(const utf8string str) {
	/* We accept a couple things that the standard doesn't, like UTF-16 surrogate pairs
		encoded as UTF-8, because some popular platforms (Windows, cough) write
		such nonsense out */
	uint len = utf8_strlen(str);
	if (UTF_INVALID == len)
		return(NULL);
	ustring ret = NEW_USTR(len);
	uchar* w = (uchar*)str;
	uch* c = ret;
	NOT_NULL_OR_RETURN(ret, NULL);
	while (*w)
		{
		uint ex;
		ex = expected_continuation_bytes(*w);
		switch (ex)
			{
		case 0:
			*c = (uch)*w;
			++w;
			break;
		case 1:
			*c = utf8_2byte(w);
			if (*c == UTF_INVALID)
				goto LErr;
			w += 2;
			break;
		case 2:
			*c = utf8_3byte(w);
			if (*c == UTF_INVALID)
				goto LErr;
			w += 3;
			break;
		case 3:
			*c = utf8_4byte(w);
			if (*c == UTF_INVALID)
				goto LErr;
			w += 4;
			break;
		default:
LErr:			delete [] ret;
			return(NULL);
			}
		++c;
		}
	*c = 0UL;
	return(ret);
}

/*** Encode a UTF-32 string (ustring) as UTF-8. Returns the allocated encoding. Returns
		NULL on failure (OOM, codepoint out of bounds). ***/
utf8string utf32_to_utf8(const ustring str) {
	utf8string ret;
	// first, calculate the length needed
	uint l = 0UL;
	const uch* w = str;
	char *x; 
	while (*w) {
		if ((*w) > 0xFFFF)
			++l;
		if ((*w) > 0x7FF)
			++l;
		if ((*w) > 0x7F)
			++l;
		++l;
		++w;
	}
	ret = new u8 [l + 1];
	NOT_NULL_OR_RETURN(ret, NULL);

	// now encode
	w = str;
	x = (char*)ret;
	while (*w) {
		if ((*w) < 0x80)
			{
			// easy case
			*x = *w;
			++x;
			++w;
			continue;
			}
		if ((*w) < 0x800)
			{
			// 2 byte encoding
			*x = HIGH_2 | ((*w) >> 6);
			++x;
			*x = HIGH_1 | ((*w) & LOW_6);
			++x;
			++w;
			continue;
			}
		if ((*w) < 0x10000)
			{
			// 3 byte encoding
			*x = HIGH_3 | ((*w) >> 12);
			++x;
			*x = HIGH_1 | (((*w) >> 6) & LOW_6);
			++x;
			*x = HIGH_1 | ((*w) & LOW_6);
			++x;
			++w;
			continue;
			}
		if ((*w) < 0x200000)
			{
			// 4 byte encoding
			*x = HIGH_4 | ((*w) >> 18);
			++x;
			*x = HIGH_1 | (((*w) >> 12) & LOW_6);
			++x;
			*x = HIGH_1 | (((*w) >> 6) & LOW_6);
			++x;
			*x = HIGH_1 | (((*w)) & LOW_6);
			++x;
			++w;
			continue;
			}

		// bad input
		delete [] ret;
		return(NULL);
	}
	*x = 0;

	return(ret);
}

/*** Is the UTF-16 character a high surrogate? ***/
bool is_surrogate_high(u16ch c) {
	return is_between(c, 0xD800, 0xDBFF);
}

/*** Is the UTF-16 character a low surrogate? ***/
bool is_surrogate_low(u16ch c) {
	return is_between(c, 0xDC00, 0xDFFF);
}

/*** Is the UTF-16 character a surrogate? ***/
bool is_surrogate(u16ch c) {
	return is_surrogate_low(c) || is_surrogate_high(c);
}

/*** Return the length of a UTF-16 string. ***/
uint utf16_strlen(utf16string str) {
	uint l = 0UL;
	while (*str) {
		if (is_surrogate_high(*str)) {
			++str;
			if (!is_surrogate_low(*str))
				++l;
		}
		++l;
		++str;
	}
	return(l);
}

/*** Convert a UTF-16 string to UTF-32, and return the allocated result. ***/
ustring utf16_to_utf32(utf16string str) {
	u16ch *from;
	uch *to;
	ustring ret = NEW_USTR(utf16_strlen(str));
	NOT_NULL_OR_RETURN(ret, NULL);
	to = ret;
	from = str;

	while (*from) {
		uch nc;
		
		if (!is_surrogate_high(*from))
			{
			// easy case
			*to = *from;
			++to;
			++from;
			}
		else
			{
			nc = (*from) - 0xD800;
			++from;
			if (is_surrogate_low(*from))
				{
				nc <<= 10;
				nc += ((*from) - 0xDC00);
				*to = nc;
				++to;
				++from;
				}
			else
				{
				// that was a lone high surrogate --we'll treat this as if it's just the high surrogate character
				*to = nc;
				++to;
				}
			}
	}
	*to = 0UL;

	return(ret);
}

/*** Convert a UTF-32 string to UTF-16, and return the allocated result. ***/
utf16string utf32_to_utf16(ustring str) {
	utf16string ret;
	uch* w = str;
	u16ch* to;
	uint l = 0;
	// first determine how many UTF-16 surrogate pairs we'll need to use
	while (*w) {
		if (*w >= 0x10000)
			++l;
		++l;
		++w;
	}
	++l;
	ret = new u16ch [l];
	NOT_NULL_OR_RETURN(ret, NULL);

	// now perform the encoding.
	w = str;
	to = ret;
	while (*w) {
		if (is_surrogate((u16ch)*w))
			{
			// UTF-32 strings can contain UTF-16 surrogate codepoints, but don't encode
			// them as UTF-16.
			// NOTE: this does mean that an invalid UTF-16 string (containing a lone low surrogate,
			// for example) will not round-trip through utf16_to_utf32()/utf32_to_utf16() without
			// losing the invalid surrogate characters. I'm OK with this. 
			++w;
			}
		else if (*w <= 0xFFFF)
			{
			*to = *w;
			++to;
			++w;
			}
		else
			{
			uch cc;
			cc = ((*w) - 0x10000);
			*to = ((cc >> 10) + 0xD800);
			++to;
			*to = ((cc & 1023) + 0xDC00);
			++to;
			++w;
			}
	}
	*to = 0;

	return(ret);
}

/***
	Table of Unicode lowercase codepoints. Shamelessly lifted (with permission)
	from Leigh Brasington, http://www.leighb.com/tounicupper.htm
	Only handles first plane codepoints.
***/
static unsigned short _u_lowers[] = {
	0x0061, 0x0062, 0x0063, 0x0064, 0x0065, 0x0066, 0x0067, 0x0068, 0x0069, 
	0x006A, 0x006B, 0x006C, 0x006D, 0x006E, 0x006F, 0x0070, 0x0071, 0x0072, 
	0x0073, 0x0074, 0x0075, 0x0076, 0x0077, 0x0078, 0x0079, 0x007A, 0x00E0, 
	0x00E1, 0x00E2, 0x00E3, 0x00E4, 0x00E5, 0x00E6, 0x00E7, 0x00E8, 0x00E9, 
	0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF, 0x00F0, 0x00F1, 0x00F2, 
	0x00F3, 0x00F4, 0x00F5, 0x00F6, 0x00F8, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 
	0x00FD, 0x00FE, 0x00FF, 0x0101, 0x0103, 0x0105, 0x0107, 0x0109, 0x010B, 
	0x010D, 0x010F, 0x0111, 0x0113, 0x0115, 0x0117, 0x0119, 0x011B, 0x011D, 
	0x011F, 0x0121, 0x0123, 0x0125, 0x0127, 0x0129, 0x012B, 0x012D, 0x012F, 
	0x0131, 0x0133, 0x0135, 0x0137, 0x013A, 0x013C, 0x013E, 0x0140, 0x0142, 
	0x0144, 0x0146, 0x0148, 0x014B, 0x014D, 0x014F, 0x0151, 0x0153, 0x0155, 
	0x0157, 0x0159, 0x015B, 0x015D, 0x015F, 0x0161, 0x0163, 0x0165, 0x0167, 
	0x0169, 0x016B, 0x016D, 0x016F, 0x0171, 0x0173, 0x0175, 0x0177, 0x017A, 
	0x017C, 0x017E, 0x0183, 0x0185, 0x0188, 0x018C, 0x0192, 0x0199, 0x01A1, 
	0x01A3, 0x01A5, 0x01A8, 0x01AD, 0x01B0, 0x01B4, 0x01B6, 0x01B9, 0x01BD, 
	0x01C6, 0x01C9, 0x01CC, 0x01CE, 0x01D0, 0x01D2, 0x01D4, 0x01D6, 0x01D8, 
	0x01DA, 0x01DC, 0x01DF, 0x01E1, 0x01E3, 0x01E5, 0x01E7, 0x01E9, 0x01EB, 
	0x01ED, 0x01EF, 0x01F3, 0x01F5, 0x01FB, 0x01FD, 0x01FF, 0x0201, 0x0203, 
	0x0205, 0x0207, 0x0209, 0x020B, 0x020D, 0x020F, 0x0211, 0x0213, 0x0215, 
	0x0217, 0x0253, 0x0254, 0x0257, 0x0258, 0x0259, 0x025B, 0x0260, 0x0263, 
	0x0268, 0x0269, 0x026F, 0x0272, 0x0275, 0x0283, 0x0288, 0x028A, 0x028B, 
	0x0292, 0x03AC, 0x03AD, 0x03AE, 0x03AF, 0x03B1, 0x03B2, 0x03B3, 0x03B4, 
	0x03B5, 0x03B6, 0x03B7, 0x03B8, 0x03B9, 0x03BA, 0x03BB, 0x03BC, 0x03BD, 
	0x03BE, 0x03BF, 0x03C0, 0x03C1, 0x03C3, 0x03C4, 0x03C5, 0x03C6, 0x03C7, 
	0x03C8, 0x03C9, 0x03CA, 0x03CB, 0x03CC, 0x03CD, 0x03CE, 0x03E3, 0x03E5, 
	0x03E7, 0x03E9, 0x03EB, 0x03ED, 0x03EF, 0x0430, 0x0431, 0x0432, 0x0433, 
	0x0434, 0x0435, 0x0436, 0x0437, 0x0438, 0x0439, 0x043A, 0x043B, 0x043C, 
	0x043D, 0x043E, 0x043F, 0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 
	0x0446, 0x0447, 0x0448, 0x0449, 0x044A, 0x044B, 0x044C, 0x044D, 0x044E, 
	0x044F, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457, 0x0458, 
	0x0459, 0x045A, 0x045B, 0x045C, 0x045E, 0x045F, 0x0461, 0x0463, 0x0465, 
	0x0467, 0x0469, 0x046B, 0x046D, 0x046F, 0x0471, 0x0473, 0x0475, 0x0477, 
	0x0479, 0x047B, 0x047D, 0x047F, 0x0481, 0x0491, 0x0493, 0x0495, 0x0497, 
	0x0499, 0x049B, 0x049D, 0x049F, 0x04A1, 0x04A3, 0x04A5, 0x04A7, 0x04A9, 
	0x04AB, 0x04AD, 0x04AF, 0x04B1, 0x04B3, 0x04B5, 0x04B7, 0x04B9, 0x04BB, 
	0x04BD, 0x04BF, 0x04C2, 0x04C4, 0x04C8, 0x04CC, 0x04D1, 0x04D3, 0x04D5, 
	0x04D7, 0x04D9, 0x04DB, 0x04DD, 0x04DF, 0x04E1, 0x04E3, 0x04E5, 0x04E7, 
	0x04E9, 0x04EB, 0x04EF, 0x04F1, 0x04F3, 0x04F5, 0x04F9, 0x0561, 0x0562, 
	0x0563, 0x0564, 0x0565, 0x0566, 0x0567, 0x0568, 0x0569, 0x056A, 0x056B, 
	0x056C, 0x056D, 0x056E, 0x056F, 0x0570, 0x0571, 0x0572, 0x0573, 0x0574, 
	0x0575, 0x0576, 0x0577, 0x0578, 0x0579, 0x057A, 0x057B, 0x057C, 0x057D, 
	0x057E, 0x057F, 0x0580, 0x0581, 0x0582, 0x0583, 0x0584, 0x0585, 0x0586, 
	0x10D0, 0x10D1, 0x10D2, 0x10D3, 0x10D4, 0x10D5, 0x10D6, 0x10D7, 0x10D8, 
	0x10D9, 0x10DA, 0x10DB, 0x10DC, 0x10DD, 0x10DE, 0x10DF, 0x10E0, 0x10E1, 
	0x10E2, 0x10E3, 0x10E4, 0x10E5, 0x10E6, 0x10E7, 0x10E8, 0x10E9, 0x10EA, 
	0x10EB, 0x10EC, 0x10ED, 0x10EE, 0x10EF, 0x10F0, 0x10F1, 0x10F2, 0x10F3, 
	0x10F4, 0x10F5, 0x1E01, 0x1E03, 0x1E05, 0x1E07, 0x1E09, 0x1E0B, 0x1E0D, 
	0x1E0F, 0x1E11, 0x1E13, 0x1E15, 0x1E17, 0x1E19, 0x1E1B, 0x1E1D, 0x1E1F, 
	0x1E21, 0x1E23, 0x1E25, 0x1E27, 0x1E29, 0x1E2B, 0x1E2D, 0x1E2F, 0x1E31, 
	0x1E33, 0x1E35, 0x1E37, 0x1E39, 0x1E3B, 0x1E3D, 0x1E3F, 0x1E41, 0x1E43, 
	0x1E45, 0x1E47, 0x1E49, 0x1E4B, 0x1E4D, 0x1E4F, 0x1E51, 0x1E53, 0x1E55, 
	0x1E57, 0x1E59, 0x1E5B, 0x1E5D, 0x1E5F, 0x1E61, 0x1E63, 0x1E65, 0x1E67, 
	0x1E69, 0x1E6B, 0x1E6D, 0x1E6F, 0x1E71, 0x1E73, 0x1E75, 0x1E77, 0x1E79, 
	0x1E7B, 0x1E7D, 0x1E7F, 0x1E81, 0x1E83, 0x1E85, 0x1E87, 0x1E89, 0x1E8B, 
	0x1E8D, 0x1E8F, 0x1E91, 0x1E93, 0x1E95, 0x1EA1, 0x1EA3, 0x1EA5, 0x1EA7, 
	0x1EA9, 0x1EAB, 0x1EAD, 0x1EAF, 0x1EB1, 0x1EB3, 0x1EB5, 0x1EB7, 0x1EB9, 
	0x1EBB, 0x1EBD, 0x1EBF, 0x1EC1, 0x1EC3, 0x1EC5, 0x1EC7, 0x1EC9, 0x1ECB, 
	0x1ECD, 0x1ECF, 0x1ED1, 0x1ED3, 0x1ED5, 0x1ED7, 0x1ED9, 0x1EDB, 0x1EDD, 
	0x1EDF, 0x1EE1, 0x1EE3, 0x1EE5, 0x1EE7, 0x1EE9, 0x1EEB, 0x1EED, 0x1EEF, 
	0x1EF1, 0x1EF3, 0x1EF5, 0x1EF7, 0x1EF9, 0x1F00, 0x1F01, 0x1F02, 0x1F03, 
	0x1F04, 0x1F05, 0x1F06, 0x1F07, 0x1F10, 0x1F11, 0x1F12, 0x1F13, 0x1F14, 
	0x1F15, 0x1F20, 0x1F21, 0x1F22, 0x1F23, 0x1F24, 0x1F25, 0x1F26, 0x1F27, 
	0x1F30, 0x1F31, 0x1F32, 0x1F33, 0x1F34, 0x1F35, 0x1F36, 0x1F37, 0x1F40, 
	0x1F41, 0x1F42, 0x1F43, 0x1F44, 0x1F45, 0x1F51, 0x1F53, 0x1F55, 0x1F57, 
	0x1F60, 0x1F61, 0x1F62, 0x1F63, 0x1F64, 0x1F65, 0x1F66, 0x1F67, 0x1F80, 
	0x1F81, 0x1F82, 0x1F83, 0x1F84, 0x1F85, 0x1F86, 0x1F87, 0x1F90, 0x1F91, 
	0x1F92, 0x1F93, 0x1F94, 0x1F95, 0x1F96, 0x1F97, 0x1FA0, 0x1FA1, 0x1FA2, 
	0x1FA3, 0x1FA4, 0x1FA5, 0x1FA6, 0x1FA7, 0x1FB0, 0x1FB1, 0x1FD0, 0x1FD1, 
	0x1FE0, 0x1FE1, 0x24D0, 0x24D1, 0x24D2, 0x24D3, 0x24D4, 0x24D5, 0x24D6, 
	0x24D7, 0x24D8, 0x24D9, 0x24DA, 0x24DB, 0x24DC, 0x24DD, 0x24DE, 0x24DF, 
	0x24E0, 0x24E1, 0x24E2, 0x24E3, 0x24E4, 0x24E5, 0x24E6, 0x24E7, 0x24E8, 
	0x24E9, 0xFF41, 0xFF42, 0xFF43, 0xFF44, 0xFF45, 0xFF46, 0xFF47, 0xFF48, 
	0xFF49, 0xFF4A, 0xFF4B, 0xFF4C, 0xFF4D, 0xFF4E, 0xFF4F, 0xFF50, 0xFF51, 
	0xFF52, 0xFF53, 0xFF54, 0xFF55, 0xFF56, 0xFF57, 0xFF58, 0xFF59, 0xFF5A, 
	};

/***
	Table of Unicode uppercase codepoints. Shamelessly lifted (with permission)
	from Leigh Brasington, http://www.leighb.com/tounicupper.htm
	Only handles first plane codepoints.
***/
static unsigned short _u_uppers[] = {
	0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048, 0x0049, 
	0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051, 0x0052, 
	0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A, 0x00C0, 
	0x00C1, 0x00C2, 0x00C3, 0x00C4, 0x00C5, 0x00C6, 0x00C7, 0x00C8, 0x00C9, 
	0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF, 0x00D0, 0x00D1, 0x00D2, 
	0x00D3, 0x00D4, 0x00D5, 0x00D6, 0x00D8, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 
	0x00DD, 0x00DE, 0x0178, 0x0100, 0x0102, 0x0104, 0x0106, 0x0108, 0x010A, 
	0x010C, 0x010E, 0x0110, 0x0112, 0x0114, 0x0116, 0x0118, 0x011A, 0x011C, 
	0x011E, 0x0120, 0x0122, 0x0124, 0x0126, 0x0128, 0x012A, 0x012C, 0x012E, 
	0x0049, 0x0132, 0x0134, 0x0136, 0x0139, 0x013B, 0x013D, 0x013F, 0x0141, 
	0x0143, 0x0145, 0x0147, 0x014A, 0x014C, 0x014E, 0x0150, 0x0152, 0x0154, 
	0x0156, 0x0158, 0x015A, 0x015C, 0x015E, 0x0160, 0x0162, 0x0164, 0x0166, 
	0x0168, 0x016A, 0x016C, 0x016E, 0x0170, 0x0172, 0x0174, 0x0176, 0x0179, 
	0x017B, 0x017D, 0x0182, 0x0184, 0x0187, 0x018B, 0x0191, 0x0198, 0x01A0, 
	0x01A2, 0x01A4, 0x01A7, 0x01AC, 0x01AF, 0x01B3, 0x01B5, 0x01B8, 0x01BC, 
	0x01C4, 0x01C7, 0x01CA, 0x01CD, 0x01CF, 0x01D1, 0x01D3, 0x01D5, 0x01D7, 
	0x01D9, 0x01DB, 0x01DE, 0x01E0, 0x01E2, 0x01E4, 0x01E6, 0x01E8, 0x01EA, 
	0x01EC, 0x01EE, 0x01F1, 0x01F4, 0x01FA, 0x01FC, 0x01FE, 0x0200, 0x0202, 
	0x0204, 0x0206, 0x0208, 0x020A, 0x020C, 0x020E, 0x0210, 0x0212, 0x0214, 
	0x0216, 0x0181, 0x0186, 0x018A, 0x018E, 0x018F, 0x0190, 0x0193, 0x0194, 
	0x0197, 0x0196, 0x019C, 0x019D, 0x019F, 0x01A9, 0x01AE, 0x01B1, 0x01B2, 
	0x01B7, 0x0386, 0x0388, 0x0389, 0x038A, 0x0391, 0x0392, 0x0393, 0x0394, 
	0x0395, 0x0396, 0x0397, 0x0398, 0x0399, 0x039A, 0x039B, 0x039C, 0x039D, 
	0x039E, 0x039F, 0x03A0, 0x03A1, 0x03A3, 0x03A4, 0x03A5, 0x03A6, 0x03A7, 
	0x03A8, 0x03A9, 0x03AA, 0x03AB, 0x038C, 0x038E, 0x038F, 0x03E2, 0x03E4, 
	0x03E6, 0x03E8, 0x03EA, 0x03EC, 0x03EE, 0x0410, 0x0411, 0x0412, 0x0413, 
	0x0414, 0x0415, 0x0416, 0x0417, 0x0418, 0x0419, 0x041A, 0x041B, 0x041C, 
	0x041D, 0x041E, 0x041F, 0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 
	0x0426, 0x0427, 0x0428, 0x0429, 0x042A, 0x042B, 0x042C, 0x042D, 0x042E, 
	0x042F, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407, 0x0408, 
	0x0409, 0x040A, 0x040B, 0x040C, 0x040E, 0x040F, 0x0460, 0x0462, 0x0464, 
	0x0466, 0x0468, 0x046A, 0x046C, 0x046E, 0x0470, 0x0472, 0x0474, 0x0476, 
	0x0478, 0x047A, 0x047C, 0x047E, 0x0480, 0x0490, 0x0492, 0x0494, 0x0496, 
	0x0498, 0x049A, 0x049C, 0x049E, 0x04A0, 0x04A2, 0x04A4, 0x04A6, 0x04A8, 
	0x04AA, 0x04AC, 0x04AE, 0x04B0, 0x04B2, 0x04B4, 0x04B6, 0x04B8, 0x04BA, 
	0x04BC, 0x04BE, 0x04C1, 0x04C3, 0x04C7, 0x04CB, 0x04D0, 0x04D2, 0x04D4, 
	0x04D6, 0x04D8, 0x04DA, 0x04DC, 0x04DE, 0x04E0, 0x04E2, 0x04E4, 0x04E6, 
	0x04E8, 0x04EA, 0x04EE, 0x04F0, 0x04F2, 0x04F4, 0x04F8, 0x0531, 0x0532, 
	0x0533, 0x0534, 0x0535, 0x0536, 0x0537, 0x0538, 0x0539, 0x053A, 0x053B, 
	0x053C, 0x053D, 0x053E, 0x053F, 0x0540, 0x0541, 0x0542, 0x0543, 0x0544, 
	0x0545, 0x0546, 0x0547, 0x0548, 0x0549, 0x054A, 0x054B, 0x054C, 0x054D, 
	0x054E, 0x054F, 0x0550, 0x0551, 0x0552, 0x0553, 0x0554, 0x0555, 0x0556, 
	0x10A0, 0x10A1, 0x10A2, 0x10A3, 0x10A4, 0x10A5, 0x10A6, 0x10A7, 0x10A8, 
	0x10A9, 0x10AA, 0x10AB, 0x10AC, 0x10AD, 0x10AE, 0x10AF, 0x10B0, 0x10B1, 
	0x10B2, 0x10B3, 0x10B4, 0x10B5, 0x10B6, 0x10B7, 0x10B8, 0x10B9, 0x10BA, 
	0x10BB, 0x10BC, 0x10BD, 0x10BE, 0x10BF, 0x10C0, 0x10C1, 0x10C2, 0x10C3, 
	0x10C4, 0x10C5, 0x1E00, 0x1E02, 0x1E04, 0x1E06, 0x1E08, 0x1E0A, 0x1E0C, 
	0x1E0E, 0x1E10, 0x1E12, 0x1E14, 0x1E16, 0x1E18, 0x1E1A, 0x1E1C, 0x1E1E, 
	0x1E20, 0x1E22, 0x1E24, 0x1E26, 0x1E28, 0x1E2A, 0x1E2C, 0x1E2E, 0x1E30, 
	0x1E32, 0x1E34, 0x1E36, 0x1E38, 0x1E3A, 0x1E3C, 0x1E3E, 0x1E40, 0x1E42, 
	0x1E44, 0x1E46, 0x1E48, 0x1E4A, 0x1E4C, 0x1E4E, 0x1E50, 0x1E52, 0x1E54, 
	0x1E56, 0x1E58, 0x1E5A, 0x1E5C, 0x1E5E, 0x1E60, 0x1E62, 0x1E64, 0x1E66, 
	0x1E68, 0x1E6A, 0x1E6C, 0x1E6E, 0x1E70, 0x1E72, 0x1E74, 0x1E76, 0x1E78, 
	0x1E7A, 0x1E7C, 0x1E7E, 0x1E80, 0x1E82, 0x1E84, 0x1E86, 0x1E88, 0x1E8A, 
	0x1E8C, 0x1E8E, 0x1E90, 0x1E92, 0x1E94, 0x1EA0, 0x1EA2, 0x1EA4, 0x1EA6, 
	0x1EA8, 0x1EAA, 0x1EAC, 0x1EAE, 0x1EB0, 0x1EB2, 0x1EB4, 0x1EB6, 0x1EB8, 
	0x1EBA, 0x1EBC, 0x1EBE, 0x1EC0, 0x1EC2, 0x1EC4, 0x1EC6, 0x1EC8, 0x1ECA, 
	0x1ECC, 0x1ECE, 0x1ED0, 0x1ED2, 0x1ED4, 0x1ED6, 0x1ED8, 0x1EDA, 0x1EDC, 
	0x1EDE, 0x1EE0, 0x1EE2, 0x1EE4, 0x1EE6, 0x1EE8, 0x1EEA, 0x1EEC, 0x1EEE, 
	0x1EF0, 0x1EF2, 0x1EF4, 0x1EF6, 0x1EF8, 0x1F08, 0x1F09, 0x1F0A, 0x1F0B, 
	0x1F0C, 0x1F0D, 0x1F0E, 0x1F0F, 0x1F18, 0x1F19, 0x1F1A, 0x1F1B, 0x1F1C, 
	0x1F1D, 0x1F28, 0x1F29, 0x1F2A, 0x1F2B, 0x1F2C, 0x1F2D, 0x1F2E, 0x1F2F, 
	0x1F38, 0x1F39, 0x1F3A, 0x1F3B, 0x1F3C, 0x1F3D, 0x1F3E, 0x1F3F, 0x1F48, 
	0x1F49, 0x1F4A, 0x1F4B, 0x1F4C, 0x1F4D, 0x1F59, 0x1F5B, 0x1F5D, 0x1F5F, 
	0x1F68, 0x1F69, 0x1F6A, 0x1F6B, 0x1F6C, 0x1F6D, 0x1F6E, 0x1F6F, 0x1F88, 
	0x1F89, 0x1F8A, 0x1F8B, 0x1F8C, 0x1F8D, 0x1F8E, 0x1F8F, 0x1F98, 0x1F99, 
	0x1F9A, 0x1F9B, 0x1F9C, 0x1F9D, 0x1F9E, 0x1F9F, 0x1FA8, 0x1FA9, 0x1FAA, 
	0x1FAB, 0x1FAC, 0x1FAD, 0x1FAE, 0x1FAF, 0x1FB8, 0x1FB9, 0x1FD8, 0x1FD9, 
	0x1FE8, 0x1FE9, 0x24B6, 0x24B7, 0x24B8, 0x24B9, 0x24BA, 0x24BB, 0x24BC, 
	0x24BD, 0x24BE, 0x24BF, 0x24C0, 0x24C1, 0x24C2, 0x24C3, 0x24C4, 0x24C5, 
	0x24C6, 0x24C7, 0x24C8, 0x24C9, 0x24CA, 0x24CB, 0x24CC, 0x24CD, 0x24CE, 
	0x24CF, 0xFF21, 0xFF22, 0xFF23, 0xFF24, 0xFF25, 0xFF26, 0xFF27, 0xFF28, 
	0xFF29, 0xFF2A, 0xFF2B, 0xFF2C, 0xFF2D, 0xFF2E, 0xFF2F, 0xFF30, 0xFF31, 
	0xFF32, 0xFF33, 0xFF34, 0xFF35, 0xFF36, 0xFF37, 0xFF38, 0xFF39, 0xFF3A, 
	};

static int __short_cmp(const void* v1, const void* v2)
{
	unsigned short s1 = *((unsigned short*)v1);
	unsigned short s2 = *((unsigned short*)v2);
	return ((int)s1 - (int)s2);
}

/*** helper function that handles cases above 0x10000 ***/
static bool _xtnd_u_isupper(uch ch)
{
#define	CASE(l1, l2, u1, u2)	if (is_between(ch, u1, u2))		return(true)
	// DESERET
	CASE(0x10428, 0x1044F, 0x10400, 0x10427);
	// OLD HUNGARIAN
	CASE(0x10CC0, 0x10CF2, 0x10C80, 0x10CB2);
	// WARANG CITI
	CASE(0x118C0, 0x118DF, 0x118A0, 0x118BF);
	// MATHEMATICAL BOLD
	CASE(0x1D41A, 0x1D433, 0x1D400, 0x1D419);
	// MATHEMATICAL ITALIC
	CASE(0x1D44E, 0x1D467, 0x1D434, 0x1D44D);
	// MATHEMATICAL BOLD ITALIC
	CASE(0x1D482, 0x1D49B, 0x1D468, 0x1D481);
	// MATHEMATICAL SCRIPT
	CASE(0x1D4B6, 0x1D4CF, 0x1D49C, 0x1D4B5);
	// MATHEMATICAL BOLD SCRIPT
	CASE(0x1D4EA, 0x1D503, 0x1D4D0, 0x1D4E9);
	// MATHEMATICAL FRAKTUR
	CASE(0x1D51E, 0x1D537, 0x1D504, 0x1D51C);
	// MATHEMATICAL DOUBLE-STRUCK
	CASE(0x1D552, 0x1D56B, 0x1D538, 0x1D550);
	// MATHEMATICAL BOLD FRAKTUR
	CASE(0x1D586, 0x1D59F, 0x1D56C, 0x1D585);
	// MATHEMATICAL SANS-SERIF
	CASE(0x1D5Ba, 0x1D5D3, 0x1D5A0, 0x1D5B9);
	// MATHEMATICAL SANS-SERIF BOLD
	CASE(0x1D5EE, 0x1D607, 0x1D5D4, 0x1D5ED);
	// MATHEMATICAL SANS-SERIF ITALIC
	CASE(0x1D622, 0x1D63B, 0x1D608, 0x1D621);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC
	CASE(0x1D656, 0x1D66F, 0x1D63C, 0x1D655);
	// MATHEMATICAL MONOSPACE
	CASE(0x1D68A, 0x1D6A3, 0x1D670, 0x1D689);
	// MATHEMATICAL BOLD SMALL GREEK LETTERS
	CASE(0x1D6C2, 0x1D6DA, 0x1D6A8, 0x1D6C0);
	// MATHEMATICAL ITALIC GREEK LETTERS
	CASE(0x1D6FC, 0x1D714, 0x1D6E2, 0x1D6FA);
	// MATHEMATICAL BOLD ITALIC GREEK LETTERS
	CASE(0x1D736, 0x1D74E, 0x1D71C, 0x1D734);
	// MATHEMATICAL SANS-SERIF BOLD GREEK LETTERS
	CASE(0x1D770, 0x1D788, 0x1D756, 0x1D76E);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC GREEK LETTERS
	CASE(0x1D7AA, 0x1D7C2, 0x1D790, 0x1D7A8);
#undef	CASE
	return(false);
}

/*** Return whether the Unicode codepoint is an uppercase letter ***/
bool u_isupper(uch ch) {
	unsigned short *s;
	unsigned short sch;
	if (ch >= 0x10000)
		return(_xtnd_u_isupper(ch));
	sch = (unsigned short)ch;
	s = (unsigned short *)bsearch(&sch, _u_uppers, array_size(_u_uppers), sizeof(_u_uppers[0]), __short_cmp);
	return(not_null(s));
}

/*** helper function that handles cases above 0x10000 ***/
static bool _xtnd_u_islower(uch ch) {
#define	CASE(l1, l2, u1, u2)	if (is_between(ch, l1, l2))		return(true)
	// DESERET
	CASE(0x10428, 0x1044F, 0x10400, 0x10427);
	// OLD HUNGARIAN
	CASE(0x10CC0, 0x10CF2, 0x10C80, 0x10CB2);
	// WARANG CITI
	CASE(0x118C0, 0x118DF, 0x118A0, 0x118BF);
	// MATHEMATICAL BOLD
	CASE(0x1D41A, 0x1D433, 0x1D400, 0x1D419);
	// MATHEMATICAL ITALIC
	CASE(0x1D44E, 0x1D467, 0x1D434, 0x1D44D);
	// MATHEMATICAL BOLD ITALIC
	CASE(0x1D482, 0x1D49B, 0x1D468, 0x1D481);
	// MATHEMATICAL SCRIPT
	CASE(0x1D4B6, 0x1D4CF, 0x1D49C, 0x1D4B5);
	// MATHEMATICAL BOLD SCRIPT
	CASE(0x1D4EA, 0x1D503, 0x1D4D0, 0x1D4E9);
	// MATHEMATICAL FRAKTUR
	CASE(0x1D51E, 0x1D537, 0x1D504, 0x1D51C);
	// MATHEMATICAL DOUBLE-STRUCK
	CASE(0x1D552, 0x1D56B, 0x1D538, 0x1D550);
	// MATHEMATICAL BOLD FRAKTUR
	CASE(0x1D586, 0x1D59F, 0x1D56C, 0x1D585);
	// MATHEMATICAL SANS-SERIF
	CASE(0x1D5Ba, 0x1D5D3, 0x1D5A0, 0x1D5B9);
	// MATHEMATICAL SANS-SERIF BOLD
	CASE(0x1D5EE, 0x1D607, 0x1D5D4, 0x1D5ED);
	// MATHEMATICAL SANS-SERIF ITALIC
	CASE(0x1D622, 0x1D63B, 0x1D608, 0x1D621);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC
	CASE(0x1D656, 0x1D66F, 0x1D63C, 0x1D655);
	// MATHEMATICAL MONOSPACE
	CASE(0x1D68A, 0x1D6A3, 0x1D670, 0x1D689);
	// MATHEMATICAL BOLD SMALL GREEK LETTERS
	CASE(0x1D6C2, 0x1D6DA, 0x1D6A8, 0x1D6C0);
	// MATHEMATICAL ITALIC GREEK LETTERS
	CASE(0x1D6FC, 0x1D714, 0x1D6E2, 0x1D6FA);
	// MATHEMATICAL BOLD ITALIC GREEK LETTERS
	CASE(0x1D736, 0x1D74E, 0x1D71C, 0x1D734);
	// MATHEMATICAL SANS-SERIF BOLD GREEK LETTERS
	CASE(0x1D770, 0x1D788, 0x1D756, 0x1D76E);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC GREEK LETTERS
	CASE(0x1D7AA, 0x1D7C2, 0x1D790, 0x1D7A8);
#undef	CASE
	return(false);
}

/*** Return whether the Unicode codepoint is an lowercase letter ***/
bool u_islower(uch ch) {
	unsigned short *s;
	unsigned short sch;
	if (ch >= 0x10000)
		return(_xtnd_u_islower(ch));
	sch = (unsigned short)ch;
	s = (unsigned short *)bsearch(&sch, _u_lowers, array_size(_u_lowers), sizeof(_u_lowers[0]), __short_cmp);
	return(not_null(s));
}

/*** Returns true iff the Unicode codepoint represents a digit ***/
bool u_isdigit(uch ch) {
#define	DRANGE(x)		if (is_between(ch, (x), (x) + 9))	return(true)
	// DIGIT ZERO to NINE
	DRANGE(0x0030);
	if (ch < 0x0660)
		return(false);
	// ARABIC-INDIC DIGIT ZERO to NINE
	DRANGE(0x0660);
	// EXTENDED ARABIC-INDIC
	DRANGE(0x06F0);
	// NKO
	DRANGE(0x07C0);
	// DEVANAGARI
	DRANGE(0x0966);
	// BENGALI
	DRANGE(0x09E7);
	// GURMUKHI
	DRANGE(0x0A66);
	// GUJARATI
	DRANGE(0xAE6);
	// ORIYA
	DRANGE(0xB66);
	// TAMIL
	DRANGE(0xBE6);
	// TELUGU
	DRANGE(0x0C66);
	// KANNADA
	DRANGE(0x0CE6);
	// MALAYALAM
	DRANGE(0x0D66);
	// SINHALA LITH
	DRANGE(0x0DE6);
	// THAI
	DRANGE(0x0E50);
	// LAO
	DRANGE(0x0ED0);
	// TIBETAN
	DRANGE(0x0F20);
	// MYANMAR
	DRANGE(0x1040);
	// KHMER
	DRANGE(0x17E0);
	// MONGOLIAN
	DRANGE(0x1810);
	// LIMBU
	DRANGE(0x1946);
	// NEW TAI LUE
	DRANGE(0x19D0);
	// TAI THAM HORA
	DRANGE(0x1A80);
	// BALINESE
	DRANGE(0x1B50);
	// SUNDANESE
	DRANGE(0x1BB0);
	// LEPCHA
	DRANGE(0x1C40);
	// OL CHIKI
	DRANGE(0x1C50);
	// VAI
	DRANGE(0xA620);
	// SAURASHTRA
	DRANGE(0xA8D0);
	// KAYAH LI
	DRANGE(0xA900);
	// JAVANESE
	DRANGE(0xA9D0);
	// MYANMAR TAI LAING
	DRANGE(0xA9F0);
	// CHAM
	DRANGE(0xAA50);
	// MEETEI MAYEK
	DRANGE(0xABF0);
	// FULLWIDTH DIGIT ZERO to NINE
	DRANGE(0xFF10);
	// OSMANYA
	DRANGE(0x104A0);
	// BRAHMI
	DRANGE(0x11066);
	// SORA SOMPENG
	DRANGE(0x110F0);
	// CHAKMA
	DRANGE(0x11136);
	// SHARADA
	DRANGE(0x111D0);
	// KHUDAWADI
	DRANGE(0x112F0);
	// TIRHUTA
	DRANGE(0x114D0);
	// MODI
	DRANGE(0x11650);
	// TAKRI
	DRANGE(0x116C0);
	// AHOM
	DRANGE(0x11730);
	// WARANG CITI
	DRANGE(0x118E0);
	// MRO
	DRANGE(0x16A60);
	// PAHAWH HMONG
	DRANGE(0x16B50);
	// MATHEMATICAL BOLD DIGIT ZERO to NINE
	DRANGE(0x1D7CE);
	// MATHEMATICAL DOUBLE-STRUCK
	DRANGE(0x1D7D8);
	// MATHEMATICAL SANS-SERIF
	DRANGE(0x1D7E2);
	// MATHEMATICAL SANS-SERIF BOLD
	DRANGE(0x1D7EC);
	// MATHEMATICAL MONOSPACE (sounds like a pretty good band name)
	DRANGE(0x1D7F6);
#undef	DRANGE

	return(false);
}

/*** If the Unicode character represents a digit, return its value (0-9). Else return (-1). ***/
int u_digitval(uch ch) {
#define	DRANGE(x)		if (is_between(ch, (x), (x) + 9))	return(ch - (x))
	// DIGIT ZERO to NINE
	DRANGE(0x0030);
	if (ch < 0x0660)
		return(false);
	// ARABIC-INDIC DIGIT ZERO to NINE
	DRANGE(0x0660);
	// EXTENDED ARABIC-INDIC
	DRANGE(0x06F0);
	// NKO
	DRANGE(0x07C0);
	// DEVANAGARI
	DRANGE(0x0966);
	// BENGALI
	DRANGE(0x09E7);
	// GURMUKHI
	DRANGE(0x0A66);
	// GUJARATI
	DRANGE(0xAE6);
	// ORIYA
	DRANGE(0xB66);
	// TAMIL
	DRANGE(0xBE6);
	// TELUGU
	DRANGE(0x0C66);
	// KANNADA
	DRANGE(0x0CE6);
	// MALAYALAM
	DRANGE(0x0D66);
	// SINHALA LITH
	DRANGE(0x0DE6);
	// THAI
	DRANGE(0x0E50);
	// LAO
	DRANGE(0x0ED0);
	// TIBETAN
	DRANGE(0x0F20);
	// MYANMAR
	DRANGE(0x1040);
	// KHMER
	DRANGE(0x17E0);
	// MONGOLIAN
	DRANGE(0x1810);
	// LIMBU
	DRANGE(0x1946);
	// NEW TAI LUE
	DRANGE(0x19D0);
	// TAI THAM HORA
	DRANGE(0x1A80);
	// BALINESE
	DRANGE(0x1B50);
	// SUNDANESE
	DRANGE(0x1BB0);
	// LEPCHA
	DRANGE(0x1C40);
	// OL CHIKI
	DRANGE(0x1C50);
	// VAI
	DRANGE(0xA620);
	// SAURASHTRA
	DRANGE(0xA8D0);
	// KAYAH LI
	DRANGE(0xA900);
	// JAVANESE
	DRANGE(0xA9D0);
	// MYANMAR TAI LAING
	DRANGE(0xA9F0);
	// CHAM
	DRANGE(0xAA50);
	// MEETEI MAYEK
	DRANGE(0xABF0);
	// FULLWIDTH DIGIT ZERO to NINE
	DRANGE(0xFF10);
	// OSMANYA
	DRANGE(0x104A0);
	// BRAHMI
	DRANGE(0x11066);
	// SORA SOMPENG
	DRANGE(0x110F0);
	// CHAKMA
	DRANGE(0x11136);
	// SHARADA
	DRANGE(0x111D0);
	// KHUDAWADI
	DRANGE(0x112F0);
	// TIRHUTA
	DRANGE(0x114D0);
	// MODI
	DRANGE(0x11650);
	// TAKRI
	DRANGE(0x116C0);
	// AHOM
	DRANGE(0x11730);
	// WARANG CITI
	DRANGE(0x118E0);
	// MRO
	DRANGE(0x16A60);
	// PAHAWH HMONG
	DRANGE(0x16B50);
	// MATHEMATICAL BOLD DIGIT ZERO to NINE
	DRANGE(0x1D7CE);
	// MATHEMATICAL DOUBLE-STRUCK
	DRANGE(0x1D7D8);
	// MATHEMATICAL SANS-SERIF
	DRANGE(0x1D7E2);
	// MATHEMATICAL SANS-SERIF BOLD
	DRANGE(0x1D7EC);
	// MATHEMATICAL MONOSPACE
	DRANGE(0x1D7F6);
#undef	DRANGE
	return(-1);
}

/*** Returns true iff the Unicode codepoint represents an alphabetic character ***/
bool u_isalpha(uch ch) {
	// TODO: this is probably not comprehensive
	if (u_islower(ch))
		return(true);
	if (u_isupper(ch))
		return(true);
	return(false);
}

/*** Returns true iff the Unicode codepoint represents a currency symbol ***/
bool u_iscurrency(uch ch) {
	// check the currency symbols block
	if (is_between(ch, 0x20a0, 0x20be))
		return(true);

	// check the currency symbols scattered around Unicode-space
	switch (ch)
		{
	case 0x0024:	// dollar $ign
	case 0x00A2:	// Cent sign
	case 0x00A3:	// Pound sign
	case 0x00A4:	// currency sign
	case 0x00A5:	// yen sign
	case 0x058F:	// Armenian dram
	case 0x060B:	// Afghani sign
	case 0x09f2:	// Bengali rupee mark
	case 0x09f3:	// Bengali rupee sign
	case 0x0af1:	// Gujarati rupee sign
	case 0x0bf9:	// Tamil rupee sign
	case 0x0e3f:	// Thai bhat
	case 0x17db:	// Khmer riel
	case 0xa838:	// North Indic rupee
	case 0xfdfc:	// rial
	case 0xfe69:	// small dollar sign
	case 0xff04:	// fullwidth dollar sign
	case 0xffe0:	// fullwidth cent sign
	case 0xffe1:	// fullwidth pound sign
	case 0xffe5:	// fullwidth yen sign
	case 0xffe6:	// fullwidth won sign
		return(true);
		}
	return(false);
}

/*** helper that handles cases above 0x10000 ***/
static uch _xtnd_u_tolower(uch ch) {
#define	CASE(l1, l2, u1, u2)	if (is_between(ch, u1, u2))		return((ch - u1) + l1)
	// DESERET
	CASE(0x10428, 0x1044F, 0x10400, 0x10427);
	// OLD HUNGARIAN
	CASE(0x10CC0, 0x10CF2, 0x10C80, 0x10CB2);
	// WARANG CITI
	CASE(0x118C0, 0x118DF, 0x118A0, 0x118BF);
	// MATHEMATICAL BOLD
	CASE(0x1D41A, 0x1D433, 0x1D400, 0x1D419);
	// MATHEMATICAL ITALIC
	CASE(0x1D44E, 0x1D467, 0x1D434, 0x1D44D);
	// MATHEMATICAL BOLD ITALIC
	CASE(0x1D482, 0x1D49B, 0x1D468, 0x1D481);
	// MATHEMATICAL SCRIPT
	CASE(0x1D4B6, 0x1D4CF, 0x1D49C, 0x1D4B5);
	// MATHEMATICAL BOLD SCRIPT
	CASE(0x1D4EA, 0x1D503, 0x1D4D0, 0x1D4E9);
	// MATHEMATICAL FRAKTUR
	CASE(0x1D51E, 0x1D537, 0x1D504, 0x1D51C);
	// MATHEMATICAL DOUBLE-STRUCK
	CASE(0x1D552, 0x1D56B, 0x1D538, 0x1D550);
	// MATHEMATICAL BOLD FRAKTUR
	CASE(0x1D586, 0x1D59F, 0x1D56C, 0x1D585);
	// MATHEMATICAL SANS-SERIF
	CASE(0x1D5Ba, 0x1D5D3, 0x1D5A0, 0x1D5B9);
	// MATHEMATICAL SANS-SERIF BOLD
	CASE(0x1D5EE, 0x1D607, 0x1D5D4, 0x1D5ED);
	// MATHEMATICAL SANS-SERIF ITALIC
	CASE(0x1D622, 0x1D63B, 0x1D608, 0x1D621);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC
	CASE(0x1D656, 0x1D66F, 0x1D63C, 0x1D655);
	// MATHEMATICAL MONOSPACE
	CASE(0x1D68A, 0x1D6A3, 0x1D670, 0x1D689);
	// MATHEMATICAL BOLD SMALL GREEK LETTERS
	CASE(0x1D6C2, 0x1D6DA, 0x1D6A8, 0x1D6C0);
	// MATHEMATICAL ITALIC GREEK LETTERS
	CASE(0x1D6FC, 0x1D714, 0x1D6E2, 0x1D6FA);
	// MATHEMATICAL BOLD ITALIC GREEK LETTERS
	CASE(0x1D736, 0x1D74E, 0x1D71C, 0x1D734);
	// MATHEMATICAL SANS-SERIF BOLD GREEK LETTERS
	CASE(0x1D770, 0x1D788, 0x1D756, 0x1D76E);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC GREEK LETTERS
	CASE(0x1D7AA, 0x1D7C2, 0x1D790, 0x1D7A8);
#undef	CASE
	return(ch);
}

/*** Return the lowercase equivalent of a Unicode codepoint ***/
uch u_tolower(uch ch) {
	unsigned short *s;
	unsigned short sch;

	if (ch >= 0x10000)
		return _xtnd_u_tolower(ch);

	sch = (unsigned short)ch;
	s = (unsigned short *)bsearch(&sch, _u_uppers, array_size(_u_uppers), sizeof(_u_uppers[0]), __short_cmp);
	NOT_NULL_OR_RETURN(s, ch);

	return (uch)_u_lowers[(s - _u_uppers)];
}

/*** helper that handles cases above 0x10000 ***/
static uch _xtnd_u_toupper(uch ch) {
#define	CASE(l1, l2, u1, u2)	if (is_between(ch, l1, l2))		return((ch - l1) + u1)
	// DESERET
	CASE(0x10428, 0x1044F, 0x10400, 0x10427);
	// OLD HUNGARIAN
	CASE(0x10CC0, 0x10CF2, 0x10C80, 0x10CB2);
	// WARANG CITI
	CASE(0x118C0, 0x118DF, 0x118A0, 0x118BF);
	// MATHEMATICAL BOLD
	CASE(0x1D41A, 0x1D433, 0x1D400, 0x1D419);
	// MATHEMATICAL ITALIC
	CASE(0x1D44E, 0x1D467, 0x1D434, 0x1D44D);
	// MATHEMATICAL BOLD ITALIC
	CASE(0x1D482, 0x1D49B, 0x1D468, 0x1D481);
	// MATHEMATICAL SCRIPT
	CASE(0x1D4B6, 0x1D4CF, 0x1D49C, 0x1D4B5);
	// MATHEMATICAL BOLD SCRIPT
	CASE(0x1D4EA, 0x1D503, 0x1D4D0, 0x1D4E9);
	// MATHEMATICAL FRAKTUR
	CASE(0x1D51E, 0x1D537, 0x1D504, 0x1D51C);
	// MATHEMATICAL DOUBLE-STRUCK
	CASE(0x1D552, 0x1D56B, 0x1D538, 0x1D550);
	// MATHEMATICAL BOLD FRAKTUR
	CASE(0x1D586, 0x1D59F, 0x1D56C, 0x1D585);
	// MATHEMATICAL SANS-SERIF
	CASE(0x1D5Ba, 0x1D5D3, 0x1D5A0, 0x1D5B9);
	// MATHEMATICAL SANS-SERIF BOLD
	CASE(0x1D5EE, 0x1D607, 0x1D5D4, 0x1D5ED);
	// MATHEMATICAL SANS-SERIF ITALIC
	CASE(0x1D622, 0x1D63B, 0x1D608, 0x1D621);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC
	CASE(0x1D656, 0x1D66F, 0x1D63C, 0x1D655);
	// MATHEMATICAL MONOSPACE
	CASE(0x1D68A, 0x1D6A3, 0x1D670, 0x1D689);
	// MATHEMATICAL BOLD SMALL GREEK LETTERS
	CASE(0x1D6C2, 0x1D6DA, 0x1D6A8, 0x1D6C0);
	// MATHEMATICAL ITALIC GREEK LETTERS
	CASE(0x1D6FC, 0x1D714, 0x1D6E2, 0x1D6FA);
	// MATHEMATICAL BOLD ITALIC GREEK LETTERS
	CASE(0x1D736, 0x1D74E, 0x1D71C, 0x1D734);
	// MATHEMATICAL SANS-SERIF BOLD GREEK LETTERS
	CASE(0x1D770, 0x1D788, 0x1D756, 0x1D76E);
	// MATHEMATICAL SANS-SERIF BOLD ITALIC GREEK LETTERS
	CASE(0x1D7AA, 0x1D7C2, 0x1D790, 0x1D7A8);
#undef	CASE
	return(ch);
}

/*** Return the uppercase equivalent of a Unicode codepoint ***/
uch u_toupper(uch ch) {
	unsigned short *s;
	unsigned short sch;

	if (ch >= 0x10000)
		return _xtnd_u_toupper(ch);

	sch = (unsigned short)ch;
	s = (unsigned short *)bsearch(&sch, _u_lowers, array_size(_u_lowers), sizeof(_u_lowers[0]), __short_cmp);
	NOT_NULL_OR_RETURN(s, ch);

	return (uch)_u_uppers[(s - _u_lowers)];
}

/*** Converts a Unicode string to lowercase ***/
void u_strlwr(ustring str) {
	while (*str)
		{
		*str = u_tolower(*str);
		++str;
		}
}


/*** Converts a Unicode string to uppercase ***/
void u_strupr(ustring str) {
	while (*str)
		{
		*str = u_toupper(*str);
		++str;
		}
}

/*** Many Unicode codepoints have glyphs resembling A-Z. These characters can be used to "spoof" Latin alphabet
	strings. This function converts look-alike Unicode characters to ASCII equivalents. ***/
uch u_despoofify(uch c) {
	/*** Depending on font, some of these may be more or less deceiving than others. ***/
	
	switch (c)
		{
		/*** Greek lookalike characters ***/
	case 0x391:
		return 'A';
	case 0x392:
		return 'B';
	case 0x395:
		return 'E';
	case 0x396:
		return 'Z';
	case 0x397:
		return 'H';
	case 0x399:
		return 'I';
	case 0x39A:
		return 'K';
	case 0x39C:
		return 'M';
	case 0x39D:
		return 'N';
	case 0x39F:
		return 'O';
	case 0x3A1:
		return 'P';
	case 0x3A4:
		return 'T';
	case 0x3A5:
		return 'Y';
	case 0x3A7:
		return 'X';
	case 0x3B9:
		return 'i';
	case 0x3BA:
		return 'K';
	case 0x3BD:
		return 'v';
	case 0x3BF:
		return 'o';
	case 0x3C1:
		return 'p';
	case 0x3C4:
		return 'T';
	case 0x3C5:
		return 'u';
	case 0x3F9:
		return 'C';
	case 0x3FA:
		return 'M';
		/*** Cyrillic lookalike characters ***/
	case 0x405:
		return 'S';
	case 0x406:
		return 'I';
	case 0x408:
		return 'J';
	case 0x410:
		return 'A';
	case 0x412:
	case 0x432:
		return 'B';
	case 0x415:
		return 'E';
	case 0x417:
	case 0x437:
	case 0x4E0:
	case 0x4E1:
		return '3';
	case 0x41a:
	case 0x43a:
		return 'K';
	case 0x41C:
	case 0x43C:
		return 'M';
	case 0x41D:
	case 0x43D:
		return 'H';
	case 0x41E:
		return 'O';
	case 0x420:
		return 'P';
	case 0x421:
		return 'C';
	case 0x422:
	case 0x442:
		return 'T';
	case 0x423:
	case 0x443:
		return 'y';
	case 0x425:
		return 'X';
	case 0x430:
		return 'a';
	case 0x435:
		return 'e';
	case 0x43E:
		return 'o';
	case 0x440:
		return 'p';
	case 0x441:
		return 'c';
	case 0x445:
		return 'x';
	case 0x44C:
		return 'b';
	case 0x455:
		return 's';
	case 0x456:
		return 'i';
	case 0x458:
		return 'j';
	case 0x461:
		return 'w';
	case 0x4BA:
	case 0x4BB:
		return 'h';
	case 0x4C0:
	case 0x4CF:
		return 'I';
		/*** Some lookalikes in the Letter-like Symbols block ***/
	case 0x2102:
		return 'C';
	case 0x210D:
		return 'H';
	case 0x210E:
	case 0x210F:
		return 'h';
	case 0x2113:
		return 'l';
	case 0x2115:
		return 'N';
	case 0x2119:
		return 'P';
	case 0x211A:
		return 'Q';
	case 0x2124:
		return 'Z';
	case 0x2128:
		return '3';
	case 0x212A:
		return 'K';
	case 0x212E:
	case 0x212F:
		return 'e';
	case 0x2133:
		return 'M';
		/*** Roman numerals also happen to look a lot like letters... ***/
	case 0x2160:
		return 'I';
	case 0x2164:
		return 'V';
	case 0x2169:
		return 'X';
	case 0x216C:
		return 'L';
	case 0x216D:
		return 'C';
	case 0x216E:
		return 'D';
	case 0x216F:
		return 'M';
	case 0x2170:
		return 'i';
	case 0x2174:
		return 'v';
	case 0x2179:
		return 'x';
	case 0x217C:
		return 'l';
	case 0x217D:
		return 'c';
	case 0x217E:
		return 'd';
	case 0x217F:
		return 'm';
		/*** These glyphs are subtly different but may resemble closely the returned counterparts ***/
	case 0x00D7:
		return 'x';
	case 0x00EC:
	case 0x00ED:
	case 0x00EE:
	case 0x00EF:
	case 0x012B:
	case 0x012D:
	case 0x012F:
	case 0x0131:
		return 'i';
	case 0x0104:
		return 'A';
	case 0x0105:
		return 'a';
	case 0x010F:
		return 'd';
	case 0x0118:
		return 'E';
	case 0x0123:
		return 'g';
	case 0x012E:
	case 0x0130:
		return 'I';
	case 0x0136:
	case 0x0138:
		return 'K';
	case 0x013A:
	case 0x013C:
	case 0x013E:
	case 0x0140:
		return 'l';
	case 0x0149:
		return 'n';
	case 0x0162:
		return 'T';
	case 0x0163:
		return 't';
	case 0x0172:
		return 'U';
	case 0x0173:
		return 'u';
	case 0x0185:
		return 'b';
	case 0x0188:
		return 'c';
	case 0x018A:
		return 'D';
	case 0x0192:
		return 'f';
	case 0x0196:
		return 'I';
	case 0x019A:
	case 0x01C0:
		return 'l';
	case 0x01A5:
	case 0x01bF:
		return 'p';
	case 0x01AE:
	case 0x21a:
		return 'T';
	case 0x01B3:
		return 'Y';
	case 0x01b4:
		return 'y';
	case 0x01b7:
		return '3';
	case 0x1e5:
		return 'g';
	case 0x1ea:
		return 'O';
	case 0x1eb:
		return 'o';
	case 0x20b:
		return 'i';
	case 0x21b:
		return 't';
	case 0x221:
		return 'd';
	case 0x224:
		return 'Z';
	case 0x225:
		return 'z';
	case 0x234:
		return 'l';
	case 0x237:
		return 'j';
	case 0x243:
		return 'B';
	case 0x248:
		return 'J';
	case 0x249:
		return 'j';
	case 0x24C:
		return 'R';
	case 0x24D:
		return 'r';
		}

	/* Check for half-width/full-width forms */
	if (is_between(c, 0xFF10, 0xFF19))
		return '0' + c - 0xFF10;
	if (is_between(c, 0xFF21, 0xFF3A))
		return 'A' + c - 0xFF21;
	if (is_between(c, 0xFF41, 0xFF5A))
		return 'a' + c - 0xFF41;

	/* Basic case: return identity */
	return(c);
}

/*** As above, but despoofifies an entire string. ***/
void u_despoofifystr(ustring str)
{
	while (*str) {
		*str = u_despoofify(*str);
		++str;
	}
}

/*** The Windows codepage 1252 to Unicode mapping ***/
static uch __win1252[32] =
	{
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
	0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
	};

/*** The Windows codepage 1250 to Unicode mapping ***/
static uch __win1250[128] =	
	{
	0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x0088, 0x2030, 0x0160, 0x2039, 0x015A, 0x0164, 0X017D, 0X0179,
	0X0090, 0X2018, 0X2019, 0X201C, 0X201D, 0X2022, 0X2013, 0X2014, 0X0098, 0X2122, 0X0161, 0X203A, 0X015B, 0X0165, 0X017E, 0X017A,
	0X00A0, 0X02C7, 0X02D8, 0X0141, 0X00A4, 0X0104, 0X00A6, 0X00A7, 0X00A8, 0X00A9, 0X015E, 0X00AB, 0X00AC, 0X00AD, 0X00AE, 0X017B,
	0X00B0, 0X00B1, 0X02DB, 0X0142, 0X00B4, 0X00B5, 0X00B6, 0X00B7, 0X00B8, 0X0105, 0X015F, 0X00BB, 0X013D, 0X02DD, 0X013E, 0X017C,
	0X0154, 0X00C1, 0X00C2, 0X0102, 0X00C4, 0X0139, 0X0106, 0X00C7, 0X010C, 0X00C9, 0X0118, 0X00CB, 0X011A, 0X00CD, 0X00CE, 0X010E,
	0X0110, 0X0143, 0X0147, 0X00DE, 0X00D4, 0X0150, 0X00D6, 0X00D7, 0X0158, 0X016E, 0X00DA, 0X0170, 0X00DC, 0X00DD, 0X0162, 0X00DF,
	0X0155, 0X00E1, 0X00E2, 0X0103, 0X00E4, 0X013A, 0X0107, 0X00E7, 0X010D, 0X00E9, 0X0119, 0X00EB, 0X011B, 0X00ED, 0X00EE, 0X010F,
	0X0111, 0X0144, 0X0148, 0X00F3, 0X00F4, 0X0151, 0X00F6, 0X00F7, 0X0159, 0X016F, 0X00FA, 0X0171, 0X00FC, 0X00FD, 0X0163, 0X02D9,
	};

/*** The Windows codepage 1251 to Unicode mapping ***/
static uch __win1251[128] = 
	{
	0X0402, 0X0403, 0X201A, 0X0453, 0X201E, 0X2026, 0X2020, 0X2021, 0X20AC, 0X2030, 0X0409, 0X2039, 0X040A, 0X040C, 0X040B, 0X040F,
	0X0452, 0X2018, 0X2019, 0X201C, 0X201D, 0X2022, 0X2013, 0X2014, 0X0098, 0X2122, 0X0459, 0X203A, 0X045A, 0X045C, 0X045B, 0X045F,
	0X00A0, 0X040E, 0X045E, 0X0408, 0X00A4, 0X0490, 0X00A6, 0X00A7, 0X0401, 0X00A9, 0X0404, 0X00AB, 0X00AC, 0X00AD, 0X00AE, 0X0407,
	0X00B0, 0X00B1, 0X0406, 0X0456, 0X0491, 0X00B5, 0X00B6, 0X00B7, 0X0451, 0X2116, 0X0454, 0X00BB, 0X0458, 0X0405, 0X0455, 0X0457, 
	0X0410, 0X0411, 0X0412, 0X0413, 0X0414, 0X0415, 0X0416, 0X0417, 0X0418, 0X0419, 0X041A, 0X041B, 0X041C, 0X041D, 0X041E, 0X041F,
	0X0420, 0X0421, 0X0422, 0X0423, 0X0424, 0X0425, 0X0426, 0X0427, 0X0428, 0X0429, 0X042A, 0X042B, 0X042C, 0X042D, 0X042E, 0X042F,
	0X0430, 0X0431, 0X0432, 0X0433, 0X0434, 0X0435, 0X0436, 0X0437, 0X0438, 0X0439, 0X043A, 0X043B, 0X043C, 0X043D, 0X043E, 0X043F,
	0X0440, 0X0441, 0X0442, 0X0443, 0X0444, 0X0445, 0X0446, 0X0447, 0X0448, 0X0449, 0X044A, 0X044B, 0X044C, 0X044D, 0X044E, 0X044F,
	};

/*** The Windows codepage 1253 to Unicode mapping ***/
static uch __win1253[128] = 
	{
	0x20AC, 0X0081, 0X201A, 0X0192, 0X201E, 0X2026, 0X2020, 0X2021, 0X0088, 0X2030, 0X008A, 0X2039, 0X008C, 0X008D, 0X008E, 0X008F,
	0X0090, 0X2018, 0X2019, 0X201C, 0X201D, 0X2022, 0X2013, 0X2014, 0X0098, 0X2122, 0X009A, 0X203A, 0X009C, 0X009D, 0X009E, 0X009F,
	0X00A0, 0X0385, 0X0386, 0X00A3, 0X00A4, 0X00A5, 0X00A6, 0X00A7, 0X00A8, 0X00A9, 0X00AA, 0X00AB, 0X00AC, 0X00AD, 0X00AE, 0X2015,
	0X00B0, 0X00B1, 0X00B2, 0X00B3, 0X0384, 0X00B5, 0X00B6, 0X00B7, 0X0388, 0X0389, 0X038A, 0X00BB, 0X038C, 0X00BD, 0X038E, 0X038F,
	0X0390, 0X0391, 0X0392, 0X0393, 0X0394, 0X0395, 0X0396, 0X0397, 0X0398, 0X0399, 0X039A, 0X039B, 0X039C, 0X039D, 0X039E, 0X039F,
	0X03A0, 0X03A1, 0X00D2, 0X03A3, 0X03A4, 0X03A5, 0X03A6, 0X03A7, 0X03A8, 0X03A9, 0X03AA, 0X03AB, 0X03AC, 0X03AD, 0X03AE, 0X0EAF,
	0X03B0, 0X03B1, 0X03B2, 0X03B3, 0X03B4, 0X03B5, 0X03B6, 0X03B7, 0X03B8, 0X03B9, 0X03BA, 0X03BB, 0X03BC, 0X03BD, 0X03BE, 0X03BF, 
	0X03C0, 0X03C1, 0X03C2, 0X03C3, 0X03C4, 0X03C5, 0X03C6, 0X03C7, 0X03C8, 0X03C9, 0X03CA, 0X03CB, 0X03CC, 0X03CD, 0X03CE, 0X00FF,
	};

/*** The MS-DOS codepage 437 to Unicode mapping ***/
static uch __DOS437[256] = 
	{
	0X0000, 0X263A, 0X263B, 0X2665, 0X2666, 0X2663, 0X2660, 0X2022, 0X25D8, 0X25CB, 0X25D9, 0X2642, 0X2640, 0X266A, 0X266B, 0X263C,
	0X25BA, 0X25C4, 0X2195, 0X203C, 0X00B6, 0X00A7, 0X25AC, 0X21A8, 0X2191, 0X2193, 0X2192, 0X2190, 0X221F, 0X2194, 0X25B2, 0X25BC,
	0X0020, 0X0021, 0X0022, 0X0023, 0X0024, 0X0025, 0X0026, 0X0027, 0X0028, 0X0029, 0X002A, 0X002B, 0X002C, 0X002D, 0X002E, 0X002F,
	0X0030, 0X0031, 0X0032, 0X0033, 0X0034, 0X0035, 0X0036, 0X0037, 0X0038, 0X0039, 0X003A, 0X003B, 0X003C, 0X003D, 0X003E, 0X003F,
	0X0040, 0X0041, 0X0042, 0X0043, 0X0044, 0X0045, 0X0046, 0X0047, 0X0048, 0X0049, 0X004A, 0X004B, 0X004C, 0X004D, 0X004E, 0X004F,
	0X0050, 0X0051, 0X0052, 0X0053, 0X0054, 0X0055, 0X0056, 0X0057, 0X0058, 0X0059, 0X005A, 0X005B, 0X005C, 0X005D, 0X005E, 0X005F,
	0X0060, 0X0061, 0X0062, 0X0063, 0X0064, 0X0065, 0X0066, 0X0067, 0X0068, 0X0069, 0X006A, 0X006B, 0X006C, 0X006D, 0X006E, 0X006F,
	0X0070, 0X0071, 0X0072, 0X0073, 0X0074, 0X0075, 0X0076, 0X0077, 0X0078, 0X0079, 0X007A, 0X007B, 0X007C, 0X007D, 0X007E, 0X2302,
	0x00c7, 0X00FC, 0X00E9, 0X00E2, 0X00E4, 0X00E0, 0X00E5, 0X00E7, 0X00EA, 0X00EB, 0X00E8, 0X00EF, 0X00EE, 0X00EC, 0X00C4, 0X00C5,
	0X00C9, 0X00E6, 0X00C6, 0X00F4, 0X00F6, 0X00F2, 0X00FB, 0X00F9, 0X00FF, 0X00D6, 0X00DC, 0X00A2, 0X00A3, 0X00A5, 0X20A7, 0X0192,
	0X00E1, 0X00ED, 0X00F3, 0X00FA, 0X00F1, 0X00D1, 0X00AA, 0X00BA, 0X00BF, 0X2310, 0X00AC, 0X00BD, 0X00BC, 0X00A1, 0X00AB, 0X00BB,
	0X2591, 0X2592, 0X2593, 0X2502, 0X2524, 0X2561, 0X2562, 0X2556, 0X2555, 0X2563, 0X2551, 0X2557, 0X255D, 0X255C, 0X255B, 0X2510,
	0X2514, 0X2534, 0X252C, 0X251C, 0X2500, 0X253C, 0X255E, 0X255F, 0X255A, 0X2554, 0X2569, 0X2566, 0X2560, 0X2550, 0X256C, 0X2567,
	0X2568, 0X2564, 0X2565, 0X2559, 0X2558, 0X2552, 0X2553, 0X256B, 0X256A, 0X2518, 0X250C, 0X2588, 0X2584, 0X258C, 0X2590, 0X2580, 
	0X03B1, 0X00DF, 0X0393, 0X03C0, 0X03A3, 0X03C3, 0X00B5, 0X03C4, 0X03A6, 0X0398, 0X03A9, 0X03B4, 0X221E, 0X03C6, 0X03B5, 0X2229,
	0X2261, 0X00B1, 0X2265, 0X2264, 0X2320, 0X2321, 0X00F7, 0X2248, 0X00B0, 0X2219, 0X00B7, 0X221A, 0X207F, 0X00B2, 0X25A0, 0X00A0,
	};

static int __win1254[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
8364, 129, 8218, 402, 8222, 8230, 8224, 8225, 710, 8240, 352, 8249, 338, 141, 142, 143, 
144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 732, 8482, 353, 8250, 339, 157, 158, 376, 
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 
286, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 304, 350, 223, 
224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 
287, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 305, 351, 255, 
};

static int __win1255[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
8364, 129, 8218, 402, 8222, 8230, 8224, 8225, 710, 8240, 138, 8249, 140, 141, 142, 143, 
144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 732, 8482, 154, 8250, 156, 157, 158, 159, 
160, 161, 162, 163, 8362, 165, 166, 167, 168, 169, 215, 171, 172, 173, 174, 175, 
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 247, 187, 188, 189, 190, 191, 
1456, 1457, 1458, 1459, 1460, 1461, 1462, 1463, 1464, 1465, 202, 1467, 1468, 1469, 1470, 1471, 
1472, 1473, 1474, 1475, 1520, 1521, 1522, 1523, 1524, 217, 218, 219, 220, 221, 222, 223, 
1488, 1489, 1490, 1491, 1492, 1493, 1494, 1495, 1496, 1497, 1498, 1499, 1500, 1501, 1502, 1503, 
1504, 1505, 1506, 1507, 1508, 1509, 1510, 1511, 1512, 1513, 1514, 251, 252, 8206, 8207, 255, 
};

static int __win1256[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
8364, 1662, 8218, 402, 8222, 8230, 8224, 8225, 710, 8240, 1657, 8249, 338, 1670, 1688, 1672, 
1711, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 1705, 8482, 1681, 8250, 339, 8204, 8205, 1722, 
160, 1548, 162, 163, 164, 165, 166, 167, 168, 169, 1726, 171, 172, 173, 174, 175, 
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 1563, 187, 188, 189, 190, 1567, 
1729, 1569, 1570, 1571, 1572, 1573, 1574, 1575, 1576, 1577, 1578, 1579, 1580, 1581, 1582, 1583, 
1584, 1585, 1586, 1587, 1588, 1589, 1590, 215, 1591, 1592, 1593, 1594, 1600, 1601, 1602, 1603, 
224, 1604, 226, 1605, 1606, 1607, 1608, 231, 232, 233, 234, 235, 1609, 1610, 238, 239, 
1611, 1612, 1613, 1614, 244, 1615, 1616, 247, 1617, 249, 1618, 251, 252, 8206, 8207, 1746, 
};

static int __win1257[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
8364, 129, 8218, 131, 8222, 8230, 8224, 8225, 136, 8240, 138, 8249, 140, 168, 711, 184, 
144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 152, 8482, 154, 8250, 156, 175, 731, 159, 
160, 161, 162, 163, 164, 165, 166, 167, 216, 169, 342, 171, 172, 173, 174, 198, 
176, 177, 178, 179, 180, 181, 182, 183, 248, 185, 343, 187, 188, 189, 190, 230, 
260, 302, 256, 262, 196, 197, 280, 274, 268, 201, 377, 278, 290, 310, 298, 315, 
352, 323, 325, 211, 332, 213, 214, 215, 370, 321, 346, 362, 220, 379, 381, 223, 
261, 303, 257, 263, 228, 229, 281, 275, 269, 233, 378, 279, 291, 311, 299, 316, 
353, 324, 326, 243, 333, 245, 246, 247, 371, 322, 347, 363, 252, 380, 382, 729, 
};

static int __win1258[] = {
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 
16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 
32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 
48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 
64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 
80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 
96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 
112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 
8364, 129, 8218, 402, 8222, 8230, 8224, 8225, 710, 8240, 138, 8249, 338, 141, 142, 143, 
144, 8216, 8217, 8220, 8221, 8226, 8211, 8212, 732, 8482, 154, 8250, 339, 157, 158, 376, 
160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 
192, 193, 194, 258, 196, 197, 198, 199, 200, 201, 202, 203, 768, 205, 206, 207, 
272, 209, 777, 211, 212, 416, 214, 215, 216, 217, 218, 219, 220, 431, 771, 223, 
224, 225, 226, 259, 228, 229, 230, 231, 232, 233, 234, 235, 769, 237, 238, 239, 
273, 241, 803, 243, 244, 417, 246, 247, 248, 249, 250, 251, 252, 432, 8363, 255, 
};

/*** Convert a character in the DOS 437 code page into the equivalent Unicode codepoint. ***/
uch ufrom437(int c)
{
	if (c < 0 || c > 0xFF)
		return((uch)c);
	return __DOS437[c];
}

/*** Convert a character in the Windows-1250 code page into the equivalent Unicode codepoint. ***/
uch ufrom1250(int c)
{
	if (c < 128 || c > 0xFF)
		return(c);	/* codepage 1250 corresponds with ASCII and Unicode for the first 128 characters */
	return __win1250[c - 128];
}

/*** Convert a character in the Windows-1251 code page into the equivalent Unicode codepoint. ***/
uch ufrom1251(int c)
{
	if (c < 128 || c > 0xFF)
		return(c);	/* codepage 1251 corresponds with ASCII and Unicode for the first 128 characters */
	return __win1251[c - 128];
}

/*** Convert a character in the Windows-1252 code page into the equivalent Unicode codepoint. ***/
uch ufrom1252(int c)
{
	if (c < 128 || c > 0x9F)
		return(c);	/* codepage 1252 corresponds with ASCII and Unicode for the first 128 characters, and with Unicode from 0xA0 to 0xFF. */
	return __win1252[c - 128];
}

/*** Convert a character in the Windows-1253 code page into the equivalent Unicode codepoint. ***/
uch ufrom1253(int c)
{
	if (c < 128 || c > 0xFF)
		return(c);	/* codepage 1253 corresponds with ASCII and Unicode for the first 128 characters */
	return __win1253[c - 128];
}

/*** Convert a character in the Windows-1254 code page into the equivalent Unicode codepoint. ***/
uch ufrom1254(int c)
{
	if (c < 0 || c > 0xFF)
		return(c);
	return __win1254[c];
}

/*** Convert a character in the Windows-1255 code page into the equivalent Unicode codepoint. ***/
uch ufrom1255(int c)
{
	if (c < 0 || c > 0xFF)
		return(c);
	return __win1255[c];
}

/*** Convert a character in the Windows-1256 code page into the equivalent Unicode codepoint. ***/
uch ufrom1256(int c)
{
	if (c < 0 || c > 0xFF)
		return(c);
	return __win1256[c];
}

/*** Convert a character in the Windows-1257 code page into the equivalent Unicode codepoint. ***/
uch ufrom1257(int c)
{
	if (c < 0 || c > 0xFF)
		return(c);
	return __win1257[c];
}

/*** Convert a character in the Windows-1258 code page into the equivalent Unicode codepoint. ***/
uch ufrom1258(int c)
{
	if (c < 0 || c > 0xFF)
		return(c);
	return __win1258[c];
}

/* Chinese and Japanese Windows codepages. */
#include "shiftjis.cpp"

/*** Convert a NUL-terminated string in a Windows codepage encoding to UTF-32. ***/
ustring wstrtoustr(const char* str, int windows_codepage)
{
	static uch (*translate_function[])(int) =
		{
		ufrom437,
		ufrom1250,
		ufrom1251,
		ufrom1252,
		ufrom1253,
		ufrom1254,
		ufrom1255,
		ufrom1256,
		ufrom1257,
		ufrom1258,
		};
	int l = strlen(str);
	int icp;
	if (932 == windows_codepage)
		{
		// Windows ANSI Japanese (Shift JIS plus NEC extended character support)
		return w932toustr(str);
		}
	if (936 == windows_codepage)
		{
		// Simplified Chinese
		return w936toustr(str);
		}
	if (!is_between_int(windows_codepage, 1250, 1258) && (windows_codepage != 437))
		return(NULL);
	ustring ustr = NEW_USTR(l);
	uch* w;
	NOT_NULL_OR_RETURN(ustr, NULL);
	if (windows_codepage == 437)
		icp = 0;
	else
		icp = windows_codepage - 1249;
	w = ustr;
	while (*str)
		{
		*w = translate_function[icp](*(unsigned char*)str);
		++str;
		++w;
		}
	*w = 0;
	return(ustr);
}

// TODO: Consider other codepage translations.
// see http://www.unicode.org/Public/MAPPINGS/
