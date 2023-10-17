/***

	kb.h

	Low-level keyboard access.

	Currently this hooks into the SDL's keyboard handler, but if other windowing system
	libraries are supported in the future, code may be written to support them as well.

	C. M. Street

***/
#ifndef __KB_H
#define __KB_H

// Needs to either be WebAssembly or a native SDL app to use the keyboard handling code.
#ifdef CODEHAPPY_SDL
#define SDL_KB_OK
#else
#ifndef CODEHAPPY_NATIVE
#define SDL_KB_OK
#endif
#endif

#ifdef SDL_KB_OK

/*** Callback type for keyboard trapping. ***/
/*** The first argument is the key. The second is the special key flags (a bitfield with KBF_* flags set). The third
	is user-specified arguments provided when the callback is registered. The return value should be 0 if everything
	is okay, and non-zero if you desire the keyboard callback to be de-registered and no longer called. ***/
typedef int (*KbCallback)(int key, int flags, void* args);

#define KBF_LSHIFT		0x0001
#define KBF_RSHIFT		0x0002
#define KBF_LCTRL		0x0040
#define KBF_RCTRL		0x0080
#define KBF_LALT		0x0100
#define KBF_RALT		0x0200
#define KBF_LGUI		0x0400
#define KBF_RGUI		0x0800
#define KBF_NUM			0x1000
#define KBF_CAPS		0x2000
#define KBF_MODE		0x4000
#define KBF_RESERVED	0x8000

/* use these flags to indicate either of the SHIFT/CTRL/ALT/GUI buttons */
#define	KBF_SHIFT		(KBF_LSHIFT | KBF_RSHIFT)
#define	KBF_CTRL		(KBF_LCTRL | KBF_RCTRL)
#define	KBF_ALT			(KBF_LALT | KBF_RALT)
#define	KBF_GUI			(KBF_LGUI | KBF_RGUI)

/* and this flag indicates no special keys are depressed */
#define	KBF_NONE		(0)

/* special keys whose state we may monitor */
enum SKey {
	SKEY_NUMPAD_0 = 256,
	SKEY_NUMPAD_1 = 257,
	SKEY_NUMPAD_2 = 258,
	SKEY_NUMPAD_3 = 259,
	SKEY_NUMPAD_4 = 260,
	SKEY_NUMPAD_5 = 261,
	SKEY_NUMPAD_6 = 262,
	SKEY_NUMPAD_7 = 263,
	SKEY_NUMPAD_8 = 264,
	SKEY_NUMPAD_9 = 265,
	SKEY_NUMPAD_PERIOD = 266,
	SKEY_NUMPAD_DIVIDE = 267,
	SKEY_NUMPAD_MULTIPLY = 268,
	SKEY_NUMPAD_MINUS = 269,
	SKEY_NUMPAD_PLUS = 270,
	SKEY_NUMPAD_ENTER = 271,
	SKEY_NUMPAD_EQUALS = 272,
	SKEY_UP_ARROW = 273,
	SKEY_DOWN_ARROW = 274,
	SKEY_RIGHT_ARROW = 275,
	SKEY_LEFT_ARROW = 276,
	SKEY_INSERT = 277,
	SKEY_HOME = 278,
	SKEY_END = 279,
	SKEY_PGUP = 280,
	SKEY_PGDN = 281,
	SKEY_F1 = 282,
	SKEY_F2 = 283,
	SKEY_F3 = 284,
	SKEY_F4 = 285,
	SKEY_F5 = 286,
	SKEY_F6 = 287,
	SKEY_F7 = 288,
	SKEY_F8 = 289,
	SKEY_F9 = 290,
	SKEY_F10 = 291,
	SKEY_F11 = 292,
	SKEY_F12 = 293,
	SKEY_F13 = 294,
	SKEY_F14 = 295,
	SKEY_F15 = 296,
	SKEY_NUMLOCK = 300,
	SKEY_CAPSLOCK = 301,
	SKEY_SCLOCK = 302,
	SKEY_RSHIFT = 303,
	SKEY_LSHIFT = 304,
	SKEY_RCTRL = 305,
	SKEY_LCTRL = 306,
	SKEY_RALT = 307,
	SKEY_LALT = 308,
	SKEY_RMETA = 309,
	SKEY_LMETA = 310,
	SKEY_LWINDOWS = 311,
	SKEY_RWINDOWS = 312,
        SKEY_LGUI = 311,
        SKEY_RGUI = 312,
	SKEY_ALTGR = 313,
	SKEY_COMPOSE = 314,
	SKEY_HELP = 315,
	SKEY_PRINT = 316,
	SKEY_SYSREQ = 317,
	SKEY_BREAK = 318,
	SKEY_MENU = 319,
	SKEY_POWER = 320,
	SKEY_EURO = 321,
	SKEY_UNDO = 322,

	/* these are ASCII, but might as well define them somewhere */
	SKEY_ENTER = 13,
	SKEY_ESCAPE = 27,
	SKEY_TAB = 9,
	SKEY_BACKSPACE = 8,
	SKEY_BELL = 7,
	SKEY_SPACE = 32,
	SKEY_NUL = 0,
	SKEY_ACK = 6,
	SKEY_NAK = 21,
	SKEY_EOT = 4,
	SKEY_FORMFEED = 12,
	SKEY_LINEFEED = 10,
	SKEY_VTAB = 11,
	SKEY_SHIFTOUT = 14,
	SKEY_SHIFTIN = 15,
	SKEY_END_OF_TRANSMISSION = 23,
	SKEY_DEL = 127,

	/* This is not actually a key, but may be used in some places to mean any key is acceptable */
	SKEY_ANY = -1,

	/* the highest value SKEY_* that I track in, e.g., KeyLast, plus 1 */
	SKEY_TRACK_MAX = 323,
};

/*** Are there keys waiting in the key buffer? (Meant to be compatible with the old MS-DOS function; does not
	record function keys, CTRL/ALT, etc. Set up traps for those, or use the extended functions.) ***/
extern bool kbhit(void);

/*** Get a key from the keyboard buffer. Removes the key from the buffer. Returns (-1) if nothing is waiting. ***/
extern int kb_getch(void);

/*** Get a key from the keyboard buffer. Does not remove the key from the buffer. Returns (-1) if nothing is waiting. ***/
extern int kb_peekch(void);

/*** Called from the event handler when a key is pressed. ***/
extern void kb_on_key_down(SDL_Keysym* keysym);

/*** Y/N prompt. Spin until the user presses 'y' or 'n'. Returns true iff yes. (ESC is handled as "no".) ***/
extern bool kb_prompt_yn(void);

/*** Register a callback function that is called when the specified key combination is depressed. ***/
/*** Note that the key will be shifted if either SHIFT key is depressed, and may be caps-locked. ***/
/*** Note: trapped keys are not inserted into the key buffer, so will not trigger kbhit() or be returned from kb_getch()/kb_peekch(). ***/
extern void kb_callback(int key, int kb_flags, KbCallback kb_callback, void* callback_args);

/*** ASCII equivalent for the symbolic SDL key code (unshifted or shifted). ***/
extern int ascii_from_keysym(SDL_Keysym* keysym);
extern int ascii_from_keysym_shifted(SDL_Keysym* keysym);

enum MouseButton {
	MOUSEBUTTON_LEFT = 0,
	MOUSEBUTTON_RIGHT,
	MOUSEBUTTON_MIDDLE,
	MOUSEBUTTON_MAX
};

/*** Keep track of whether keys have been pressed since the last call. ***/
/* (Also preserves mouse button state: why have another object for that? Maybe call this KeyMouseLast?) */
class KeyLast {
public:
	KeyLast();
	KeyLast(void* ds);

	/* Saves the current state of keyboard flags to compare against later. 
	   Do this at the end of your application's main loop. */
	void save(void* display);	
	void save();

	/* Has the specified key been depressed or released since the last state save? */
	bool now_down(void* display, int keycode);
	bool now_up(void* display, int keycode);
	bool now_down(int keycode);
	bool now_up(int keycode);

	/* Bonus: has save been called yet? Can use this to determine if we're in the main
	   loop for the first time, and save a static bool. */
	bool first() const	{ return !saved; }

	/* Bonus bonus: mouse button tracking! */
	bool mouse_now_down(void* display, enum MouseButton mb);
	bool mouse_now_up(void* display, enum MouseButton mb);
	bool mouse_now_down(enum MouseButton mb);
	bool mouse_now_up(enum MouseButton mb);

        /* Bonus bonus bonus: return the value from kbmap! */
        bool kbmap_state(int keycode) const { return kbmap[keycode]; }

private:
	bool kbmap[SKEY_TRACK_MAX];
	bool mouse_state[MOUSEBUTTON_MAX];
	bool saved;
	void* display_saved;
};

#endif  // SDL_KB_OK

#endif  // __KB_H
