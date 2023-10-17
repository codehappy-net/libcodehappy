/***

	sprintf.h

	A version of sprintf() that doesn't suck. Allocates the memory needed for the 
	formatted string and returns it.

	Copyright (c) 2014-2022, C. M. Street

***/
#ifndef ___CODEHAPPY_SPRINTF_H
#define ___CODEHAPPY_SPRINTF_H

extern char *mysprintf(const char *fmt, ...);

#endif   // ___CODEHAPPY_SPRINTF_H
