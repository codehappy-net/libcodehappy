/***
	roman.h

	Declarations for Roman numeral functions

	Copyright (c) 2014-2022 C. M. Street
***/
#ifndef ROMAN_H
#define ROMAN_H

extern uint32_t roman_to_integer(const char* roman, bool permit_medieval);
extern char* integer_to_roman(uint32_t dec, bool no_subtractive);

#endif  // ROMAN_H
// end roman.h