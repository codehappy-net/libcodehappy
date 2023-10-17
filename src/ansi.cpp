/***

	ansi.cpp

	Support for printing strings containing ANSI control characters in the terminal, and
	have them do the right thing. Color, attributes, and cursor control are supported.

	Most terminals support this functionality natively, but the terminal shipped with Windows is 
	a notable exception. These functions give support for control codes even in Windows terminals.

	Currently only built into libcodehappy in native Windows environments.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifdef PDCURSES_OK

#define	CHAR_ESC	0033
#define	CHAR_CSI	0233

static bool __is_termcode(const char* w) {
	if (*w == CHAR_ESC)
		return(true);
	if (*w == CHAR_CSI)
		return(true);
	return(false);
}

static bool __curses_init = false;

#define	COLOR_PAIR_INDEX(fg, bg)	(((bg << 3) + (fg)) + 1)

void ensure_curses_lib(void) {
	if (__curses_init)
		return;
	initscr();
	raw();
	start_color();
	attrset(A_NORMAL);
	init_pair(COLOR_PAIR_INDEX(COLOR_WHITE, COLOR_BLACK), COLOR_WHITE, COLOR_BLACK);
	attron(COLOR_PAIR(COLOR_PAIR_INDEX(COLOR_WHITE, COLOR_BLACK)));
	keypad(stdscr, TRUE);
	__curses_init = true;
}

void curses_lib_done(void) {
	if (!__curses_init)
		return;
	endwin();
	__curses_init = false;
}

static bool __is_final_ch(const char w) {
	return is_between(w, '@', '~');
}

static void __clear_line_between(int col1, int col2, int rw) {
	int i;

	for (i = col1; i <= col2; ++i) {
		move(rw, i);
		addch(' ');
	}
}

static void __clear_lines(int rw1, int rw2, int max_x) {
	int i, j;

	for (j = rw1; j <= rw2; ++j) {
		for (i = 0; i <= max_x; ++i) {
			move(j, i);
			addch(' ');
		}
	}
}

static void __clear_to_end(void) {
	int x, y, sx, sy;
	getyx(stdscr, y, x);
	getmaxyx(stdscr, sy, sx);
	__clear_line_between(x, sx - 1, y);
	__clear_lines(y + 1, sy - 1, sx - 1);
	move(y, x);
}

static void __clear_to_start(void) {
	int x, y, sx, sy;
	getyx(stdscr, y, x);
	getmaxyx(stdscr, sy, sx);
	__clear_line_between(0, x, y);
	__clear_lines(0, y - 1, sx - 1);
	move(y, x);
}

static void __cursor_move_delta(int dx, int dy) {
	int x, y, sx, sy;
	
	getyx(stdscr, y, x);
	getmaxyx(stdscr, sy, sx);
	x += dx;
	y += dy;
	x = CLAMP(x, 0, sx - 1);
	y = CLAMP(y, 0, sy - 1);
	move(y, x);
}

#define	MISSING_ARG		0xFFFFFFFFUL
#define	HI_INTENSITY	(8)

static void __set_sgr_parameters(u32* iargs, u32 cargs) {
	int i;
	const u32 colors[16] =
		{COLOR_BLACK, COLOR_RED, COLOR_GREEN, COLOR_YELLOW,
		 COLOR_BLUE, COLOR_MAGENTA, COLOR_CYAN, COLOR_WHITE,
		 COLOR_BLACK + HI_INTENSITY, COLOR_RED + HI_INTENSITY, COLOR_GREEN + HI_INTENSITY, COLOR_YELLOW + HI_INTENSITY,
		 COLOR_BLUE + HI_INTENSITY, COLOR_MAGENTA + HI_INTENSITY, COLOR_CYAN + HI_INTENSITY, COLOR_WHITE + HI_INTENSITY	};
	// we start with COLOR_WHITE on COLOR_BLACK
	static u32 __fg_clr = 7;
	static u32 __bg_clr = 0;

	for (i = 0; i < cargs; ++i) {
		if (MISSING_ARG == iargs[i])
			continue;

		switch (iargs[i]) {
		case 0:
			/* reset / normal */
			attrset(A_NORMAL);
			break;
		case 1:
			/* bold / hi-intensity */
			attroff(A_DIM);
			attron(A_BOLD);
			break;
		case 2:
			/* faint / lo-intensity */
			attroff(A_BOLD);
			attron(A_DIM);
			break;
		case 3:
			/* italic / inverse video */
			attron(A_REVERSE);
			break;
		case 4:
			/* underline: single */
			attron(A_UNDERLINE);
			break;
		case 5:
		case 6:
			/* blink (slow) and blink (rapid): for our purposes, the same code */
			attron(A_BLINK);
			break;
		case 7:
			/* image: negative (for our purposes, inverse video) */
			attron(A_REVERSE);
			break;
		case 8:
			/* conceal (we'll implement as A_INVIS) */
			attron(A_INVIS);
			break;
		case 9:
			/* crossed-out (strikethrough) */
			// Not supported
			break;

		/* case 10-19, set terminal font -- not supported */
		/* case 20, Fraktur -- not supported */
		
		case 21:
			/* bold: off or underline: double (we support bold: off) */
			attroff(A_BOLD);
			break;

		case 22:
			/* normal color or intensity */
			attrset(A_NORMAL);
			break;

		case 23:
			/* italic/Fraktur: off, implemented here as inverse video: off */
			attroff(A_REVERSE);
			break;

		case 24:
			/* underline: none */
			attroff(A_UNDERLINE);
			break;

		case 25:
			/* blink off */
			attroff(A_BLINK);
			break;

		/* case 26: this code is reserved, not supported */
		
		case 27:
			/* image: positive, implemented here as inverse video: off */
			attroff(A_REVERSE);
			break;

		case 28:
			/* reveal */
			attroff(A_INVIS);
			break;

		/* case 29: strike-through off, "not crossed out", not supported */

		case 30:
		case 31:
		case 32:
		case 33:
		case 34:
		case 35:
		case 36:
		case 37:
			/* set text color */
			attroff(A_BOLD);
			attroff(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			__fg_clr = (iargs[i] - 30);
			init_pair(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr]), colors[__fg_clr], colors[__bg_clr]);
			attron(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			break;

		/* case 38: reserved, sometimes used for extended/RGB foreground color, not supported -- see code 48 */
		// TODO: might be neat to have some kind of support for these two codes

		case 39:
			/* default text color -- implementation defined according to standard. Implemented here as a reset code, basically. */
			attrset(A_NORMAL);
			break;

		case 40:
		case 41:
		case 42:
		case 43:
		case 44:
		case 45:
		case 46:
		case 47:
			/* set background color */
			attroff(A_BOLD);
			attroff(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			__bg_clr = (iargs[i] - 40);
			init_pair(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr]), colors[__fg_clr], colors[__bg_clr]);
			attron(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			break;

		/* case 48: reserved, sometimes used for extended/RGB background color, not supported -- see code 38 */

		case 49:
			/* default background color -- implementation defined according to standard. Implemented here as a reset code, basically. */
			attrset(A_NORMAL);
			break;

		case 90:
		case 91:
		case 92:
		case 93:
		case 94:
		case 95:
		case 96:
		case 97:
			/* set text color, hi-intensity -- not standard but commonly provided */
			// Note: this adds HI_INTENSITY to get the bright colors -- this might not be portable, on some
			// terminals this might activate blinking, etc. The #ifdef-ed out code below uses the bold attribute instead
			// to indicate hi-intensity colors, which might be more portable.
#if 0
			attron(A_BOLD);
			attroff(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			__fg_clr = (iargs[i] - 90);
			init_pair(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr]), colors[__fg_clr], colors[__bg_clr]);
			attron(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
#else
			attroff(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			__fg_clr = (iargs[i] - 90) + HI_INTENSITY;
			init_pair(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr]), colors[__fg_clr], colors[__bg_clr]);
			attron(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
#endif
			break;

		case 100:
		case 101:
		case 102:
		case 103:
		case 104:
		case 105:
		case 106:
		case 107:
			/* set background color, hi-intensity -- not standard but commonly provided */
			// note comment above about HI_INTENSITY
			attroff(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			__bg_clr = (iargs[i] - 100) + HI_INTENSITY;
			init_pair(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr]), colors[__fg_clr], colors[__bg_clr]);
			attron(COLOR_PAIR(COLOR_PAIR_INDEX(colors[__fg_clr], colors[__bg_clr])));
			break;

		/*
			The remaining SGR codes are reserved or not supported:
				50 - reserved
				51 - framed
				52 - encircled
				53 - overlined
				54 - not framed or encircled
				55 - not overlined
				56-59 - reserved
				60-65 - ideogram codes
		*/
		}
	}
}

static const char* __exec_csi(const char* str) {
	const char* w, *arg;
	bool has_args;
	u32 iargs[8] = {MISSING_ARG, MISSING_ARG, MISSING_ARG, MISSING_ARG,
			MISSING_ARG, MISSING_ARG, MISSING_ARG, MISSING_ARG};
	u32 x, y;
	u32 sx, sy;
	u32 cargs = 0;
	static u32 save_x = 0, save_y = 0;
	
	w = str;
	forever {
		if (!(*w))
			return w;	// ???
		if (__is_final_ch(*w))
			break;
		++w;
	}

	/* determine integer args, if any */
	has_args = (w != str);
	if (has_args)
		{
		arg = str;
		while (arg < w)
			{
			if (*arg == ';')
				{
				iargs[cargs++] = MISSING_ARG;
				++arg;
				continue;
				}
			else
				{
				u32 ag;
				ag = atoi(arg);
				iargs[cargs++] = ag;
				while (arg < w)
					{
					if (*arg == ';')
						break;
					++arg;
					}
				++arg;
				}
			}
		}
	if (MISSING_ARG == iargs[0] && MISSING_ARG == iargs[1])
		has_args = false;

// convenience macros for the first two args
#define	i1	iargs[0]
#define	i2	iargs[1]

	switch (*w)
		{
	case 'A':
		/* cursor up */
		__cursor_move_delta(0, has_args ? -i1 : -1);
		break;
	case 'B':
		/* cursor down */
		__cursor_move_delta(0, has_args ? i1 : 1);
		break;
	case 'C':
		/* cursor forward */
		__cursor_move_delta(has_args ? i1 : 1, 0);
		break;
	case 'D':
		/* cursor back */
		__cursor_move_delta(has_args ? -i1 : -1, 0);
		break;
	case 'E':
		/* cursor next line */
		getyx(stdscr, y, x);
		if (has_args)
			y += i1;
		else
			++y;
		x = 0;
		move(y, x);
		break;
	case 'F':
		/* cursor previous line */
		getyx(stdscr, y, x);
		if (has_args)
			y -= i1;
		else
			--y;
		x = 0;
		if (y < 0)
			y = 0;
		move(y, x);
		break;
	case 'G':
		/* cursor horizontal absolute */
		// TODO: is this 0-based or 1-based? I'm treating the argument as 1-based.
		getyx(stdscr, y, x);
		if (has_args)
			x = i1 - 1;
		assert(x >= 0);
		move(y, x);
		break;
	case 'H':
		/* cursor position */
		// this IS 1-based
		getyx(stdscr, y, x);
		if (i1 == MISSING_ARG)
			y = 0;
		else
			y = i1 - 1;
		if (MISSING_ARG == i2)
			x = 0;
		else
			x = i2 - 1;
		move(y, x);
		break;
	case 'J':
		/* erase display */
		if (0 == i1 || MISSING_ARG == i1)
			{
			// clear from the cursor to the end of the screen
			__clear_to_end();
			}
		else if (1 == i1)
			{
			// clear from the cursor to the start of the screen
			__clear_to_start();
			}
		else if (i1 == 2)
			{
			// clear the entire screen and move to upper-left corner
			erase();
			move(0, 0);
			}
		break;
	case 'K':
		/* erase in line */
		getyx(stdscr, y, x);
		getmaxyx(stdscr, sy, sx);
		if (0 == i1 || MISSING_ARG == i1)
			// clear from cursor to end of line
			__clear_line_between(x, sx - 1, y);
		else if (1 == i1)
			// clear from cursor to beginning of line
			__clear_line_between(0, x, y);
		else if (2 == i1)
			// clear entire line
			__clear_lines(y, y, sx - 1);
		move(y, x);
		break;
	case 'S':
		/* scroll up */
		if (MISSING_ARG == i1)
			scroll(stdscr);
		else
			scrl(i1);
		break;
	case 'T':
		/* scroll down */
		// TODO: implement this feature. The single argument, if present, is the number of lines to scroll.
		assert(false);
		break;
	case 'f':
		/* horizontal and vertical position */
		move((i1 == MISSING_ARG) ? 0 : i1 - 1, (i2 == MISSING_ARG) ? 0 : i2 - 1);
		break;
	case 'm':
		/* select graphic rendition (SGR) */
		__set_sgr_parameters(iargs, cargs);
		break;
	case 's':
		/* save cursor position */
		getyx(stdscr, save_y, save_x);
		break;
	case 'u':
		/* restore cursor position */
		move(save_y, save_x);
		break;
	case '?':
		/* supported DECTCEM codes: ?25l: hide the cursor, ?25h: shows the cursor */
		++w;
		if (!strncmp(w, "25", 2))
			{
			w += 2;
			switch (*w)
				{
			case 'l':
				/* hide the cursor */
				curs_set(0);
				break;
			case 'h':
				/* show the cursor */
				curs_set(1);
				break;
				}
			}
		break;
		/*** Not implemented:
				5i	AUX port on
				4i	AUX port off
				6(n)	device status report
		***/
		}

#undef	i2
#undef	i1
	return(w + 1);
}

static const char* __exec_termcode(const char* str)
{
	bool is_csi;
	
	if (*str == CHAR_CSI)
		{
		is_csi = true;
		++str;
		}
	else if (*str == CHAR_ESC && *(str + 1) == '[')
		{
		is_csi = true;
		str += 2;
		}
	
	if (is_csi)
		{
		// CSI escape codes
		return __exec_csi(str);
		}
	else
		{
		// TODO: non-CSI escape codes -- none of these are currently supported
		/***
			Some that might be useful to add:
			
				ESC N, ESC O: these indicate the following single character is from an
					alternate character set (like IBM-PC extended characters)
					
				ESC ]: Operating system command. Need to find a reference for what
					these codes indicate.
		***/
		++str;
		}

	return(str);
}

void ansi_puts(const char* str)
{
	ensure_curses_lib();
	
	forever
		{
		if (!*str)
			break;
		if (__is_termcode(str))
			{
			str = __exec_termcode(str);
			}
		else
			{
			addch(*str);
			++str;
			}
		}

	refresh();
}

#endif  // PDCURSES_OK
/* end ansi.cpp */