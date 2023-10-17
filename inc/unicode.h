/***

	unicode.h

	Definitions to support Unicode strings in the library.

	Internally, ustrings are UTF-32. Functions are provided to convert
	UTF-8 and UTF-16 to and from UTF-32. Some support functions
	that work directly on UTF-8/16 strings are included as well.

	Includes some Unicode-aware <ctype.h>-type functions. Will work for most
	Unicode characters, but with changing specs, I don't guarantee it will
	work on all.

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef __UNICODE_H__
#define __UNICODE_H__

/*** 32-bit character string (i.e., UTF-32). ***/
typedef		uint32_t *
			ustring;

/*** 32-bit character ***/
typedef		uint32_t
			uch;
// NOTE: not uchar, that's an unsigned (8-bit) character

/*** the following are types for UTF-8 and UTF-16 strings ***/
typedef		unsigned char *
			utf8string;
typedef		uint16_t *
			utf16string;
typedef		uint16_t
			u16ch;

/*** String functions for ustring ***/
extern u32 ustrlen(const ustring ustr);
extern u32 ustrcpy(ustring ustr_dest, ustring ustr_src, u32 max_chars_dest);
extern u32 ustrncpy(ustring ustr_dest, ustring ustr_src, u32 nchars, u32 max_chars_dest);
extern i32 ustrcmp(ustring ustr1, ustring ustr2);
extern i32 ustrncmp(ustring ustr1, ustring ustr2, u32 nchars);
extern i32 ustricmp(ustring ustr1, ustring ustr2);
extern u32 ustrcat(ustring ustr, ustring ustr_cat, u32 max_chars_dest);
extern ustring ustrdup(const ustring ustr);
extern ustring ustrstr(ustring haystack, const ustring needle);
extern ustring ustrchr(ustring haystach, const uch needle);

/*** Convert a C one-byte char string into a Unicode string. ***/
extern ustring cstr2ustr(const char* c_str);

/*** Convert a Unicode UTF-32 string into a 8-bit character C string. Replace any characters outside
	the 8-bit range with the passed in char "c" (or removed if c < 0). If replace is non-NULL, it will return how many characters
	outside the 8-bit range were encountered and replaced. ***/
extern char* ustr2str(const ustring u_str, int c, u32* replace);

/*** Is the C string passed in ASCII? ***/
extern bool is_ascii(const char* str);

/*** Is the C string passed in UTF-8? ***/
extern bool is_utf8(const char* str);

/*** Return the length (in codepoints) of a zero-terminated UTF-8 string.
		Return UTF_INVALID if the encoding is invalid. ***/
extern uint utf8_strlen(const utf8string str);

/*** Convert a zero-terminated UTF-8 encoded string into an allocated UTF-32 string (ustring).
		Returns NULL on failure (OOM or an invalid UTF-8 encoding.) ***/
extern ustring utf8_to_utf32(const utf8string str);

/*** Encode a UTF-32 string (ustring) as UTF-8. Returns the allocated encoding. Returns
		NULL on failure (OOM, codepoint out of bounds). ***/
extern utf8string utf32_to_utf8(const ustring str);

/*** Is the UTF-16 character a high surrogate? ***/
extern bool is_surrogate_high(u16ch c);

/*** Is the UTF-16 character a low surrogate? ***/
extern bool is_surrogate_low(u16ch c);

/*** Is the UTF-16 character a surrogate? ***/
extern bool is_surrogate(u16ch c);

/*** Return the length (in codepoints) of a UTF-16 string. ***/
extern uint utf16_strlen(utf16string str);

/*** Convert a UTF-16 string to UTF-32, and return the allocated result. ***/
extern ustring utf16_to_utf32(utf16string str);

/*** Convert a UTF-32 string to UTF-16, and return the allocated result. ***/
extern utf16string utf32_to_utf16(ustring str);

/*** Support macros ***/
#ifndef	NEW_USTR
#define	NEW_USTR(n)		((ustring)new u32 [n + 1])
#endif
#ifndef NEW_STR
#define NEW_STR(n)		(new u8 [n + 1])
#endif

#define	UTF_INVALID			(0xFFFFFFFFUL)

/*** Some ctype.h type functions ***/

/*** Return whether the Unicode codepoint is an uppercase letter ***/
extern bool u_isupper(uch ch);

/*** Return whether the Unicode codepoint is an lowercase letter ***/
extern bool u_islower(uch ch);

/*** Returns true iff the Unicode codepoint represents a digit ***/
extern bool u_isdigit(uch ch);

/*** If the Unicode character represents a digit, return its value (0-9). Else return (-1). ***/
extern int u_digitval(uch ch);

/*** Returns true iff the Unicode codepoint represents an alphabetic character ***/
extern bool u_isalpha(uch ch);

/*** Returns true iff the Unicode codepoint represents a currency symbol ***/
extern bool u_iscurrency(uch ch);

/*** Return the lowercase equivalent of a Unicode codepoint ***/
extern uch u_tolower(uch ch);

/*** Return the uppercase equivalent of a Unicode codepoint ***/
extern uch u_toupper(uch ch);

/*** Converts a Unicode string to lowercase ***/
extern void u_strlwr(ustring str);

/*** Converts a Unicode string to uppercase ***/
extern void u_strupr(ustring str);

/*** Many Unicode codepoints have glyphs resembling A-Z. These characters can be used to "spoof" Latin alphabet
	strings. This function converts look-alike Unicode characters to ASCII equivalents. ***/
extern uch u_despoofify(uch c);

/*** As above, but despoofifies an entire string. ***/
extern void u_despoofifystr(ustring str);

/*** Convert a character in the Windows-1250 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1250(int c);

/*** Convert a character in the Windows-1251 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1251(int c);

/*** Convert a character in the Windows-1252 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1252(int c);

/*** Convert a character in the Windows-1253 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1253(int c);

/*** Convert a character in the Windows-1254 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1254(int c);

/*** Convert a character in the Windows-1255 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1255(int c);

/*** Convert a character in the Windows-1256 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1256(int c);

/*** Convert a character in the Windows-1257 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1257(int c);

/*** Convert a character in the Windows-1258 code page into the equivalent Unicode codepoint. ***/
extern uch ufrom1258(int c);

/*** Convert a character in the DOS 437 code page into the equivalent Unicode codepoint. ***/
/* This is the original IBM PC code mapping, with box drawing characters etc. in "high ASCII", as well as many
	glyphs for unprintable/control characters. Based on Wang's WISCII. */
extern uch ufrom437(int c);

/*** Translate a string in Windows 932 codepage (ANSI Japanese, similar to Shift JIS with support for NEC extended characters)
	to a Unicode UTF-32 string. Returns the allocated result. ***/
extern ustring w932toustr(const char* str);

/*** Translate a string in Windows 936 codepage (Simplified Chinese) to a Unicode UTF-32 string. Returns the allocated result. ***/
extern ustring w936toustr(const char* str);

/*** Convert a NUL-terminated string in a Windows codepage encoding to UTF-32. ***/
/* Currently supported codepages: 437 (MS-DOS), 932 (ANSI Japanese), 936 (Chinese), 
	1250, 1251, 1252, 1253, 1254, 1255, 1256, 1257, 1258. */
extern ustring wstrtoustr(const char* str, int windows_codepage);

#endif // __UNICODE_H__
