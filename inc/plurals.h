/***

	plurals.h

	Plurals of English nouns.

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef PLURALS_H
#define PLURALS_H

/*** Returns the plural of the passed English word. Not threadsafe. ***/
extern const char* english_plural(const char* singular_word);

/*** Given the singular form and a count, gives the correct form. ***/
extern const char* english_singular_or_plural(const char* singular_form, int count);

/*** Pluralizes an Engllsh phrase. Not threadsafe. ***/
extern const char* plural_phrase(const char* singular_phrase);

/*** Attempts to determine the singular form from the plural. Not threadsafe. ***/
extern const char* english_singular(const char* plural_word);

/*** Determine whether or not the passed word is an English plural. ***/
extern int is_english_plural(const char* word);

/*** Verify the order of the internal list of irregular English plurals. ***/
extern void debug_verify_plural_sort(void);

/*** Match the capitalization of retbuf to the capitalization of match_word, character by character.
	If we run out of characters in match_word, we continue with the last case. ***/
extern void match_capitalization(char* retbuf, const char* match_word);

/*** Is the character any of [AEIOUaeiou]? ***/
extern bool is_english_vowel(const char cc);

#endif  // PLURALS_H
