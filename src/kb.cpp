/***

	kb.cpp

	Low-level keyboard access.

	Currently this hooks into the SDL's keyboard handler, but if other windowing system
	libraries are supported in the future, code may be written to support them as well.

	Copyright (c) 2014-2022, Chris Street

***/

#ifdef CODEHAPPY_SDL

#include "libcodehappy.h"

#include <vector>
#include <mutex>
#include <algorithm>
#include <utility>
#include <unordered_map>

#define	SCANCODE(x, y)		{ x, y },

/*** This array converts SDL scancode values to an unshifted ASCII, where possible. ***/
/*** Scancodes represent a physical key, so translation tables have to be used to get the shifted character. ***/
static int __scancode2ascii[][2] =
{
#ifndef CODEHAPPY_NATIVE
	SCANCODE(SDL_SCANCODE_A, 'a')
	SCANCODE(SDL_SCANCODE_B, 'b')
	SCANCODE(SDL_SCANCODE_C, 'c')
	SCANCODE(SDL_SCANCODE_D, 'd')
	SCANCODE(SDL_SCANCODE_E, 'e')
	SCANCODE(SDL_SCANCODE_F, 'f')
	SCANCODE(SDL_SCANCODE_G, 'g')
	SCANCODE(SDL_SCANCODE_H, 'h')
	SCANCODE(SDL_SCANCODE_I, 'i')
	SCANCODE(SDL_SCANCODE_J, 'j')
	SCANCODE(SDL_SCANCODE_K, 'k')
	SCANCODE(SDL_SCANCODE_L, 'l')
	SCANCODE(SDL_SCANCODE_M, 'm')
	SCANCODE(SDL_SCANCODE_N, 'n')
	SCANCODE(SDL_SCANCODE_O, 'o')
	SCANCODE(SDL_SCANCODE_P, 'p')
	SCANCODE(SDL_SCANCODE_Q, 'q')
	SCANCODE(SDL_SCANCODE_R, 'r')
	SCANCODE(SDL_SCANCODE_S, 's')
	SCANCODE(SDL_SCANCODE_T, 't')
	SCANCODE(SDL_SCANCODE_U, 'u')
	SCANCODE(SDL_SCANCODE_V, 'v')
	SCANCODE(SDL_SCANCODE_W, 'w')
	SCANCODE(SDL_SCANCODE_X, 'x')
	SCANCODE(SDL_SCANCODE_Y, 'y')
	SCANCODE(SDL_SCANCODE_Z, 'z')
	SCANCODE(SDL_SCANCODE_1, '1')
	SCANCODE(SDL_SCANCODE_2, '2')
	SCANCODE(SDL_SCANCODE_3, '3')
	SCANCODE(SDL_SCANCODE_4, '4')
	SCANCODE(SDL_SCANCODE_5, '5')
	SCANCODE(SDL_SCANCODE_6, '6')
	SCANCODE(SDL_SCANCODE_7, '7')
	SCANCODE(SDL_SCANCODE_8, '8')
	SCANCODE(SDL_SCANCODE_9, '9')
	SCANCODE(SDL_SCANCODE_0, '0')
	SCANCODE(SDL_SCANCODE_RETURN, '\n')
	SCANCODE(SDL_SCANCODE_ESCAPE, '\033')
	SCANCODE(SDL_SCANCODE_BACKSPACE, '\010')
	SCANCODE(SDL_SCANCODE_TAB, '\011')
	SCANCODE(SDL_SCANCODE_SPACE, ' ')
	SCANCODE(SDL_SCANCODE_MINUS, '-')
	SCANCODE(SDL_SCANCODE_EQUALS, '=')
	SCANCODE(SDL_SCANCODE_LEFTBRACKET, '[')
	SCANCODE(SDL_SCANCODE_RIGHTBRACKET, ']')
	SCANCODE(SDL_SCANCODE_BACKSLASH, '\\')
	SCANCODE(SDL_SCANCODE_NONUSHASH, '#')
	SCANCODE(SDL_SCANCODE_SEMICOLON, ';')
	SCANCODE(SDL_SCANCODE_APOSTROPHE, '\'')
	SCANCODE(SDL_SCANCODE_GRAVE, '`')
	SCANCODE(SDL_SCANCODE_COMMA, ',')
	SCANCODE(SDL_SCANCODE_PERIOD, '.')
	SCANCODE(SDL_SCANCODE_SLASH, '/')
	SCANCODE(SDL_SCANCODE_KP_DIVIDE, '/')
	SCANCODE(SDL_SCANCODE_KP_MULTIPLY, '*')
	SCANCODE(SDL_SCANCODE_KP_MINUS, '-')
	SCANCODE(SDL_SCANCODE_KP_PLUS, '+')
	SCANCODE(SDL_SCANCODE_KP_ENTER, '\n')
	SCANCODE(SDL_SCANCODE_KP_1, '1')
	SCANCODE(SDL_SCANCODE_KP_2, '2')
	SCANCODE(SDL_SCANCODE_KP_3, '3')
	SCANCODE(SDL_SCANCODE_KP_4, '4')
	SCANCODE(SDL_SCANCODE_KP_5, '5')
	SCANCODE(SDL_SCANCODE_KP_6, '6')
	SCANCODE(SDL_SCANCODE_KP_7, '7')
	SCANCODE(SDL_SCANCODE_KP_8, '8')
	SCANCODE(SDL_SCANCODE_KP_9, '9')
	SCANCODE(SDL_SCANCODE_KP_0, '0')
	SCANCODE(SDL_SCANCODE_KP_PERIOD, '.')
	SCANCODE(SDL_SCANCODE_NONUSBACKSLASH, '\\')
	SCANCODE(SDL_SCANCODE_KP_EQUALS, '=')
	SCANCODE(SDL_SCANCODE_KP_COMMA, ',')
	SCANCODE(SDL_SCANCODE_KP_EQUALSAS400, '=')
	SCANCODE(SDL_SCANCODE_KP_LEFTPAREN, '(')
	SCANCODE(SDL_SCANCODE_KP_RIGHTPAREN, ')')
	SCANCODE(SDL_SCANCODE_KP_LEFTBRACE, '{')
	SCANCODE(SDL_SCANCODE_KP_RIGHTBRACE, '}')
	SCANCODE(SDL_SCANCODE_KP_TAB, '\011')
	SCANCODE(SDL_SCANCODE_KP_BACKSPACE, '\010')
	SCANCODE(SDL_SCANCODE_KP_A, 'a')
	SCANCODE(SDL_SCANCODE_KP_B, 'b')
	SCANCODE(SDL_SCANCODE_KP_C, 'c')
	SCANCODE(SDL_SCANCODE_KP_D, 'd')
	SCANCODE(SDL_SCANCODE_KP_E, 'e')
	SCANCODE(SDL_SCANCODE_KP_F, 'f')
	SCANCODE(SDL_SCANCODE_KP_PERCENT, '%')
	SCANCODE(SDL_SCANCODE_KP_LESS, '<')
	SCANCODE(SDL_SCANCODE_KP_GREATER, '>')
	SCANCODE(SDL_SCANCODE_KP_AMPERSAND, '&')
	SCANCODE(SDL_SCANCODE_KP_VERTICALBAR, '|')
	SCANCODE(SDL_SCANCODE_KP_COLON, ':')
	SCANCODE(SDL_SCANCODE_KP_HASH, '#')
	SCANCODE(SDL_SCANCODE_KP_SPACE, ' ')
	SCANCODE(SDL_SCANCODE_KP_AT, '@')
	SCANCODE(SDL_SCANCODE_KP_EXCLAM, '!')
#endif  // !CODEHAPPY_NATIVE
};

/*** Translate an unshifted ASCII character to its shifted equivalent. ***/
static int __ascii2shiftascii[] =
{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 								// 0 - 15
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 								// 16 - 31
	0, 0, 0, 0, 0, 0, 0, '\"', 0, 0, 0, 0, '<', '_', '>', '?', 						// 32 - 47
	')', '!', '@', '#', '$', '%', '^', '&', '*', '(', 0, ':', 0, '+', 0, 0, 		// 48 - 63
	0, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',	// 64 - 79 
	'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', 0, 0, 	// 80 - 95
	'~', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',	// 96 - 111 
	'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 0, 0, 0, 0, 0, 		 	// 112 - 127
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 	// 128 - 255
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};

/***

	Alternate scancode translations by keyboard layout exist for the following:
	
	SDL_SCANCODE_BACKSLASH	
	SDL_SCANCODE_GRAVE		
	SDL_SCANCODE_NONUSBACKSLASH

	Probably others, too.

	Most of these translations should be handled by the SDL scancode-to-keycode translation,
	but might want to keep eyes out for any layout problems.

***/

// TODO: handle layout translations for US Mac/Windows, UK Mac/Windows, Swiss German, German, French, at least.

static int __scancode_comp(const void* v1, const void* v2)
{
	const int* s1 = (const int*)v1;
	const int* s2 = ((const int*)v2);

	return((*s1) - (*s2));
}

static int _scancode_bsearch(const int search_for)
{
#if 0
	int** c;
	u32 idx;

	c = (int**)asearch(&search_for, __scancode2ascii, array_size(__scancode2ascii), __scancode_comp);

	if (is_null(c))
		return(-1);

	idx = ((c - (int**)__scancode2ascii) >> 1);

	return (__scancode2ascii[idx][1]);
#else
	const u32 sz = sizeof(__scancode2ascii) / sizeof(__scancode2ascii[0]);
	for (u32 i = 0; i < sz; ++i) {
		if (__scancode2ascii[i][0] == search_for)
			return __scancode2ascii[i][1];
	}
	return -1;
#endif
}

static bool _capslock = false;

struct kbcallbacks {
	u64			key_flags;
	std::vector<KbCallback>	kb_callbacks;
	std::vector<void *>	kb_callback_args;
	std::vector<bool>	kb_active;
};

/*** Global key buffer. OK that it's a static buffer -- keyboard event handler runs on one thread, and mutex protects
	it from other threads. ***/
static Scratchpad __sp_keybuffer(1024);
static std::mutex __key_mutex;
static std::unordered_map<u64, kbcallbacks> __kb_chash;

#define	KB_LOCK		__key_mutex.lock();
#define	KB_UNLOCK	__key_mutex.unlock();
#define	KB_ENSURE	{}

/*** Are there keys waiting in the key buffer? (Meant to be compatible with the old MS-DOS function; does not
	record function keys, CTRL/ALT, etc. Set up traps for those, or use the extended functions.) ***/
bool kbhit(void)
{
	bool ret;
	KB_ENSURE;
	KB_LOCK;
	ret = (__sp_keybuffer.length() > 0); 
	KB_UNLOCK;
	return(ret);
}

/*** Get a key from the keyboard buffer. Removes the key from the buffer. Returns (-1) if nothing is waiting. ***/
int kb_getch(void)
{
	int ret;
	KB_ENSURE;
	KB_LOCK;
	ret = __sp_keybuffer.getch();
	KB_UNLOCK;
	return(ret);
}

/*** Get a key from the keyboard buffer. Does not remove the key from the buffer. Returns (-1) if nothing is waiting. ***/
int kb_peekch(void)
{
	int ret = -1;
	KB_ENSURE;
	KB_LOCK;
	if (__sp_keybuffer.length() > 0)
		ret = __sp_keybuffer.buffer()[0];
	KB_UNLOCK;
	return(ret);
}

static u64 __kbc_hash(int key, int kb_flags)
{
	return (u64)key + ((u64)kb_flags << 32ULL);
}

bool __key_trapped(int ascii, u32 basic_flags, u32 all_flags)
{
	u64 hash1, hash2;
	kbcallbacks* kbcf;
	if (isupper(ascii))
		ascii = tolower(ascii);
	hash1 = __kbc_hash(ascii, (int)basic_flags);
	hash2 = __kbc_hash(ascii, (int)all_flags);
	bool ret = false;

	/* call keyboard traps */
	forever {
		if (__kb_chash.find(hash1) == __kb_chash.end())
			kbcf = nullptr;
		else
			kbcf = &__kb_chash[hash1];
		if (not_null(kbcf)) {
			int i;
			for (i = 0; i < kbcf->kb_callbacks.size(); ++i) {
				if (!kbcf->kb_active[i])
					continue;
				ret = true;
				if (kbcf->kb_callbacks[i](ascii, basic_flags, kbcf->kb_callback_args[i]))
					kbcf->kb_active[i] = false;
			}
		}
		if (hash1 == hash2)
			break;
		hash1 = hash2;
		basic_flags = all_flags;
	}

	return ret;
}

static int ascii_shifted_SDL1(SDL_Keysym* keysym) {
	// Use the Unicode mapping feature in SDL v.1 to do the shift for me.
	int unicode = (int)keysym->unicode;
	if (unicode > 255)	// we'll allow any code that fits in an u8, not just ASCII.
		return 0;
	return unicode;
}

// TODO: Unicode input support (can use function below)
static uch uch_shifted_SDL1(SDL_Keysym* keysym) {
	return (uch)keysym->unicode;
}

// This should give the unshifted, base key.
int ascii_from_keysym(SDL_Keysym* keysym) {
	int ascii = int(keysym->sym);

	if (ascii >= 256) {
#if 0
		// search in the scancode table
		int code = _scancode_bsearch(keysym->scancode);
		if (code != -1)
			ascii = code;
#else
		ascii = 0;
#endif
	}

	return(ascii);
}

int ascii_from_keysym_shifted(SDL_Keysym* keysym) {
#if 0
	int ascii = int(keysym->sym);

	if (ascii >= 256) {
		// search in the scancode table
		int code = _scancode_bsearch(keysym->scancode);
		if (code != -1)
			ascii = code;
	}
	if (keysym->mod & KMOD_SHIFT) {
		if (is_between(ascii, 0, 255)) {
			int code = __ascii2shiftascii[ascii];
			if (code != 0)
				ascii = code;
		}
	}
	if ((keysym->mod & KMOD_CAPS) != 0 || _capslock) {
		ascii = toupper(ascii);
	}

	return(ascii);
#else
	return ascii_shifted_SDL1(keysym);
#endif
}

/*** Called from the event handler when a key is pressed. ***/
void kb_on_key_down(SDL_Keysym* keysym) {
	int ascii;
	int code;
	
	if (unlikely(is_null(keysym)))
		return;
	// TODO: translate numlock + number pad keys to movement directions

	ascii = (int)keysym->sym;
#ifndef CODEHAPPY_NATIVE
	if (ascii >= 256) {
		// see if we have an ASCII equivalent for the physical scancode in the scancode table
		code = _scancode_bsearch(keysym->scancode);
		if (code != -1)
			ascii = code;
	}
#endif

	// If we get here, we're an ASCII (or UTF-8, etc.) code. Check for modifiers.
#if 0
	if (keysym->mod & KMOD_SHIFT) {
		// Shift the key.
		if (is_between(ascii, 0, 255)) {
			code = __ascii2shiftascii[ascii];
			if (code != 0)
				ascii = code;
		}
	}
	if (keysym->mod & KMOD_CAPS) {
		// Caps Lock is on.
		ascii = toupper(ascii);
	}
#else
	ascii = ascii_shifted_SDL1(keysym);
#endif

	// Check for key traps.
	// TODO: Check interplay of caps lock and shift?
	u32 basic_flags, all_flags;
	u64 hash;
	basic_flags = keysym->mod;
	// Turn caps lock flag off -- generally people don't want their key trap to depend on the caps lock state.
	basic_flags &= (~KMOD_CAPS);
	all_flags = basic_flags;
	if (nonzero(all_flags & KMOD_CTRL))
		all_flags |= KMOD_CTRL;
	if (nonzero(all_flags & KMOD_ALT))
		all_flags |= KMOD_ALT;
	if (nonzero(all_flags & KMOD_SHIFT))
		all_flags |= KMOD_SHIFT;
	if (nonzero(all_flags & KMOD_GUI))
		all_flags |= KMOD_GUI;
	if (__key_trapped(ascii, basic_flags, all_flags))
		return;
	// Send the key to the UI control with focus, if any
#if 0	// not yet ported
	if (main_window_handle_key(ascii, keysym->scancode, all_flags))
		return;
#endif
	if (ascii >= 256 || ascii < 0 || (keysym->mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI))) {
		// A special key that wasn't trapped -- return.
		return;
	}

	// A regular ol' key. Add it to the key buffer.
	KB_LOCK;
	__sp_keybuffer.addc(ascii);
	KB_UNLOCK;
}

/*** Y/N prompt. Spin until the user presses 'y' or 'n'. Returns true iff yes. (ESC is handled as "no".) ***/
bool kb_prompt_yn(void) {
	int k;
	forever
		{
		k = kb_getch();
		switch (k)
			{
		case 'Y':
		case 'y':
			return(true);
		case 'N':
		case 'n':
		case 033:
			return(false);
			}
		}
	return(false);
}

/*** Register a callback function that is called when the specified key combination is depressed. ***/
void kb_callback(int key, int kb_flags, KbCallback kb_callback, void* callback_args) {
	if (isupper(key))
		key = tolower(key);
	u64 hash = __kbc_hash(key, kb_flags);
	kbcallbacks& kbc = __kb_chash[hash];

	kbc.kb_callback_args.push_back(callback_args);
	kbc.kb_active.push_back(true);
	kbc.kb_callbacks.push_back(kb_callback);
}

KeyLast::KeyLast() {
	saved = false;
	display_saved = nullptr;
	for (int e = 0; e < SKEY_TRACK_MAX; ++e)
		kbmap[e] = false;
	for (int e = 0; e < 3; ++e)
		mouse_state[e] = false;
}

KeyLast::KeyLast(void* display) {
	saved = false;
	display_saved = display;
	for (int e = 0; e < SKEY_TRACK_MAX; ++e)
		kbmap[e] = ((Display*)display)->key_down(e);
	mouse_state[MOUSEBUTTON_LEFT] = ((Display*)display)->mouse_l();
	mouse_state[MOUSEBUTTON_RIGHT] = ((Display*)display)->mouse_r();
	mouse_state[MOUSEBUTTON_MIDDLE] = ((Display*)display)->mouse_m();
}

void KeyLast::save(void* display) {
#ifdef CODEHAPPY_WASM
/*
	for (int e = 0; e < SKEY_TRACK_MAX; ++e) {
		kbmap[e] = ((Display*)display)->key_down(e);
	}
*/	
	int nk = 0;
	u8* buf = (u8 *)SDL_GetKeyboardState(&nk);
	for (int e = 0; e < nk && e < SKEY_TRACK_MAX; ++e) {
		kbmap[e] = (buf[e] != 0);
	}
#else
	// use SDL_KeyState() for more responsiveness
	int nk;
	SDL_PumpEvents();
	u8* buf = (u8 *)SDL_GetKeyState(&nk);
	for (int e = 0; e < nk && e < SKEY_TRACK_MAX; ++e) {
		kbmap[e] = (buf[e] != 0);
	}
#endif	
	mouse_state[MOUSEBUTTON_LEFT] = ((Display*)display)->mouse_l();
	mouse_state[MOUSEBUTTON_RIGHT] = ((Display*)display)->mouse_r();
	mouse_state[MOUSEBUTTON_MIDDLE] = ((Display*)display)->mouse_m();
	saved = true;
	display_saved = display;
}

void KeyLast::save() {
	ship_assert(display_saved != nullptr);
	save(display_saved);
}

bool KeyLast::now_down(void* display, int keycode) {
	display_saved = display;
	if (keycode < 0 || keycode >= SKEY_TRACK_MAX)
		return false;
	if (kbmap[keycode])
		return false;
	return ((Display*)display)->key_down(keycode);
}

bool KeyLast::now_up(void* display, int keycode) {
	display_saved = display;
	if (keycode < 0 || keycode >= 128)
		return false;
	if (!kbmap[keycode])
		return false;
	return !(((Display*)display)->key_down(keycode));
}

bool KeyLast::now_down(int keycode) {
	ship_assert(display_saved != nullptr);
	return now_down(display_saved, keycode);
}

bool KeyLast::now_up(int keycode) {
	ship_assert(display_saved != nullptr);
	return now_up(display_saved, keycode);
}

bool KeyLast::mouse_now_down(void* display, enum MouseButton mb) {
	if (mb < MOUSEBUTTON_LEFT || mb >= MOUSEBUTTON_MAX)
		return false;
	if (mouse_state[mb])
		return false;
	switch (mb) {
	case MOUSEBUTTON_LEFT:
		return ((Display*)display)->mouse_l();
	case MOUSEBUTTON_RIGHT:
		return ((Display*)display)->mouse_r();
	case MOUSEBUTTON_MIDDLE:
		return ((Display*)display)->mouse_m();
	}
	return false;
}

bool KeyLast::mouse_now_up(void* display, enum MouseButton mb) {
	if (mb < MOUSEBUTTON_LEFT || mb >= MOUSEBUTTON_MAX)
		return false;
	if (!mouse_state[mb])
		return false;
	switch (mb) {
	case MOUSEBUTTON_LEFT:
		return !(((Display*)display)->mouse_l());
	case MOUSEBUTTON_RIGHT:
		return !(((Display*)display)->mouse_r());
	case MOUSEBUTTON_MIDDLE:
		return !(((Display*)display)->mouse_m());
	}
	return false;
}

bool KeyLast::mouse_now_down(enum MouseButton mb) {
	ship_assert(display_saved != nullptr);
	return mouse_now_down(display_saved, mb);
}


bool KeyLast::mouse_now_up(enum MouseButton mb) {
	ship_assert(display_saved != nullptr);
	return mouse_now_up(display_saved, mb);
}

#endif  // CODEHAPPY_SDL
/*** end kb.cpp ***/
