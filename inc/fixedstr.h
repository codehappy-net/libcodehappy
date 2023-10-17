/***

	fixedstr.h

	Fixed-size strings.

	What good is a fixed-size string, you ask? Well, the layout of the string is always known, and unused
	bytes are filled with zeroes (not garbage.) So, you can use them as Plain Old Data -- put them in a
	structure or in a cuckoo hash table and they can compare without any special functions (just memcmp), or use them
	as an interchange format to disk or in a network packet (paying attention to endianness), for example.
	
	Fixed strings also keep track of their size, allowing constant time length operation and protection
	against overruns. This also means they can include NUL bytes if desired. 

	They may also make your code look retro-cool.

	Example:

		struct oppa_FORTRAN_style {
			FIXEDSTR(20, name);
			FIXEDSTR(16, address);
			FIXEDSTR(2, state_code);
			FIXEDSTR(16, country);
			FIXEDSTR(5, ZIP_code);
			FIXEDSTR(80, string_madness);
		};

	Copyright (c) 2014-2022 Chris Street.

***/
#ifndef FIXEDSTR_H
#define FIXEDSTR_H

// Declare a fixed-size string of a given size. Uses an anonymous struct in an anonymous union and
// designated initializers, so you want C99 or a modern C++ compiler for this.
// Use this macro to actually declare the strings. Do be careful if using #pragma pack or something of the sort.
// Note that these strings are not zero terminated, so don't go digging around for the buffer; please use
// fixedstr2cstr() instead.
#define	FIXEDSTR(size, name)			\
		union {							\
			struct {					\
				u16 sz;					\
				u16 len;				\
				unsigned char c[size];	\
				} z;					\
			u16 init[3];				\
			} name = { .init = { size, 0, 0 } }

//
// Yes, we do this for Unicode too! Everything is Unicode and the world is Unicode. 
// However, if we encode our Unicode strings as UTF-32, and allow NUL bytes in our little
// eight-bit fixed strings, then there is no reason we can't create a fixed Unicode string
// from a fixed string large enough to accomodate it. The implementation is trivial.
// So in reality, the world is just a regular fixed string buffer, only four times larger.
//
// (When you know how to turn objects into other objects, this is called "object oriented programming."
// And co-opting the u16 sz/len fields to mean length in codepoints instead of chars is 
// polymorphism, an important sub-discpline of the field.)
//
#define	FIXEDUSTR(size, name)	FIXEDSTR((size) * 4, name)

/* A non-anonymous struct to use as a rhyming type -- you could cast a pointer to an 
	anonymous struct declared as above to this. It is not recommended. */
struct __fixedstr {
	u32 sz;
	unsigned char c[0];
};

// The type "fixedstr" is just a handle. Users of this module don't really need to care about
// implementation anyway, right?
typedef	void *
		fixedstr;
typedef	void *
		fixedustr;

/* Copy fixed string src into fixed string dest. Return value is the number of characters copied. */
extern uint fixedstrcpy(fixedstr dest, fixedstr src);

/* Concatenate fixed strings together (src to the end of dest). Returns the length of the concatenated string dest. */
extern uint fixedstrcat(fixedstr dest, fixedstr src);

/* Copy a C string (src) into a fixedstr (dest). Returns the number of characters actually copied. */
extern uint fixedstrfromcstr(fixedstr dest, const char* src);

/* Get the character at position "index" in the fixedstr. Returns -1 if the index is out of bounds. */
extern int fixedchar(const fixedstr string, const uint index);

/* Get the index of the first character in "string" matching "match", at or beyond index "startpos". Returns -1 if there
	are no (more) occurrences of "match". */
extern int fixedstrchr(const fixedstr string, const unsigned char match, const uint startpos = 0UL);

/* Return the length of a fixed string. */
extern uint fixedstrlen(const fixedstr str);

/* Return the max length of a fixed string. */
extern uint fixedstrmaxlen(const fixedstr str);

/*** You know access to the fixed string's character buffer? The thing I told you up there I didn't want you to have?
	Well, this function gives it to you -- for the case when you want to store strings with NUL characters. ***/
extern unsigned char* fixedstrbuf(const fixedstr str);

/*** And return a pointer to the string length. Yes, this means you can use fixedstrings like Pascal strings, if you want 
	Pascal strings. Here you go. Hugs and kisses. ***/
extern unsigned char* fixedstrpascal(const fixedstr str);

/* Copy a fixed string to a C string. Specify the allocated length of the C string (include the zero terminator.) Returns
	the number of characters copied (including zero terminator.) The output C string is always null-terminated. */
extern uint fixedstr2cstr(const fixedstr s1, char* s2, u32 maxlen);

/*** Unicode versions of the above functions. ***/
extern uint fixedustrcpy(fixedustr dest, fixedustr src);
extern uint fixedustrcat(fixedustr dest, fixedustr src);
extern uint fixedustrfromustr(fixedustr dest, ustring src);
extern int fixeduchar(const fixedustr string, const uint index);
extern int fixedustruchr(const fixedustr string, const uch match, const uint startpos = 0UL);
extern uint fixedustrlen(const fixedustr str);
extern uint fixedustrmaxlen(const fixedustr str);
extern uch* fixedustrbuf(const fixedstr str);
extern uint fixedustr2ustr(const fixedustr s1, ustring s2, u32 maxlen);

#endif  // FIXEDSTR_H
