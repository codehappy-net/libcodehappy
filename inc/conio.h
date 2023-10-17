/***

	conio.h

	Implementation of some console I/O functions that were supported by
	Turbo C and DJGPP (<conio.h>). Helpful when porting older code.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifndef	__CONSOLE_IO__
#define __CONSOLE_IO__

#ifdef PDCURSES_OK

/*** Clear to the end of the current line. ***/
extern void clreol(void);

/*** Clear the screen. ***/
extern void clrscr(void);

/*** Puts a string to the console. Returns 0 on success. ***/
extern int cputs(const char* str);

/*** Delete the current line. ***/
// TODO: this should scroll lines below the current line up.
extern void delline(void);

/*** Set the cursor position. ***/
extern void gotoxy(int x, int y);

/*** Set the current text attributes. ***/
extern void textattr(int attr);

/*** Set the text's background color. ***/
extern void textbackground(int clr);

/*** Return the position of the cursor. ***/
extern int wherex(void);
extern int wherey(void);


/*** The text colors. ***/
#define	TEXTCOLOR_BLACK			0
#define	TEXTCOLOR_DARK_RED		1
#define	TEXTCOLOR_DARK_GREEN	2
#define	TEXTCOLOR_DARK_YELLOW	3
#define	TEXTCOLOR_DARK_BLUE		4
#define	TEXTCOLOR_DARK_MAGENTA	5
#define	TEXTCOLOR_DARK_CYAN		6
#define	TEXTCOLOR_GRAY			7
#define	TEXTCOLOR_DARK_GRAY		8
#define	TEXTCOLOR_RED			9
#define	TEXTCOLOR_GREEN			10
#define	TEXTCOLOR_YELLOW		11
#define	TEXTCOLOR_BLUE			12
#define	TEXTCOLOR_MAGENTA		13
#define	TEXTCOLOR_CYAN			14
#define	TEXTCOLOR_WHITE			15

/*** make a foreground and background text color into an attribute code suitable for input to textattr() ***/
#define	COLOR_ATTR(fg, bg)		((fg) + ((bg) << 4))

/***
	Some functions not supported:

	cgets() - has the disadvantages of gets() plus being non-portable.
***/

#endif  // PDCURSES_OK

#endif  // __CONSOLE_IO__
/* end conio.h */