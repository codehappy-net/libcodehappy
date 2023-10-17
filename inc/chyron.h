/***

	chyron.h

	Implements a chyron, a status bar that can give animated alerts.
	
	Copyright (c) 2022 C. M. Street

***/
#ifndef _CHYRON_H_
#define _CHYRON_H_
#ifdef CODEHAPPY_SDL

#define BG_TRANSPARENT	(~0UL)

/* Chryon display strategies: how the messages appear in the chyron */
enum ChyronStrategy {
	CHYRON_STILL = 0,		/* statuses remain still and on display in the chyron, rotate on timer */
	CHYRON_TICKER = 1,		/* statuses continuously scroll in from right like a ticker tape */
	CHYRON_SCROLL_UP = 2,		/* statuses scroll in from bottom and rotate on a timer */
	CHYRON_FADEIN_FADEOUT = 3,	/* statuses fade in and out on a timer */
};

/* For each of the chyron display strategies, there are three modes: messages appear only once, messages continue to appear
   until they are removed by handle, messages continue to appear for a set time and when the timer expires they are removed. */
enum ChryonMode {
	CHYRON_ONCE = 0,
	CHYRON_REMOVE_MANUAL = 1,
	CHYRON_REMOVE_ON_TIMER = 2,
};

class Chyron {
public:
	Chyron(ChryonStrategy strat = CHYRON_STILL, int align = ALIGN_BOTTOM, RGBColor fg_col = C_WHITE, RGBColor bg_col = BG_TRANSPARENT);
	~Chyron();

private:
	int align;		// SIDE_ flag indicating the location of the chyron (SIDE_TOP, SIDE_BOTTOM)
	int height;		// height of the chyron
	int padding;		// padding of the chyron (number of pixels away from the top/bottom it is drawn)
	u32 sec;		// seconds between rotating messages
	u32 scroll;		// scroll speed, in pixels per update
	RGBColor fg;		// foreground color
	RGBColor bg;		// background color (or BG_TRANSPARENT)
};

#endif  // CODEHAPPY_SDL
#endif  // _CHYRON_H_
/* end chyron.h */
