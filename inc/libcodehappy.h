/***

	libcodehappy.h

	Cross-platform app builder: can produce in-browser WASM/Emscripten apps, or
	native apps with or without SDL on Win64 or Linux.

	Includes lots and lots of other useful bits from my enormous C library.

	Just call codehappy_main() with a pointer to your app's main loop function.
	It will be called on every refresh (about 40 times/second by default.) The
	first argument to your main loop will be a pointer to the app's display context.
	You can modify the display directly using the power of the complete
	Codehappy 2-D gfx library, with extensive drawing primitives, floodfills,
	pattern fills, anti-aliasing, blitting, text drawing, etc. You may pass a void
	pointer to codehappy_main() that will be given to your main loop function as the
	second argument.

	Copyright (c) Chris Street, 2022-2023

***/
#ifndef LIBCODEHAPPY
#define LIBCODEHAPPY

#ifdef CODEHAPPY_CUDA
#ifndef CODEHAPPY_NATIVE
#define CODEHAPPY_NATIVE
#endif
#endif  // CODEHAPPY_CUDA

#ifdef CODEHAPPY_SDL_NATIVE
#ifndef CODEHAPPY_NATIVE_SDL
#define CODEHAPPY_NATIVE_SDL
#endif
#endif

#ifdef CODEHAPPY_NATIVE_SDL
#ifndef CODEHAPPY_NATIVE
#define CODEHAPPY_NATIVE
#endif
#ifndef CODEHAPPY_SDL
#define CODEHAPPY_SDL
#endif
#endif  // CODEHAPPY_NATIVE_SDL

#ifndef CODEHAPPY_NATIVE
#ifndef CODEHAPPY_WASM
#define CODEHAPPY_WASM
#endif
#ifndef CODEHAPPY_SDL
#define CODEHAPPY_SDL
#endif
#endif  // !CODEHAPPY_NATIVE

#if defined(_WIN32) || defined(WIN32) || defined(WINDOWS) || defined(_WIN64) || defined(WIN64)
#if !defined(__linux__) && !defined(LINUX)
#define CODEHAPPY_WINDOWS
#endif
#endif

#if defined(__x86_64__) || defined(_M_X64)
#define CODEHAPPY_X86_64
#endif

#ifdef _MSC_VER
#define CODEHAPPY_MSFT
#endif

#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_WINDOWS)
/* TODO: add pdcurses build back for Win64 */
//#define PDCURSES_OK
#endif

#if defined(__linux__) || defined(LINUX) || defined(linux) || defined(__UNIX__) || defined(UNIX)
#define CODEHAPPY_LINUX
#endif

#if defined(CODEHAPPY_WINDOWS) || defined(__linux__) || defined(LINUX)
#ifdef CODEHAPPY_NATIVE
#define CLIPBOARD_SUPPORT
#endif  // Native build
#endif  // Windows or Linux

#if defined(CODEHAPPY_NATIVE) && !defined(CODEHAPPY_WINDOWS) && !defined(CODEHAPPY_LINUX)
#define CODEHAPPY_LINUX
#endif

#ifdef CODEHAPPY_SDL
#ifdef CODEHAPPY_WINDOWS
#include "external/sdl/SDL.h"
#include "external/sdl/SDL_mixer.h"
#else
// Emscripten provides their own headers for these
#include <SDL.h>
#include <SDL_mixer.h>
#endif  // !CODEHAPPY_WINDOWS
#endif  // CODEHAPPY_SDL

#ifdef CODEHAPPY_WASM
#include <emscripten.h>
#include <emscripten/fetch.h>
#endif

#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_SDL)
/* some differences between SDL 1.2.15 API and WASM's implementation of SDL 1 API. */
typedef SDL_keysym SDL_Keysym;
#define	KMOD_GUI	KMOD_META
#endif

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <cassert>
#include <mutex>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>
#ifndef M_PI
#define M_PI	3.14159265358979323846
#endif

#ifdef CODEHAPPY_WINDOWS
#include <windows.h>
#undef  MOUSE_MOVED
#endif

/*** A scope mutex: construct this to lock the specified mutex, and when it's destroyed 
     at end of scope it auto-unlocks. (Can also be explicitly unlocked.) ***/
class ScopeMutex {
public:
	ScopeMutex(std::mutex& mtx_) : mtx(mtx_) {
		mtx.lock();
		locked = true;
	}
	~ScopeMutex() {
		unlock();
	}
	void unlock() {
		if (locked)
			mtx.unlock();
		locked = false;
	}

private:
	std::mutex& mtx;
	bool locked;
};

/*** Generally useful definitions ***/
#include "general.h"

/*** Unicode (UTF-32, UTF-16, UTF-8) string support ***/
#include "unicode.h"

/*** Various low level bit-twiddling utility functions. ***/
#include "bits.h"

/*** High-quality random number generation ***/
#include "rand.h"

/*** Miscellaneous functions. ***/
#include "misc.h"

/*** Cross-platform file and directory support. ***/
#include "files.h"

/*** Dynamic memory buffers, used in ramfiles &c. ***/
#include "scratchpad.h"

/*** Declarations and definitions intended to help porting old library code. ***/
#include "port.h"

/*** 128/256 unsigned integer support. ***/
#include "external/uint128_t.h"
#include "external/uint256_t.h"

/*** Featureful 2-D drawing and bitmap operations. ***/
#include "drawing.h"

/*** Color operations. ***/
#include "colors.h"

/*** Color space conversions. ***/
#include "space.h"

/*** Palette functions. ***/
#include "palette.h"

/*** Sound rendering, PCM WAV support, etc. ***/
#include "wavrender.h"

/*** Some basic user input functions. ***/
#include "input.h"

/*** Text manipulation, for parsers, tokenizers, lexers, etc. ***/
#include "parser.h"

/*** Grab bag containers. ***/
#include "grabbag.h"

/*** Efficient LZ compression. ***/
#include "external/lzf.h"

/*** Portability macros for byte endianness. ***/
#include "external/endian.h"

/*** Bigint support. ***/
#include "external/bigint.h"

/*** Arithmetic with saturation. ***/
#include "overflow.h"

/*** RAM files ***/
#include "ramfiles.h"

/*** Wrappers for wget. ***/
#include "wget.h"

/*** Miscellaneous contributed functions. ***/
#include "extern.h"

/*** Calendar, date, and time functions. ***/
#include "calendar.h"

/*** CSV load, save & edit support. ***/
#include "csv.h"

/*** GIF save support. ***/
#include "gif.h"

/*** Quantize and dither bitmaps. ***/
#include "quantize.h"

/*** PCX load and save support. ***/
#include "pcx.h"

/*** Rotation. ***/
#include "rotate.h"

/*** Roman numerals. ***/
#include "roman.h"

/*** English plurals. ***/
#include "plurals.h"

/*** Permutations and subsets. ***/
#include "permute.h"

/*** Music synthesis instrument patch support. ***/
#include "patch.h"

/*** x86/x64 assembly routines, for the native library. ***/
#include "asm.h"

/*** Bitwise I/O for files and memory buffers. ***/
#include "bitfile.h"

/*** Entropy compression and decompression. ***/
#include "entropy.h"

/*** Common string table. ***/
#include "strtable.h"

/*** OGG/Vorbis support. ***/
#define STB_VORBIS_HEADER_ONLY
#include "external/stb_vorbis.cpp"

/*** MP3 decoding. ***/
#include "external/minimp3_ext.h"

/*** Colored console text fprintf(). ***/
#include "external/console-colors.h"

/*** Type safe binary search. ***/
#include "external/asearch.h"

/*** An improved sprintf(). ***/
#include "sprintf.h"

/*** SQLite! ***/
//#ifdef CODEHAPPY_NATIVE
#include "external/sqlite3.h"
//#endif

/*** SVG icons, including a number of UI elements. ***/
#include "embedsvg.h"

/*** Low-level keyboard handling. ***/
#include "kb.h"

/*** UI controls (for native+SDL or WASM applications.) ***/
#include "ui.h"

/*** Libdivide ***/
#include "external/libdivide.h"

/*** Point, rect, sqrect operations ***/
#include "ref.h"

/*** Quadtrees. ***/
#include "quadtree.h"

/*** Primality testing. ***/
#include "isprime.h"

/*** Pattern-defeating quicksort. ***/
#include "external/pdqsort.h"

#if 0
/*** Nuklear minimal windows/UI code. ***/
#include "external/nuklear.h"
#endif

/*** PDCurses console I/O library. ***/
#ifdef PDCURSES_OK
#include "external/pdcurses/curses.h"
#include "external/pdcurses/panel.h"
#endif

/*** ANSI console output. ***/
#include "ansi.h"

/*** Drop-in replacement for the old <conio.h>. ***/
#include "conio.h"

/*** ISO currency codes. ***/
#include "currcode.h"

/*** Tribools and FuzzyBools. ***/
#include "tribool.h"

/*** Fixed length string support (ASCII/8-bit and Unicode). ***/
#include "fixedstr.h"

/*** Support for ChrisDraw format. ***/
#include "chrisdraw.h"

/*** Stopwatch object for timing. ***/
#include "stopwatch.h"

/*** Clipboard support on Windows or X11. ***/
#ifdef CLIPBOARD_SUPPORT
#include "external/clipboard.h"
#endif

/*** LibTom: big integer and floating-point math (including number theoretical functions). ***/
#include "external/libtom.h"

/*** Support for encoding/decoding a few video codecs like Cinepak and MS Video 1. ***/
#include "external/vcodec.h"

/*** Genann, a simple, minimalist, free artificial neural network library. ***/
#include "external/genann.h"

/*** KANN artificial neural network library. ***/
#include "external/kann.h"
#include "external/kautodiff.h"

/*** File dialogs, etc. for console or SDL apps. ***/
#ifdef CODEHAPPY_NATIVE
#ifdef INCLUDE_FILE_DIALOGS
// This requires linking with ComDlg32 (-lcomdlg32)
#include "external/tinyfiledialogs.h"
#endif
#endif

/*** Pretty printing functions. ***/
#include "prettyprint.h"

/*** Tiny MIDI parser. ***/
#include "external/tml.h"

/*** Tiny SoundFont2 load & synthesis support. ***/
#include "external/tsf.h"

/*** Fast Fourier transforms. ***/
#include "external/fft.h"

/*** ISAAC CSPRNGs. ***/
#include "external/isaac.h"
#include "external/isaac64.h"

/*** Platform-specific strong PRNGs. ***/
#include "rng_os.h"

/*** Vectors, square matrices (2d/3d/4d), and quaternions. ***/
#include "external/noc_vec.h"

/*** Image processing neural network training, evaluation, and modeling functions. ***/
#include "imgnnet.h"

/*** The pi function, and nth_prime(), next_prime(), previous_prime(). ***/
#include "pifunc.h"

/*** Genetic optimization. ***/
#include "genetic.h"

/*** Automated function inversion using FFANNs. ***/
#include "invfn.h"

/*** Numeric logical requirements. ***/
#include "require.h"

/*** Circular buffers. ***/
#include "circbuf.h"

/*** RPC calls to Python. ***/
#include "python_rpc.h"

/*** Command line argument parsing and processing. ***/
#include "argparse.h"

/*** Pearson chi-squared test and statistical utility functions. ***/
#include "stats.h"

/*** Elo rating calculator. ***/
#include "elo.h"

/*** API to the GPT-NeoX-20B large language model. ***/
#include "gpt-neox.h"

/*** String splits. ***/
#include "split.h"

/*** Text datasets. ***/
#include "textdataset.h"

/*** GGML -- extremely useful C++ deep learning library ***/
#define GGML_USE_K_QUANTS
#ifdef CODEHAPPY_CUDA
#define GGML_USE_CUBLAS
#endif
#define LLAMA_API_INTERNAL
#include "external/ggml/common.h"
#include "external/ggml/llama.h"
#include "external/ggml/grammar-parser.h"
#include "external/ggml/sampling.h"
#include "external/ggml/train.h"
#include "external/ggml/llava.h"
#include "external/ggml/clip.h"

/*** Stable Diffusion inference in ggml ***/
#ifdef CODEHAPPY_CUDA
#define SD_USE_CUBLAS
#endif
#include "external/stable-diffusion/stable-diffusion.h"
#include "external/stable-diffusion/rng.hpp"
#include "external/stable-diffusion/rng_philox.hpp"
#include "external/stable-diffusion/model.h"
#include "external/stable-diffusion/util.h"

/*** Language model embeddings. ***/
#include "lmembed.h"

/*** Llama LM inference. ***/
#include "llama.h"

/*** Latent diffusion model code (incl. SDServer) ***/
#include "ldm.h"

/*** BERT model ggml support. ***/
#include "external/bert.h"

/*** BERT embedding manager. ***/
#include "bert.h"

/*** the Segment Anything model (SAM) ***/
#include "sam.h"

/*** reading EXIF data from image files. ***/
#include "exif.h"

/*** fast search & replace tree, fill it with terms to search and do S&R with them. much faster than repeated p_replace(). ***/
#include "search-tree.h"

/*** an expression evaluator. supports user-defined operators, functions, etc. with definable order of operations ***/
#include "expeval.h"

/*** Application's main declaration. ***/
#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_SDL) && defined(CODEHAPPY_WINDOWS)
#define app_main()	WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevIns, LPSTR lpszArgument, int iShow)
#else
#define app_main()	main(int argc, const char* argv[])
#endif

/*** Macros for accessing command line arguments from app_main(). ***/
#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_SDL) && defined(CODEHAPPY_WINDOWS)
#define	app_argc()	libcodehappy_argc((const char*)lpszArgument)
#define	app_argv(i)	libcodehappy_argv((const char*)lpszArgument, i)
extern u32 libcodehappy_argc(const char* arg);
extern const char* libcodehappy_argv(const char* args, u32 i);
#else
#define	app_argc()	(argc)
#define	app_argv(i)	(argv[i])
#endif  // Native SDL Windows

/*** The library's VerboseStreams, for e.g. reporting errors. ***/
extern VerboseStream codehappy_cerr;
extern VerboseStream codehappy_cout;
extern void codehappy_verbose(bool v);
extern bool is_codehappy_verbose();

#ifdef CODEHAPPY_SDL

/*** The type of the main loop callback. ***/
class Display;
typedef void (*MainLoopCallback)(Display*, void*);

typedef u32 UICONTROL_HANDLE;
const UICONTROL_HANDLE UICONTROL_INVALID = 0xFFFFFFFFUL;

/*** The console. ***/
class Console {
public:
	Console();
	~Console();

	// Render to the passed-in bitmap.
	void render(SBitmap* bmp);

	// Add a character to the console.
	void addch(int ch);

	// Set the console font.
	void set_font(Font* nfont) { font = nfont; }
	void set_font(Font& nfont) { font = &nfont; }

private:
	u32 cur_line;
	u32 top_line;
	u32 line_height;
	u32 lines_scr;
	Font* font;
	std::vector<Scratchpad *> lines;
};

/*** The main Display object represents the canvas of the app. ***/
class Display {
public:
	~Display();

	/* Return information about mouse state. */
	int mouse_x(void) const { return mx; }
	int mouse_y(void) const { return my; }
	bool mouse_l(void) const { return ml; }
	bool mouse_r(void) const { return mr; }
	bool mouse_m(void) const { return mm; }

	/* Dimensions of the display. */
	u32 width(void) const { return surface->w; }
	u32 height(void) const { return surface->h; }

	/* Put or get a pixel directly. More complicated drawing should be done through bitmap(). */
	void put_pixel(int x, int y, RGBColor c);
	RGBColor get_pixel(int x, int y) const;

	/* Return an SBitmap representing the drawing surface. Any
		drawing on this object will modify the application canvas!
		The pointer is owned by this object, do not destroy it. */
	SBitmap* bitmap(void) const;

	/* Returns whether the passed key is currently held down. Shifted/caps lock doesn't count. */
	bool key_down(int key) const;

	/* Returns whether a special key (non-ASCII) is currently held down. See the SKey enum in kb.h. */
	bool special_down(SKey key) const;

	/* Are we in console mode? */
	bool console_mode(void) const { return in_console; }

	/* Set console mode on or off. */
	void set_console_mode(bool console);

	/* Close the window. */
	void close_window();

	/* Add a new control to the display. */
	UICONTROL_HANDLE add_control(const UIButton& button);
	UICONTROL_HANDLE add_control(const UICheckbox& chkbx);
	UICONTROL_HANDLE add_control(const UIScrollbar& scrollbar);
	UICONTROL_HANDLE add_control(const UIScrollbarSet& scrollbars);
	UICONTROL_HANDLE add_control(const UIButtonGroup& button_grp);

	/* Return a pointer to a specified control. */
	UIControl* control(UICONTROL_HANDLE handle);

	/* If a UI control exists at the specified position on the display, return its pointer, else nullptr. */
	UIControl* control_at_pos(const SPoint& p);

	/* Operations to perform on controls. */
	void activate_control(UICONTROL_HANDLE handle);
	void deactivate_control(UICONTROL_HANDLE handle);
	bool is_active(UICONTROL_HANDLE handle);
	void hide_control(UICONTROL_HANDLE handle);
	void unhide_control(UICONTROL_HANDLE handle);
	bool is_hidden(UICONTROL_HANDLE handle);
	void remove_control(UICONTROL_HANDLE handle);

private:
	/* Private constructor -- only our friends get to create us. There should only be
		one Display object, otherwise bad things might happen. */
	Display(u32 width, u32 height, MainLoopCallback loop, void* user_data);

	/* The internal main loop that handles SDL events, etc. */
	static void internal_main_loop(void);

	/* Perform UI draw. */
	void ui_frame_draw(void);

	/* Send UI mouse messages. */
	void ui_mouse_move(void);

	/* Send UI mouse clicks. */
	// TODO: allow UI events on middle mouse button click/release?
	void ui_mouse_click(bool isright, bool isdown);

	/* Send UI keyboard events. */
	void ui_keyboard_event(bool isup, SDL_Keysym* event);

	/* Lock/unlock mutex to access UI controls. */
	void ui_lock(void);
 	void ui_unlock(void);

	/* Mouse position and button state. */
	int mx, my;
	bool ml, mr, mm;

	/* An SBitmap that we can return. */
	SBitmap* i_bitmap;

	/* The client app's main loop. */
	MainLoopCallback app_main_loop;

	/* A user-provided void pointer to pass along to the app's main loop. */
	void* app_data;

	/* The SDL surface. */
	SDL_Surface* surface;

	/* Are we in console mode? */
	bool in_console;

	/* Have we received a QUIT message from the frontend (SDL)? */
	static bool time_to_die;

	/* The console */
	Console console;

	/* Controls managed by the UI. */
	std::unordered_map<UICONTROL_HANDLE, UIControl*> controls;
	UICONTROL_HANDLE next_handle;
	UIControl* ui_focus;

	/* UI mutex. */
	std::mutex ui_mutex;

	/* Our good friend. */
	friend void codehappy_main(MainLoopCallback main_loop_fn, void* user_data, u32 width, u32 height, u32 fps);
};

/*** UI/SDL specific includes that reference Display/Console ***/

/*** Easy way to get text input in a GUI application (similar to a text box/HTML form, but created & updated with a minimum of code in your main loop) ***/
#include "autoconsole.h"

/***
	Call this to launch the application. Your main loop callback has the prototype

		void main_loop(Display* display_context, void* user_data);

	The application's canvas will be created, and on every refresh your app's main
	loop will be called with a pointer to the Display object and the user_data.
***/
extern void codehappy_main(MainLoopCallback main_loop_fn, void* user_data, u32 width, u32 height, u32 fps = 0);

/***
	Call this to stop the Emscripten main loop. You won't get any more SDL updates, and the
	app main loop callback will no longer be called. Can be done if the app is "finished".
***/
extern void codehappy_stop(void);

/***
	Call this to stop SDL events and close the main application window. Control will return
	to the application's main function.
***/
extern void codehappy_close_window(void);

/***
	Call this to turn on or off console mode.
***/
extern void codehappy_console(bool is_on);

/***
	Call this to change the window title in native builds. In WASM this attempts
	to change document.title.
***/
extern void codehappy_window_title(const char* new_title);

// TODO: Can't set the window's icon with SDL unless I adopt v2.0.

/* Initialize SDL without creating a Display object (good for SDL terminal applications -- using audio, e.g.) */
extern void codehappy_init_audiovisuals();

/*** WASM/Emscripten specific stuff below. ***/

/*** Functions that allow HTTP requests or persistent storage. ***/

#ifdef CODEHAPPY_WASM
/***
	Save the passed-in data to a file in the virtual (IndexedDB) filesystem.
***/
extern void codehappy_persist_file(const char* fname, u8* data, u32 num_bytes);

/***
	Delete the named file in the virtual (IndexedDB) filesystem.
***/
extern void codehappy_delete_file(const char* fname);

/***
	Make an HTTP request, synchronously. Returns nullptr on failure, or the data in a ramfile on success.
***/
extern RamFile* codehappy_URI_fetch(const char* URI);

/***
	Begin an asynchronous HTTP request. Give a callback to receive the ramfile when done, or to handle errors.
	The error handling callback can be nullptr, or the same as the on_success callback (when called on failure,
	the ramfile is always nullptr.) 
***/
typedef void (*AsyncHTTPCallback)(RamFile *);
extern void codehappy_URI_fetch_async(const char* URI, AsyncHTTPCallback on_success, AsyncHTTPCallback on_failure);

/***
	Alternative async HTTP request API w/o callbacks. This returns a handle representing the request; you poll via
	codehappy_async_fetch_done() to determine if the request is finished, and codehappy_async_fetch_data() to get
	the finished ramfile.
***/
extern u32 codehappy_URI_fetch_async(const char* URI);
extern bool codehappy_async_fetch_done(u32 handle);
extern RamFile* codehappy_async_fetch_data(u32 handle);

/***
	Exported function that can be called from Javascript: it's called by open_file(), which on file selection in JS
	sends the file data to us. See "template_w_file_dialog.html" for the definition of open_file. Note that if you
	compile a libcodehappy WASM app using this feature, you should add -sEXPORTED_RUNTIME_METHODS=[ccall] to your 
	compiler invocation to allow Javascript to call C functions.	
***/
extern "C" {
EMSCRIPTEN_KEEPALIVE int codehappy_file_from_js(u8 *buffer, u32 size);
}

/***
	If you've included -sEXPORTED_RUNTIME_METHODS=[ccall] in your build to allow Javascript to call our C functions,
	then you can call this function to pull up a file dialog and the file contents, on a successful selection, will 
	be sent to codehappy_file_from_js() and can be retrieved using the functions below.

	'filter' can be nullptr to permit all files in the dialog by default, or it can be a comma-seperated list of
	file extensions (e.g., ".png,.jpg,.jpeg,.gif") that are valid for the file dialog.

	The return value is a handle for the new file that can be passed to codehappy_js_file(). Although this is
	an u32, don't assume anything about order; it's meaningless besides representing this particular data.
***/
extern u32 codehappy_file_selection_from_js(const char* filter);

/***
	Is the file with the specified handle available from codehappy_js_file()?
***/
extern bool codehappy_js_file_available(u32 handle);

/***
	Fetch a file given to us by a JavaScript FileReader. "handle" is the file's handle.
	Returns nullptr if the file is not (yet) available. If bytes_out is non-NULL, the length
	of the file buffer in bytes is set in bytes_out.
***/
extern u8* codehappy_js_file(u32 handle, u32* bytes_out);

#endif  // CODEHAPPY_WASM

/*** Return the current application frame number (the number of times through the libcodehappy main loop.)
	Used by UI controls, stopwatches, and animation objects. ***/
extern u32 codehappy_frame(void);

#endif  // CODEHAPPY_SDL

#endif  // LIBCODEHAPPY
