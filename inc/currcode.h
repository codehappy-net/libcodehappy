/***

	currcode.h

	ISO-4217 three letter currency codes for each nation.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifndef CURRCODE_H
#define CURRCODE_H

/*** verify currency sort ***/
extern void debug_verify_currency_sort(void);

/*** given a country name, return the 3-letter ISO 4217 currency code ***/
extern const char* currency_iso4217_for_country(const char country[]);

#endif   // CURRCODE_H
/* end currcode.h */