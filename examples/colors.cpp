/***

	colors.cpp

	Test console colors.

	C. M. Street, 2022

***/

#include "libcodehappy.h"

int main(void) {
	cc_fprintf((cc_color_t)CC_FG_BLUE, stdout, "Test ");
	cc_fprintf((cc_color_t)(CC_FG_BLACK | CC_BG_WHITE), stdout, "Test ");
	cc_fprintf((cc_color_t)CC_FG_DARK_RED, stdout, "Test ");
	cc_fprintf((cc_color_t)CC_FG_WHITE, stdout, "Test ");
	cc_fprintf((cc_color_t)(CC_FG_YELLOW | CC_BG_DARK_BLUE), stdout, "Test\n");
	return 0;
}