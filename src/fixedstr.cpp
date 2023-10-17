/***

	fixedstr.cpp

	Fixed-size strings.

	What good is a fixed-size string, you ask? Well, the layout of the string is always known, and unused
	bytes are filled with zeroes (not garbage.) So, you can use them as Plain Old Data -- put them in a
	structure and they can compare in a cuckoo hash table without any special comparison, or use them
	as an interchange format to disk or in a network packet (paying attention to endianness), for example.
	
	Fixed strings also keep track of their size, allowing constant time length operation and protection
	against overruns.

	Example:

		struct oppa_FORTRAN_style {
			FIXEDSTR(20)		name;
			FIXEDSTR(16)		address;
			FIXEDSTR(2)		state_code;
			FIXEDSTR(16)		country;
			FIXEDSTR(6)		ZIP_code;
			FIXEDSTR(80)		string_madness;
		};

	Copyright (c) 2014-2022 Chris Street.

***/

/* Copy fixed string s2 into fixed string s1. Return value is the number of characters copied. */
uint fixedstrcpy(fixedstr dest, fixedstr src) {
	uint len = fixedstrlen(src), maxlen = fixedstrmaxlen(dest);
	unsigned char* buf = fixedstrbuf(dest);
	for (int e = 0; e < len && e < maxlen; ++e) {
		buf[e] = fixedchar(src, (uint)e);
	}
	((u16 *)dest)[1] = (u16)e;
	return e;
}

/* Concatenate fixed strings together (src to the end of dest). Returns the length of the concatenated string dest. */
uint fixedstrcat(fixedstr dest, fixedstr src) {
	uint len = fixedstrlen(src), lend = fixedstrlen(dest), maxlen = fixedstrmaxlen(dest);
	unsigned char* buf = fixedstrbuf(dest) + lend;
	for (uint e = 0; e < len && e + lend < maxlen; ++e) {
		buf[e] = fixedchar(src, e);
	}
	((u16 *)dest)[1] = (u16)(e + lend);
	return (uint)(e + lend);
}

/* Copy a C string (src) into a fixedstr (dest). Returns the number of characters actually copied. */
uint fixedstrfromcstr(fixedstr dest, const char* src) {
	const unsigned char* w = (const unsigned char*)src;
	unsigned char* buf = fixedstrbuf(dest);
	unsigned char* bufe = buf + fixedstrmaxlen(dest);
	while (*w && buf < bufe) {
		*buf = *w;
		++w;
		++buf;
	}
	((u16 *)dest)[1] = (u16)(buf - fixedstrbuf(dest));
	return (uint)(buf - fixedstrbuf(dest));
}

/* Get the character at position "index" in the fixedstr. Returns -1 if the index is out of bounds. */
int fixedchar(const fixedstr string, const uint index) {
	if (index >= fixedstrlen(string))
		return (-1);
	unsigned char* buf = fixedstrbuf(string) + index;
	return (int)(*buf);
}

/* Get the index of the first character in "string" matching "match", at or beyond index "startpos". Returns -1 if there
	are no (more) occurrences of "match". */
int fixedstrchr(const fixedstr string, const unsigned char match, const uint startpos) {
	unsigned char* buf = fixedstrbuf(string) + startpos;
	unsigned char* bufe = buf + fixedstrlen(string);
	while (buf < bufe) {
		if (*buf == match)
			return (buf - fixedstrbuf(string));
		++buf;
	}
	return -1;
}

/* one of the most eminently inlineable functions in this library -- for sure */
uint fixedstrmaxlen(const fixedstr str) {
	return *((u16 *)str);
}

/* Return the length of a fixed string. */
uint fixedstrlen(const fixedstr str) {
	return ((u16 *)str)[1];
}

/* Another fine inline candidate. */
unsigned char* fixedstrbuf(const fixedstr str) {
	return ((struct __fixedstr *)str)->c;
}

/*** And return a pointer to the string length. Yes, this means you can use fixedstrings like Pascal strings, if you want 
	Pascal strings. Here you go. Hugs and kisses. ***/
unsigned char* fixedstrpascal(const fixedstr str) {
	return (unsigned char*)(((u16 *)str) + 1);
}

uint fixedustrcpy(fixedustr dest, fixedustr src) {
	uint len = fixedustrlen(src), maxlen = fixedustrmaxlen(dest);
	uch* buf = fixedustrbuf(dest);
	for (uint e = 0; e < len && e < maxlen; ++e) {
		buf[e] = fixeduchar(src, e);
	}
	((u16 *)dest)[1] = (u16)e;
	return e;
}

uint fixedustrcat(fixedustr dest, fixedustr src) {
	uint len = fixedustrlen(src), lend = fixedustrlen(dest), maxlen = fixedustrmaxlen(dest);
	uch* buf = fixedustrbuf(dest) + lend;
	for (uint e = 0; e < len && e + lend < maxlen; ++e) {
		buf[e] = fixedchar(src, e);
	}
	((u16 *)dest)[1] = (u16)(e + lend);
	return (uint)(e + lend);
}

uint fixedustrfromustr(fixedustr dest, ustring src) {
	const uch* w = (const uch*)src;
	uch* buf = fixedustrbuf(dest);
	uch* bufe = buf + fixedustrmaxlen(dest);
	while (*w && buf < bufe) {
		*buf = *w;
		++w;
		++buf;
	}
	((u16 *)dest)[1] = (u16)(buf - fixedustrbuf(dest));
	return (uint)(buf - fixedustrbuf(dest));
}

int fixeduchar(const fixedustr string, const uint index) {
	if (index >= fixedustrlen(string))
		return (-1);
	uch* buf = fixedustrbuf(string) + index;
	return (int)(*buf);
}

int fixedustruchr(const fixedustr string, const uch match, const uint startpos) {
	uch* buf = fixedustrbuf(string) + startpos;
	uch* bufe = buf + fixedustrlen(string);
	while (buf < bufe) {
		if (*buf == match)
			return (buf - fixedustrbuf(string));
		++buf;
	}
	return -1;
}

uint fixedustrlen(const fixedustr str) {
	return ((u16 *)str)[1];
}

uint fixedustrmaxlen(const fixedustr str) {
	// The macro FIXEDUSTR() sets the size in bytes, but we want maxlen in uchs (u32s).
	// We maintain the actual ustring length -- ((u16*)str)[1] -- in 32-bit codepoints.
	return *((u16 *)str) / 4;
}

uch* fixedustrbuf(const fixedstr str) {
	return (uch *)(((struct __fixedstr *)str)->c);
}

/* Copy a fixed string to a C string. Specify the allocated length of the C string (include the zero terminator.) Returns
	the number of characters copied (including zero terminator.) The output C string is always null-terminated. */
uint fixedstr2cstr(const fixedstr s1, char* s2, u32 maxlen) {
	const unsigned char* buf = fixedstrbuf(s1);
	const unsigned char* bufe = buf + fixedstrlen(s1);
	char* w = s2, *we = s2 + maxlen;

	while (buf < bufe && w < we) {
		*w = (char)*buf;
		++w;
		++buf;
	}
	// Zero-terminate.
	if (w == we && w != s2) {
		--w;
	}
	*w = '\000';
	return (w - s2) + 1;
}

/* Above, for Unicode strings. */
uint fixedustr2ustr(const fixedustr s1, ustring s2, u32 maxlen) {
	const uch* buf = fixedustrbuf(s1);
	const uch* bufe = buf + fixedustrlen(s1);
	uch* w = (uch *)s2;
	uch* we = w + maxlen;

	while (buf < bufe && w < we) {
		*w = *buf;
		++w;
		++buf;
	}
	// Zero-terminate.
	if (w == we && w != (uch *)s2) {
		--w;
	}
	*w = 0;
	return (w - (uch *)s2) + 1;
}

/*** end fixedstr.cpp ***/