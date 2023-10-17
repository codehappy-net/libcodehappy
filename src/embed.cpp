/***

	embed.cpp

	This file includes the embedded TrueType fonts and instrument patches
	for libcodehappy. These take up a ton of memory and time during compilation,
	when they are seldom added or changed. We already split a couple external
	libraries out of the single-file build, breaking this stuff out is a mercy.

	Copyright (c) 2022 Chris Street

***/
#include <libcodehappy.h>

#include "fonts/fonts.i"
#include "patches/all.i"
