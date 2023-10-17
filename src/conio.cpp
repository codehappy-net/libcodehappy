/***

	conio.cpp

	Implementation of some console I/O functions that were supported by
	Turbo C and DJGPP (<conio.h>). Helpful when porting older code.

	Copyright (c) 2014-2022 C. M. Street.

***/
#ifdef PDCURSES_OK

/*** Clear to the end of the current line. ***/
void clreol(void) {
	ansi_puts("\033[K");
}

/*** Clear the screen. ***/
void clrscr(void) {
	ansi_puts(ANSI_CLEAR_SCREEN);
}

/*** Puts a string to the console. Returns 0 on success. ***/
int cputs(const char* str) {
	ansi_puts(str);
	return 0;
}

/*** Delete the current line. ***/
// TODO: this should scroll lines below the current line up.
void delline(void) {
	ansi_puts("\033[2K");
}

/*** Set the cursor position. ***/
void gotoxy(int x, int y) {
	move(y, x);
}

/*** Set the current text attributes. ***/
void textattr(int attr) {
	// TODO: TBI
}

/*** Set the text's background color. ***/
void textbackground(int clr) {
	// TODO: TBI
}

/*** Return the position of the cursor. ***/
int wherex(void) {
	return getcurx(stdscr);
}

int wherey(void) {
	return getcury(stdscr);
}

#endif  // PDCURSES_OK
/*** end conio.cpp ***/