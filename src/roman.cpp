/***

	roman.cpp

	Translate to or from Roman numerals

	In more modern use, the subtractive Roman numerals have been generalized;
	for example, the year 1990 was sometimes written in Roman numerals as MXM.
	The roman_to_integer function understands these modern usages, but
	integer_to_roman does not produce them.

	In the centuries between Roman times and the wide-spread adoption of
	Indic-Arabic numerals, several new 'Roman numerals' were created and
	in various use; these are sometimes called today "medieval Roman numerals"
	as they primarily appear in medieval manuscripts. Not all of these were
	in use at the same time or in the same regions, but these functions
	recognize the following medieval forms:

		A = 5	G = 400	O = 11	S = 7
		B = 300	H = 200	P = 400	T = 160
		E = 250	K = 250	Q = 90	Y = 150
		F = 40	N = 90	R = 80	Z = 2000

	Note that S, as an abbreviation for Latin words beginning with "sept*",
	was sometimes 70 (often, in references to the Septuagint.) A or Q could
	alternately mean 500. There is apparently some historical use of Z as 7.

	integer_to_roman can also produce forms without subtractive numerals;
	these are seen, for example, on some clock faces (using IIII for 4 instead
	of IV.) Of course roman_to_integer understands these as well.

	Copyright (c) 2014-2022 Chris Street

***/

#include "libcodehappy.h"

struct RomanToDec {
	char		numeral;
	u32		dec_equiv;
	bool		is_medieval;
};

static const RomanToDec __transrom[] = {
	{ 'Z',	2000UL,	true	},
	{ 'M',	1000UL,	false	},
	{ 'D',	500UL,	false	},
	{ 'P',	400UL,	true	},
	{ 'G',	400UL,	true	},
	{ 'B',	300UL,	true	},
	{ 'E',	250UL,	true	},
	{ 'K',	250UL,	true	},
	{ 'H',	200UL, 	true	},
	{ 'T',	160UL,	true	},
	{ 'Y',	150UL,	true	},
	{ 'C',	100UL,	false	},
	{ 'N',	90UL,	true	},
	{ 'Q',	90UL,	true	},
	{ 'R',	80UL,	true	},
	{ 'L',	50UL, 	false	},
	{ 'F',	40UL,	true	},
	{ 'O',	11UL,	true	},
	{ 'X',	10UL,	false	},
	{ 'S',	7UL,	true	},
	{ 'V',	5UL,	false	},
	{ 'A', 	5UL,	true	},
	{ 'I',	1UL,	false	},
	{'\000', 0UL,	false	},
};

#define	ROMAN_MAX_STRLEN	64

static bool __validate_roman_numerals(const uint32_t digits[], uint32_t nd) {
	uint32_t e, f;

	/* any increasing numeral that's larger than a previous numeral (not
		immediately preceding) is no good -- CXC is okay but not XXC */
	for (e = 2; e < nd; ++e) {
		if (digits[e] <= digits[e - 1])
			continue;
		for (f = 0; f < e - 1; ++f)
			if (digits[f] < digits[e])
				return(false);
	}

	return(true);
}

uint32_t roman_to_integer(const char* roman, bool permit_medieval) {
	uint32_t n[ROMAN_MAX_STRLEN];
	uint32_t nd = 0;
	uint32_t acc = 0;
	uint32_t l = strlen(roman);
	uint32_t e;
	const RomanToDec *rtd;
	const char* r = roman, *re;

	if (l > ROMAN_MAX_STRLEN)
		return (0UL);
	re = roman + l;

	forever {
		if (r >= re)
			break;

		rtd = __transrom;
		until (0UL == rtd->dec_equiv) {
			if (!rtd->is_medieval || permit_medieval)
				if (rtd->numeral == toupper(*r))
					break;
			++rtd;
		}

		if (0UL == rtd->dec_equiv)
			break;

		n[nd++] = rtd->dec_equiv;
		++r;
	}

	/*** validate that the number is in a standard
		form, i.e. decreasing digits except for subtractive
		digits ***/
	if (!__validate_roman_numerals(n, nd))
		return(0UL);

	for (e = 0; e < nd; ++e) {
		if (e + 1 < nd && n[e] < n[e + 1])
			acc -= n[e++];
		acc += n[e];
	}

	return(acc);
}

/* helper functions */
static void __concat_roman(uint32_t* dec, Scratchpad* sp, uint32_t val, char numeral) {
	while ((*dec) >= val) {
		sp->addc(numeral);
		(*dec) -= val;
	}
}

static void __concat_subtractive(uint32_t* dec, Scratchpad* sp, uint32_t val, const char* str) {
	if ((*dec) >= val) {
		sp->strcat(str);
		(*dec) -= val;
	}
}

char* integer_to_roman(uint32_t dec, bool no_subtractive) {
	Scratchpad* sp = NULL;
	char* retbuf = NULL;

	if (0 == dec || 10000UL < dec)
		goto LRet;

	sp = new Scratchpad(16);
	if (NULL == sp)
		goto LRet;

	__concat_roman(&dec, sp, 1000UL, 'M');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 900UL, "CM");
	__concat_roman(&dec, sp, 500UL, 'D');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 400UL, "CD");
	__concat_roman(&dec, sp, 100UL, 'C');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 90UL, "XC");
	__concat_roman(&dec, sp, 50UL, 'L');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 40UL, "XL");
	__concat_roman(&dec, sp, 10UL, 'X');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 9UL, "IX");
	__concat_roman(&dec, sp, 5UL, 'V');
	if (!no_subtractive)
		__concat_subtractive(&dec, sp, 4UL, "IV");
	__concat_roman(&dec, sp, 1UL, 'I');

	/* free the scratchpad structure, but not its buffer */
	retbuf = (char *) sp->relinquish_buffer();
	delete sp;

LRet:
	return(retbuf);
}


/* end roman.cpp */
