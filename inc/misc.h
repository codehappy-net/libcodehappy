/***
	misc.h

	Miscellaneous function declarations. Several functions in this file
	are provided for compatibility; others are general-purpose
	subroutines that don't fit elsewhere.

	Copyright (c) 2005-2022 Chris Street
***/
#ifndef MISC_H
#define MISC_H

/*** Replace all occurences of "replace" in "buf" with "with" ***/
extern void strreplace(char *buf, const char *replace, const char *with);

/*** case-insensitive versions of strcmp(), strncmp(), and strstr(). ***/
extern int __stricmp(const char* s1, const char* s2);
extern int __strnicmp(const char* s1, const char* s2, unsigned int n);
#define	_strincmp(x, y, z)	_strnicmp((x), (y), (z))
extern char *__stristr(const char* haystack, const char* needle);

/*** implementations of strlwr() and strupr() and strdup() for platforms on which they are missing ***/
extern void __strlwr(char* s1);
extern void __strupr(char* s1);
extern char* __strdup(const char* s);

/*** fast implementation of strlen() that may be better than the stdlib version ***/
extern uint _strlen(const char* str);

/*** like strncmp() and strcmp(), but act as if their input strings were backwards ***/
extern int strncmp_backwards(const char *w1, const char *w2, int n);
extern int strcmp_backwards(const char *w1, const char *w2);

/*** fill the specified memory with zeroes ***/
extern void zeromem(void* ptr, uint32_t size);

/*** Linear regression. ***/
extern void BestFitLinearSolution(double *xpoints, double *ypoints, int npoints, double *alpha, double *beta, double *rsquared);

/*** Pythagorean metric ***/
extern uint32_t distance_squared(int32_t x1, int32_t y1, int32_t x2, int32_t y2);
extern uint64_t distance_squared_64(int64_t x1, int64_t y1, int64_t x2, int64_t y2);
extern uint32_t distance_squared_3d(int32_t x1, int32_t y1, int32_t z1, int32_t x2, int32_t y2, int32_t z2);
extern uint64_t distance_squared_3d_64(int64_t x1, int64_t y1, int64_t z1, int64_t x2, int64_t y2, int64_t z2);

/*** estimate of confidence that a cumulative probability will fall below
	the specified t-score with given degrees of freedom ***/
extern double confidence(double t_score, int dof);

/* number of significant figures in a string representing a number */
extern int CountSigFigs(char *v);

/* hardly infallible, but can catch some cases checking titles won't */
extern bool FNameLooksFemale(const char *name);

/* the value of a hexadecimal digit */
extern int valhexdigit(const char ch);

/*** print a character n times ***/
extern void nputc(char ch, u32 count);

/* Does the C string str end with the C string end? */
/* starts_with() is a macro defined in __useful.h */
extern bool ends_with(const char* str, const char* end);

/*** Returns the position of the first occurence of "str_search" in the string "str_in",
	at or after the (0-indexed) position "pos". Returns (-1) if not found. ***/
extern i32 findinstr(i32 pos, const char* str_search, const char* str_in);

/*** Empty string predicate: returns true iff the string is NULL or zero-length. ***/
extern bool FEmpty(const char *str);

/*** Returns the index of the string that matches the argument string (case-insensitive), or -1 if not found. nstr needs to be
	the number of strings we're matching against. ***/
extern int striindex(const char *match, int nstr, ...);

/*** As above, but case-sensitive. ***/
extern int strindex(const char *match, int nstr, ...);

/*** Remove all instances of string "r" from string "str". ***/
extern void RemoveStr(char *str, char *r);

/*** Is the given character text? ***/
extern int istext(int c);

/*** Perform a perfect shuffle (i.e. every possible permutation is equally likely) of nel elements each of size elsize bytes. ***/
extern void perfect_shuffle(void* data, uint nel, size_t elsize);

/*** Ask user to make a selection in [min_valid, max_valid]. Prompt until the user gives the expected response. ***/
extern int menu_response(int min_valid, int max_valid, const char* prompt);

/*** Verify that the number of command-line arguments (besides program name/path) is at least min_arg. If not,
	print the specified error message and quit. ***/
extern void check_number_args(int argc, int min_arg, const char* errmsg);

/*** Yes or no response from user -- returns true iff yes ***/
extern bool yes_or_no(const char* prompt);

/*** A hashing/fingerprinting function. Based on IEEE POSIX P1003.2. ***/
u64 hash_FNV1a(const void* data, u32 data_size);

/*** Compare two blocks of memory without short-circuiting. Useful if you want to compare
	a password or key (e.g.) but do not want to leak information about the comparison by
	taking a different amount of time depending on the number of leading bytes that match. ***/
extern bool secure_memequal(const void* array1, u32 array1_size, const void* array2, u32 array2_size);
/*** As above; a version of strcmp() that does not use any short-circuiting. ***/
extern bool secure_strcmp(const char* str1, const char* str2);

/*** Returns true iff the string matches the pattern. Patterns have the specified case sensitivity and allow wildcards "?" for any one
	character and "*" for 0 or more characters. ***/
extern bool string_matches_pattern(const char* str, const char* pattern, bool is_case_sensitive);

/*** Produce a beep of the specified frequency (in Hz) and duration (in milliseconds). ***/
extern void speaker_beep(u32 frequency, u32 duration_msec);

/*** Set an ostream to max floating point precision. ***/
extern void max_precision(std::ostream& o);

/* Replace the text "rw" in the string "p" with the text "with". */
extern void p_replace(char* p, const char* rw, const char* with);
extern void p_replace(std::string& p, const std::string& rw, const std::string& with);

/* Find the next of two (or three) characters, starting from "w". */
extern const char* next_of_two(const char* w, char c1, char c2);
extern const char* next_of_three(const char* w, char c1, char c2, char c3);

/* Find the next of two (or three) strings, starting from "w". */
extern const char* next_of_two(const char * w, const char * s1, const char * s2);
extern const char* next_of_three(const char * w, const char * s1, const char * s2, const char * s3);

/*** starts_with() moved to a function, due to a conflict with leejet's Stable Diffusion library. ***/
extern bool starts_with(const char* str, const char* pfx);

#endif  // MISC_H
/* end misc.h */
