/***

	ansi.h

	Support for printing strings containing ANSI control characters in the terminal, and
	them doing the right thing. Color, attributes, and cursor control are supported.

	Most terminals support this functionality natively, but Windows is a notable exception.

	A few general-purpose PDcurses functions (used by the ANSI library) are included as well.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifndef __ANSI_H
#define	__ANSI_H

#ifdef PDCURSES_OK

/*** #defines giving string constants for various useful ANSI escape codes. ***/
#define	ANSI_ESCAPE_CODE		"\033["
#define	ANSI_CURSOR_OFF			"\033[?25l"
#define	ANSI_CURSOR_ON			"\033[?25h"
#define	ANSI_CLEAR_SCREEN		"\033[2J"
#define	ANSI_CURSOR_UP			"\033[A"
#define	ANSI_CURSOR_DOWN		"\033[B"
#define	ANSI_CURSOR_FWD			"\033[C"
#define	ANSI_CURSOR_BACK		"\033[D"
#define	ANSI_CURSOR_NEXT_LINE	"\033[E"
#define	ANSI_CURSOR_PREV_LINE	"\033[F"
#define	ANSI_SCROLL_UP			"\033[S"
#define	ANSI_NORMAL_TEXT		"\033" "0m"
#define	ANSI_BOLD				"\033" "1m"
#define	ANSI_BOLD_OFF			"\033" "21m"
#define	ANSI_INVERSE_VIDEO		"\033" "7m"
#define	ANSI_NORMAL_VIDEO		"\033" "27m"
#define	ANSI_TEXT_BLACK			"\033" "30m"
#define	ANSI_TEXT_RED			"\033" "31m"
#define	ANSI_TEXT_GREEN			"\033" "32m"
#define	ANSI_TEXT_YELLOW		"\033" "33m"
#define	ANSI_TEXT_BLUE			"\033" "34m"
#define	ANSI_TEXT_MAGENTA		"\033" "35m"
#define	ANSI_TEXT_CYAN			"\033" "36m"
#define	ANSI_TEXT_WHITE			"\033" "37m"
#define	ANSI_TEXT_BRT_BLACK		"\033" "90m"
#define	ANSI_TEXT_BRT_RED		"\033" "91m"
#define	ANSI_TEXT_BRT_GREEN		"\033" "92m"
#define	ANSI_TEXT_BRT_YELLOW	"\033" "93m"
#define	ANSI_TEXT_BRT_BLUE		"\033" "94m"
#define	ANSI_TEXT_BRT_MAGENTA	"\033" "95m"
#define	ANSI_TEXT_BRT_CYAN		"\033" "96m"
#define	ANSI_TEXT_BRT_WHITE		"\033" "97m"
#define	ANSI_BG_BLACK			"\033" "40m"
#define	ANSI_BG_RED				"\033" "41m"
#define	ANSI_BG_GREEN			"\033" "42m"
#define	ANSI_BG_YELLOW			"\033" "43m"
#define	ANSI_BG_BLUE			"\033" "44m"
#define	ANSI_BG_MAGENTA			"\033" "45m"
#define	ANSI_BG_CYAN			"\033" "46m"
#define	ANSI_BG_WHITE			"\033" "47m"
#define	ANSI_BG_BRT_BLACK		"\033" "100m"
#define	ANSI_BG_BRT_RED			"\033" "101m"
#define	ANSI_BG_BRT_GREEN		"\033" "102m"
#define	ANSI_BG_BRT_YELLOW		"\033" "103m"
#define	ANSI_BG_BRT_BLUE		"\033" "104m"
#define	ANSI_BG_BRT_MAGENTA		"\033" "105m"
#define	ANSI_BG_BRT_CYAN		"\033" "106m"
#define	ANSI_BG_BRT_WHITE		"\033" "107m"

/*** Print the string to the terminal, interpreting ANSI codes ***/
extern void ansi_puts(const char* str);

/*** If the PDCurses library is not initialized, do so with reasonable default settings. ***/
extern void ensure_curses_lib(void);

/*** Will call endwin() if PDCurses is active; otherwise it is a no-op. ***/
extern void curses_lib_done(void);

#endif  // PDCURSES_OK

#endif  // __ANSI_H
/* end ansi.h */