/***

	libcodehappy.cpp

	Implementation of codehappy_main(), basic Display methods, and other wrappers for
	important WASM/Emscripten functions (virtual filesystem management, HTTP GET requests to
	allow server-side data stores or computation, etc.) This library can also be built
	as a native console library or a native + SDL GUI application library on 64-bit
	Linux and Windows systems (at a minimum -- the majority of methods and objects are
	written to be decently portable, most have been tested and run under 32-bit Windows,
	and some have been tested on Android or Apple.)

	This is also the main source file -- all other libcodehappy source files are #included at
	the bottom of this file. Only a few external libraries in C are built and linked into the
	library separately: these include SQLite (such an awesome library) and pdcurses (because 
	Windows console support kind of stinks.)

	Copyright (c) Chris Street, 2022-2023

***/

/*** For the single source file build, get the image format and TrueType support. ***/
#include "external/stb_image_wrappers.i"

#include "libcodehappy.h"

#ifdef CODEHAPPY_SDL

#include <unordered_map>

static Display* __app_canvas = nullptr;

Console::Console() {
//	font = new Font(font_apple_ii_40);
	font = nullptr; // use the bit font by default for now, to avoid pulling font data into binaries that don't use the console.
	lines.push_back(new Scratchpad(64));
	cur_line = 0;
	top_line = 0;
	line_height = 12;
	lines_scr = 10;
}

Console::~Console() {
	for (u32 i = 0; i < lines.size(); ++i)
		delete lines[i];
	if (!is_null(font))
		delete font;
}

void Console::addch(int ch) {
	if (ch == '\n' || ch == '\r') {
		// New line.
		lines.push_back(new Scratchpad(64));
		++cur_line;
		if (top_line + lines_scr <= cur_line) {
			top_line = (cur_line - lines_scr) + 1;
		}
		return;
	}
	if (ch == 8) {
		// Backspace.
		u8* li = lines[cur_line]->buffer();
		if (*li) {
			li[strlen((char*)li) - 1] = 0;
			lines[cur_line]->update_str();
		}
		return;
	}
	if (ch == 9) {
		// Tab.
		for (int e = 0; e < 4; ++e)
			addch(' ');
		return;
	}
	if (ch < 32 || ch >= 127) {
		// Control code or non-ASCII, ignore.
		return;
	}
	lines[cur_line]->addc(u8(ch));
}

bool __allspaces(char* w) {
	while (*w) {
		if (!isspace(*w))
			return false;
		++w;
	}
	return true;
}

void Console::render(SBitmap* bmp) {
	u32 y = 4;
	if (is_null(bmp))
		return;
	bmp->clear();
	for (u32 i = 0; i < lines_scr; ++i) {
		if (i + top_line >= lines.size())
			break;
		Scratchpad* sp = lines[i + top_line];
		if (sp->length() == 0 || __allspaces((char*)sp->buffer())) {
			y += line_height;
			y += 4;
			continue;
		}
		if (!is_null(font)) {
			SBitmap* l = font->render_cstr((char *)sp->buffer(), 12, false, nullptr);
			if (l->height() > line_height) {
				line_height = l->height();
				lines_scr = (bmp->height() - 4) / (line_height + 4);
			}
			Font::blit(l, bmp, 3, y, C_WHITE);
			y += l->height();
			y += 4;
			delete l;
		} else {
			bmp->putstr_bitfont((const char *)sp->buffer(), 3, y, 1, C_WHITE);
			y += 12;
			if (10 == lines_scr)
				lines_scr = (bmp->height() - 4) / 12;
		}
	}
}

static bool __sdlinit = false;
void codehappy_init_audiovisuals() {
	if (__sdlinit)
		return;
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_JOYSTICK /* SDL_INIT_VIDEO */);
	SDL_EnableUNICODE(1);
	if (Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 4096) == 0) {
		//codehappy_cerr << "SDL mixer open error\n";
	}
	Mix_Init(MIX_INIT_FLAC | MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG);
	__sdlinit = true;
}

Display::Display(u32 width, u32 height, MainLoopCallback loop, void* user_data) {
	codehappy_init_audiovisuals();
	surface = SDL_SetVideoMode(width, height, 32, SDL_SWSURFACE);
	ship_assert(!is_null(surface));
	i_bitmap = new SBitmap(width, height, (u8*)surface->pixels);
	app_data = user_data;
	app_main_loop = loop;
	in_console = false;
	next_handle = 0UL;
	ui_focus = nullptr;
	ml = mr = mm = false;
	mx = my = 0;
}

bool Display::time_to_die = false;

Display::~Display() {
	if (i_bitmap != nullptr)
		delete i_bitmap;
}

SBitmap* Display::bitmap(void) const {
	return i_bitmap;
}

void Display::put_pixel(int x, int y, RGBColor c) {
	if (x < 0 || x >= surface->w)
		return;
	if (y < 0 || y >= surface->h)
		return;
	u32 *buf = (u32 *)surface->pixels;
	int off = y * surface->w + x;
	buf[off] = c;
}

RGBColor Display::get_pixel(int x, int y) const {
	if (x < 0 || x >= surface->w)
		return X_NEON_PINK;
	if (y < 0 || y >= surface->h)
		return X_NEON_PINK;
	u32 *buf = (u32 *)surface->pixels;
	int off = y * surface->w + x;
	return buf[off];
}

/* Status of keyboard scancodes. */
static std::unordered_map<int, bool> __keycode_down;

bool Display::key_down(int key) const {
	return __keycode_down[key];
}

bool Display::special_down(SKey key) const {
	return __keycode_down[int(key)];
}

void Display::set_console_mode(bool console) {
	in_console = console;
}

void Display::ui_lock(void) {
	ui_mutex.lock();
}

void Display::ui_unlock(void) {
	ui_mutex.unlock();
}

UICONTROL_HANDLE Display::add_control(const UIButton& button) {
	UIButton* alloc_button = new UIButton(button);
	ui_lock();
	UICONTROL_HANDLE ret = next_handle;
	controls[next_handle++] = alloc_button;
	alloc_button->set_in_display(true);
	ui_unlock();
	return ret;
}

UICONTROL_HANDLE Display::add_control(const UICheckbox& chkbx) {
	UICheckbox* alloc_checkbox = new UICheckbox(chkbx);
	ui_lock();
	UICONTROL_HANDLE ret = next_handle;
	controls[next_handle++] = alloc_checkbox;
	alloc_checkbox->set_in_display(true);
	ui_unlock();
	return ret;
}

UICONTROL_HANDLE Display::add_control(const UIScrollbar& scrollbar) {
	UIScrollbar* alloc_scrollbar = new UIScrollbar(scrollbar);
	ui_lock();
	UICONTROL_HANDLE ret = next_handle;
	controls[next_handle++] = alloc_scrollbar;
	alloc_scrollbar->set_in_display(true);
	ui_unlock();
	return ret;
}

UICONTROL_HANDLE Display::add_control(const UIScrollbarSet& scrollbars) {
	UIScrollbarSet* alloc_scrollbars = new UIScrollbarSet(scrollbars);
	ui_lock();
	UICONTROL_HANDLE ret = next_handle;
	controls[next_handle++] = alloc_scrollbars;
	alloc_scrollbars->set_in_display(true);
	ui_unlock();
	return ret;
}

UICONTROL_HANDLE Display::add_control(const UIButtonGroup& button_grp) {
	UIButtonGroup* alloc_bgrp = new UIButtonGroup(button_grp);
	ui_lock();
	UICONTROL_HANDLE ret = next_handle;
	controls[next_handle++] = alloc_bgrp;
	alloc_bgrp->set_in_display(true);
	ui_unlock();
	return ret;
}

UIControl* Display::control(UICONTROL_HANDLE handle) {
	if (handle >= next_handle)
		return nullptr;
	return controls[handle];
}

UIControl* Display::control_at_pos(const SPoint& p) {
	ui_lock();
	for (auto& c : controls) {
		auto ctrl = c.second;
		SCoord co;
		ctrl->display_coord(co);
		if (co.Contains(p, i_bitmap)) {
			ui_unlock();
			return (ctrl);
		}
	}
	ui_unlock();
	return nullptr;
}

void Display::activate_control(UICONTROL_HANDLE handle) {
	ui_lock();
	controls[handle]->activate();
	ui_unlock();
}

void Display::deactivate_control(UICONTROL_HANDLE handle) {
	ui_lock();
	controls[handle]->deactivate();
	ui_unlock();
}

bool Display::is_active(UICONTROL_HANDLE handle) {
	ui_lock();
	bool ret = controls[handle]->is_active();
	ui_unlock();
	return ret;
}

void Display::hide_control(UICONTROL_HANDLE handle) {
	ui_lock();
	controls[handle]->hide();
	ui_unlock();
}

void Display::unhide_control(UICONTROL_HANDLE handle) {
	ui_lock();
	controls[handle]->unhide();
	ui_unlock();
}

bool Display::is_hidden(UICONTROL_HANDLE handle) {
	ui_lock();
	bool ret = controls[handle]->is_hidden();
	ui_unlock();
	return ret;
}

void Display::remove_control(UICONTROL_HANDLE handle) {
	ui_lock();
	controls.erase(handle);
	ui_unlock();
}

void Display::ui_frame_draw(void) {
	// We can't have the map change under us.
	ui_lock();
	// We update mouse motion every frame for controls that request it, so hover focus can be tracked.
	ui_mouse_move();

	// Draw all non-hidden controls.
#if 0
	u32 y = 5;
#endif
	for (auto& c : controls) {
		auto ctrl = c.second;
#if 0
		SCoord co;
		u32 sz;
		Font f(&font_oregon);
		char cstr[64];
		ctrl->display_coord(co);
		sprintf(cstr, "Control handle %d at (%d, %d)-(%d, %d)",
			c.first, co.X1(bitmap()), co.Y1(bitmap()), co.X2(bitmap()), co.Y2(bitmap()));
		sz = f.font_size_for_height(24);
		SBitmap* ret = f.render_cstr(cstr, sz, false, nullptr);
		Font::blit(ret, bitmap(), 20, y, C_BLACK);
		y += ret->height();
		delete ret;
#endif
		if (ctrl->is_hidden())
			continue;
		ctrl->draw();
	}
	ui_unlock();
}

void Display::ui_mouse_move(void) {
	// (precondition!) Don't need to lock the UI mutex here: caller already has for us.
	SPoint mouse(mx, my);

	for (auto& c : controls) {
		auto ctrl = c.second;
		if (!ctrl->is_active())
			continue;
		if (ctrl->is_hidden())
			continue;
		SCoord co;
		ctrl->display_coord(co);
		if (co.Contains(mouse)) {
			SPoint mouse_adj(mx - co.X1(i_bitmap), my - co.Y1(i_bitmap));
			ctrl->on_mouse_motion(mouse_adj);
		}
	}
}

void Display::ui_mouse_click(bool isright, bool isdown) {
	SPoint mouse(mx, my);

	ui_lock();
	for (auto& c : controls) {
		auto ctrl = c.second;
		if (!ctrl->is_active())
			continue;
		if (ctrl->is_hidden())
			continue;
		SCoord co;
		ctrl->display_coord(co);
		if (co.Contains(mouse)) {
			SPoint mouse_adj(mx - co.X1(i_bitmap), my - co.Y1(i_bitmap));
			if (ui_focus != nullptr)
				ui_focus->lost_ui_focus();
			ctrl->give_ui_focus();
			ui_focus = ctrl;
			if (isright) {
				if (isdown)
					ctrl->on_right_click_down(mouse_adj);
				else
					ctrl->on_right_click_up(mouse_adj);
			} else {
				if (isdown)
					ctrl->on_left_click_down(mouse_adj);
				else
					ctrl->on_left_click_up(mouse_adj);
			}
		}
	}
	ui_unlock();
}

void Display::ui_keyboard_event(bool isup, SDL_Keysym* event) {
	int ascii_shifted, scancode;
	u32 flags;

	ascii_shifted = ascii_from_keysym_shifted(event);
	flags = event->mod;
	if (nonzero(flags & KMOD_CTRL))
		flags |= KMOD_CTRL;
	if (nonzero(flags & KMOD_ALT))
		flags |= KMOD_ALT;
	if (nonzero(flags & KMOD_SHIFT))
		flags |= KMOD_SHIFT;
	if (nonzero(flags & KMOD_GUI))
		flags |= KMOD_GUI;
	scancode = event->scancode;

	ui_lock();
	for (auto& c : controls) {
		auto ctrl = c.second;
		if (!ctrl->is_active())
			continue;
		if (ctrl->is_hidden())
			continue;
		if (!ctrl->needs_keypresses())
			continue;
		if (isup)
			ctrl->on_key_up(ascii_shifted, scancode, flags);
		else
			ctrl->on_key_down(ascii_shifted, scancode, flags);
	}
	ui_unlock();
}
 
static u32 __frame_global = 0;
u32 codehappy_frame(void) {
	return __frame_global;
}

#ifdef CODEHAPPY_NATIVE
static void bmp_rgb_to_bgr(SBitmap* bmp) {
	for (u32 y = 0; y < bmp->height(); ++y)
		for (u32 x = 0; x < bmp->width(); ++x) {
			RGBColor a, c = bmp->get_pixel(x, y);
			a = RGB_ALPHA(c);
			c = RGB_TO_BGR(c);
			ADD_ALPHA(c, a);
			bmp->put_pixel(x, y, c);
		}
}
#endif

/* Making this a static class method allows access to private data members w/o friendship. */	
void Display::internal_main_loop() {
	if (SDL_MUSTLOCK(__app_canvas->surface))
		SDL_LockSurface(__app_canvas->surface);

	if (__app_canvas->in_console) {
		__app_canvas->console.render(__app_canvas->i_bitmap);
	}
	__app_canvas->app_main_loop(__app_canvas, __app_canvas->app_data);

	SDL_Event event;
	int a;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_QUIT:
			time_to_die = true;
			break;

		case SDL_MOUSEMOTION:
			__app_canvas->mx = event.motion.x;
			__app_canvas->my = event.motion.y;
			break;

		case SDL_MOUSEBUTTONUP:
			if (event.button.button == SDL_BUTTON_LEFT) {
				__app_canvas->ml = false;
				__app_canvas->ui_mouse_click(false, false);
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				__app_canvas->mr = false;
				__app_canvas->ui_mouse_click(true, false);
			} else if (event.button.button == SDL_BUTTON_MIDDLE) {
				__app_canvas->mm = false;
			}
			break;

		case SDL_MOUSEBUTTONDOWN:
			if (event.button.button == SDL_BUTTON_LEFT) {
				__app_canvas->ml = true;
				__app_canvas->ui_mouse_click(false, true);
			} else if (event.button.button == SDL_BUTTON_RIGHT) {
				__app_canvas->mr = true;
				__app_canvas->ui_mouse_click(true, true);
			} else if (event.button.button == SDL_BUTTON_MIDDLE) {
				__app_canvas->mm = true;
			}
			break;

		case SDL_KEYDOWN:
			__keycode_down[ascii_from_keysym(&event.key.keysym)] = true;
			a = int(event.key.keysym.sym);
			if (a >= 256) {
				__keycode_down[a] = true;
			}
			kb_on_key_down(&event.key.keysym);
			__app_canvas->ui_keyboard_event(false, &event.key.keysym);
			if (__app_canvas->in_console) {
				__app_canvas->console.addch(ascii_from_keysym_shifted(&event.key.keysym));
			}
			break;

		case SDL_KEYUP:
			__keycode_down[ascii_from_keysym(&event.key.keysym)] = false;
			a = int(event.key.keysym.sym);
			if (a >= 256) {
				__keycode_down[a] = false;
			}
			__app_canvas->ui_keyboard_event(true, &event.key.keysym);
			break;
		}
	}

	__app_canvas->ui_frame_draw();

#ifdef CODEHAPPY_NATIVE
	/* SDL in the native build uses BGR order for their colors. */
	// TODO: for perf reasons this may be kinda dumb, can we set/toggle RGB channel ordering on SBitmap instead?
	// (It only 'maybe' is bad for perf, since blitting a bunch of default RGB-order bitmaps onto your BGR surface in draw
	// may be slower than pretending the canvas is RGB, doing the blits directly, then reversing order when rendering is
	// finished, as we're doing here, if draw is complex enough, and if draw isn't that complex, do we care?)
	bmp_rgb_to_bgr(__app_canvas->i_bitmap);
#endif  // CODEHAPPY_NATIVE

	if (SDL_MUSTLOCK(__app_canvas->surface))
		SDL_UnlockSurface(__app_canvas->surface);
	SDL_Flip(__app_canvas->surface);

#ifdef CODEHAPPY_NATIVE
	/* Convert BGR back to RGB. */
	bmp_rgb_to_bgr(__app_canvas->i_bitmap);
#endif  // CODEHAPPY_NATIVE

	++__frame_global;
}

void codehappy_stop(void) {
#ifdef CODEHAPPY_WASM
	emscripten_cancel_main_loop();
#endif
}

void codehappy_main(MainLoopCallback main_loop_fn, void* user_data, u32 width, u32 height, u32 fps) {
	__app_canvas = new Display(width, height, main_loop_fn, user_data);
#ifdef CODEHAPPY_NATIVE_SDL
	int save_delay_ms = 0;
	// Default target framerate is 40 fps.
	if (0 == fps)
		fps = 40;
	forever {
		auto loop_start = std::chrono::high_resolution_clock::now();
		__app_canvas->internal_main_loop();
		if (Display::time_to_die) {
			break;
		}
		std::chrono::milliseconds elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - loop_start);
		int remain_ms = (1000 / fps) - elapsed.count();
		remain_ms += save_delay_ms;
		save_delay_ms = 0;
		// SDL_Delay() doesn't have the resolution to reliably correctly sleep any period
		// this small, so save up the milliseconds for waiting later and just continue. 
		if (remain_ms < 16) {
			save_delay_ms = std::max(remain_ms, 0);
			continue;
		}
		SDL_Delay(remain_ms);
	}
#else   // CODEHAPPY_WASM
	emscripten_set_main_loop(Display::internal_main_loop, fps, 1);
#endif  // CODEHAPPY_NATIVE_SDL
}

void codehappy_window_title(const char* new_title) {
#ifdef CODEHAPPY_NATIVE
	SDL_WM_SetCaption(new_title, nullptr);
#else  // CODEHAPPY_WASM
	EM_ASM({
	  document.title = UTF8ToString($0);
	}, new_title);
#endif
}

#ifdef CODEHAPPY_WASM

/*** Functions to manage the WASM virtual filesystem, and to make URI requests. ***/

static void __persist_callback(emscripten_fetch_t* fetch) {
	emscripten_fetch_close(fetch);
}

void codehappy_persist_file(const char* fname, u8* data, u32 num_bytes) {
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "EM_IDB_STORE");
	attr.attributes = EMSCRIPTEN_FETCH_REPLACE | EMSCRIPTEN_FETCH_PERSIST_FILE;
	attr.requestData = (char *)data;
	attr.requestDataSize = num_bytes;
	attr.onsuccess = __persist_callback;
	attr.onerror = __persist_callback;
	emscripten_fetch(&attr, fname);
}

void codehappy_delete_file(const char* fname) {
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "EM_IDB_DELETE");
	attr.onsuccess = __persist_callback;
	attr.onerror = __persist_callback;
	emscripten_fetch(&attr, fname);
}

static RamFile * __fetch_to_ramfile(emscripten_fetch_t *fetch) {
	if (fetch->status == 200) {
		// 200 OK
		RamFile * rf = new RamFile;
		char* databuf;

		databuf = new char [fetch->numBytes + 1];
		memcpy(databuf, fetch->data, fetch->numBytes);
		databuf[fetch->numBytes] = '\000';
		rf->open_static(databuf, fetch->numBytes, RAMFILE_DEFAULT);

		emscripten_fetch_close(fetch);
		return rf;
	}

	emscripten_fetch_close(fetch);
	return nullptr;
}

RamFile* codehappy_URI_fetch(const char* URI) {
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);

	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;

	emscripten_fetch_t *fetch = emscripten_fetch(&attr, URI);

	return __fetch_to_ramfile(fetch);
}

struct FetchRequestInfo {
	FetchRequestInfo() { done = false; rf = nullptr; cbk_good = nullptr; cbk_bad = nullptr; URI[0] = 0; }
	bool done;
	AsyncHTTPCallback cbk_good;
	AsyncHTTPCallback cbk_bad;
	RamFile* rf;
	char URI[256];
};

static volatile bool __fd_wait = false;
static u32 __next_fetch_handle = 1;
static std::unordered_map<u32, FetchRequestInfo> __fetch_done;

static u32 __handle_from_URI(const char* URI) {
	while (__fd_wait) SDL_Delay(1);
	for (const auto& e : __fetch_done) {
		const auto& fri = e.second;
		u32 handle = e.first;
		if (!strncmp(fri.URI, URI, 255)) {
			return handle;
		}
	}
	return 0;
}

static void __async_fetch_success(emscripten_fetch_t *fetch) {
	u32 handle = __handle_from_URI(fetch->url);
	if (0 == handle) {
		emscripten_fetch_close(fetch);
		return;
	}

	RamFile* rf = __fetch_to_ramfile(fetch);

	if (__fetch_done[handle].cbk_good != nullptr) {
		__fetch_done[handle].URI[0] = '\000';
		__fetch_done[handle].cbk_good(rf);
	} else {
		__fetch_done[handle].rf = rf;
	}
	__fetch_done[handle].done = true;
}

static void __async_fetch_failure(emscripten_fetch_t *fetch) {
	u32 handle = __handle_from_URI(fetch->url);
	if (0 == handle) {
		emscripten_fetch_close(fetch);
		return;
	}
	if (__fetch_done[handle].cbk_bad != nullptr) {
		__fetch_done[handle].cbk_bad(nullptr);
	}
	__fetch_done[handle].URI[0] = '\000';
	__fetch_done[handle].done = true;
	emscripten_fetch_close(fetch);
}

void codehappy_URI_fetch_async(const char* URI, AsyncHTTPCallback on_success, AsyncHTTPCallback on_failure) {
	FetchRequestInfo fri;
	u32 handle = __next_fetch_handle++;

	fri.cbk_good = on_success;
	fri.cbk_bad = on_failure;
	strncpy(fri.URI, URI, 255);
	fri.URI[255] = '\000';
	__fd_wait = true;
	__fetch_done[handle] = fri;
	__fd_wait = false;

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = __async_fetch_success;
	attr.onerror = __async_fetch_failure;
	emscripten_fetch(&attr, URI);
}

u32 codehappy_URI_fetch_async(const char* URI) {
	FetchRequestInfo fri;
	u32 handle = __next_fetch_handle++;

	strncpy(fri.URI, URI, 255);
	fri.URI[255] = '\000';
	__fd_wait = true;
	__fetch_done[handle] = fri;
	__fd_wait = false;

	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = __async_fetch_success;
	attr.onerror = __async_fetch_failure;
	emscripten_fetch(&attr, URI);

	return handle;
}

bool codehappy_async_fetch_done(u32 handle) {
	return __fetch_done[handle].done;
}

RamFile* codehappy_async_fetch_data(u32 handle) {
	RamFile* rf;
	if (!__fetch_done[handle].done)
		return nullptr;

	rf = __fetch_done[handle].rf;
	__fetch_done[handle].URI[0] = '\000';
	__fetch_done[handle].rf = nullptr;
	return rf;
}

/*** Functions to permit user to select files on their own computer and send them to libcodehappy through JavaScript. ***/

struct FileDataJS {
	bool v;
	u8* data;
	u32 size;
	u32 handle;
};
static std::vector<FileDataJS> __jsfiles;
static std::mutex __jsfile_mtx;

extern "C" {
EMSCRIPTEN_KEEPALIVE int codehappy_file_from_js(u8 *buffer, u32 size) {
	// we can assume FIFO since the file dialog is modal/synchronous
	for (int e = __jsfiles.size() - 1; e >= 0; --e) {
		if (!__jsfiles[e].v) {
			__jsfiles[e].data = buffer;
			__jsfiles[e].size = size;
			__jsfiles[e].v = true;
			return 1;
		}
	}
	return 0;
}
} // extern "C"

static bool __has_handle(u32 h) {
	// preassumption: mutex is already locked.
	for (u32 e = 0; e < __jsfiles.size(); ++e)
		if (__jsfiles[e].handle == h)
			return true;
	return false;
}

u32 codehappy_file_selection_from_js(const char* filter) {
	ScopeMutex sm(__jsfile_mtx);
	FileDataJS fdjs;

	fdjs.data = nullptr;
	fdjs.size = 0;
	do {
		fdjs.handle = RandU32();
	} while (__has_handle(fdjs.handle));
	fdjs.v = false;
	__jsfiles.push_back(fdjs);

	/* Javascript to open the file selector. We keep hold of the mutex while
	   this is going on. */
	if (is_null(filter)) {
		EM_ASM(
		  var file_selector = document.createElement('input');
		  file_selector.setAttribute('type', 'file');
		  file_selector.setAttribute('onchange','open_file(event)');
		  file_selector.click();
		);
	} else {
		EM_ASM( {
		  var file_selector = document.createElement('input');
		  file_selector.setAttribute('type', 'file');
		  file_selector.setAttribute('onchange','open_file(event)');
		  file_selector.setAttribute('accept', UTF8ToString($0));
		  file_selector.click();
		}, filter);
	}

	return fdjs.handle;
}

bool codehappy_js_file_available(u32 handle) {
	ScopeMutex sm(__jsfile_mtx);
	for (u32 e = 0; e < __jsfiles.size(); ++e)
		if (__jsfiles[e].handle == handle)
			return __jsfiles[e].v;
	return false;
}

u8* codehappy_js_file(u32 handle, u32* bytes_out) {
	ScopeMutex sm(__jsfile_mtx);
	for (u32 e = 0; e < __jsfiles.size(); ++e)
		if (__jsfiles[e].handle == handle && __jsfiles[e].v) {
			if (!is_null(bytes_out))
				*bytes_out = __jsfiles[e].size;
			return __jsfiles[e].data;
		}
	return nullptr;
}

#endif  // CODEHAPPY_WASM

#endif  // CODEHAPPY_SDL

#if defined(CODEHAPPY_NATIVE) && defined(CODEHAPPY_SDL) && defined(CODEHAPPY_WINDOWS)
u32 libcodehappy_argc(const char* arg) {
	const char* w = arg;
	const char* we = w + strlen(w);
	// Start at 1, to account for argv[0] being the executable's name.
	u32 carg = 1;

	while (w < we) {
		if (isspace(*w)) {
			++w;
			continue;
		}
		// An argument enclosed in double quotes.
		if (*w == '\"') {
			++w;
			while (w < we && *w != '\"') {
				++w;
			}
			if (w < we)
				++w;
			++carg;
			continue;
		}
		// A regular argument.
		while (w < we && !isspace(*w))
			++w;
		++carg;
	}
	return carg;
}

static char __retbufargv[1024];
const char* libcodehappy_argv(const char* args, u32 i) {
	const char* w = args;
	const char* we = w + strlen(w);
	u32 carg = 0;

	// argv[0] 'should' be the executable name. We don't care.
	if (0 == i) {
		return nullptr;
	}
	--i;

	while (w < we) {
		const char* ws;
		if (isspace(*w)) {
			++w;
			continue;
		}
		// An argument enclosed in double quotes.
		if (*w == '\"') {
			++w;
			ws = w;
			while (w < we && *w != '\"') {
				++w;
			}
			strncpy(__retbufargv, ws, w - ws);
			__retbufargv[w - ws] = '\000';
			if (w < we)
				++w;
			++carg;
			if (carg > i)
				break;
			continue;
		}
		// A regular argument.
		ws = w;
		while (w < we && !isspace(*w))
			++w;
		strncpy(__retbufargv, ws, w - ws);
		__retbufargv[w - ws] = '\000';
		++carg;
		if (carg > i)
			break;
	}
	if (carg <= i)
		return nullptr;

	return __retbufargv;
}
#endif  // Native && SDL && Windows

#ifdef DEBUG
VerboseStream codehappy_cerr(true, std::cerr);
VerboseStream codehappy_cout(true, std::cout);
#else
// let codehappy_cerr be on by default, for now.
VerboseStream codehappy_cerr(true, std::cerr);
VerboseStream codehappy_cout(false, std::cout);
#endif

void codehappy_verbose(bool v) {
	if (v) {
		codehappy_cerr.verbose();
		codehappy_cout.verbose();
	} else {
		codehappy_cerr.quiet();
		codehappy_cout.quiet();
	}
}

bool is_codehappy_verbose() {
	// assert(codehappy_cerr.is_verbose() == codehappy_cout.is_verbose());
	return codehappy_cout.is_verbose();
}

/*** Include the source files to make an easy one-file build. ***/
#include "bits.cpp"
#include "unicode.cpp"
#include "scratchpad.cpp"
#include "external/miniz.h"
#include "rand.cpp"
#include "misc.cpp"
#include "files.cpp"
#include "colors.cpp"	
#include "space.cpp"
#include "palette.cpp"
#include "drawing.cpp"
#include "gif.cpp"
#include "quantize.cpp"
#include "ramfiles.cpp"
#include "wavrender.cpp"
#include "parser.cpp"
#include "input.cpp"
#include "kb.cpp"
#include "external/lzf_c.cpp"
#include "external/lzf_d.cpp"
#include "wget.cpp"
#include "calendar.cpp"
#include "extern.cpp"
#include "csv.cpp"
#include "pcx.cpp"
#include "rotate.cpp"
#include "roman.cpp"
#include "plurals.cpp"
#include "permute.cpp"
#include "patch.cpp"
// TODO: fixit.
//#undef STB_VORBIS_HEADER_ONLY
//#include "external/stb_vorbis.cpp"
//#define STB_VORBIS_HEADER_ONLY
#include "external/console-colors.cpp"
#include "external/uint128_t.cpp"
#include "external/uint256_t.cpp"
#include "external/bigint_con.cpp"
#include "external/bigint_fun.cpp"
#include "overflow.cpp"
#include "asm.cpp"
#include "sprintf.cpp"
#include "bitfile.cpp"
#include "entropy.cpp"
#define MINIMP3_IMPLEMENTATION
#include "external/minimp3_ext.h"
#include "strtable.cpp"
#include "embedsvg.cpp"
#include "ui.cpp"
#include "ref.cpp"
#include "quadtree.cpp"
#include "isprime.cpp"
#include "ansi.cpp"
#include "conio.cpp"
#include "currcode.cpp"
#include "fixedstr.cpp"
#include "chrisdraw.cpp"
#include "stopwatch.cpp"
#include "external/libtom.cpp"
#include "external/vcodec.cpp"
#include "external/genann.c"
#include "external/kann.c"
#include "external/kautodiff.c"
#ifdef INCLUDE_FILE_DIALOGS
// This is OK, but adds dependencies to ComDlg32 (-lcomdlg32) on Windows. The header file
// isn't included unless INCLUDE_FILE_DIALOGS is #defined.
#include "external/tinyfiledialogs.cpp"
#endif
#include "prettyprint.cpp"
#define TML_IMPLEMENTATION
#define TSF_IMPLEMENTATION
#include "external/tml.h"
#include "external/tsf.h"
#define FFT_IMPLEMENTATION
#include "external/fft.h"
#include "external/isaac.cpp"
#include "external/isaac64.cpp"
#include "rng_os.cpp"
#include "imgnnet.cpp"
#ifdef INCLUDE_EMBED
#include "embed.cpp"
#endif
#include "pifunc.cpp"
#include "genetic.cpp"
#include "invfn.cpp"
#include "python_rpc.cpp"
#include "argparse.cpp"
#include "stats.cpp"
#include "elo.cpp"
#include "gpt-neox.cpp"
#include "split.cpp"
#include "textdataset.cpp"
#include "lmembed.cpp"
#include "llama.cpp"
#include "external/stable-diffusion/stable-diffusion.cpp"
#include "external/stable-diffusion/util.cpp"
#include "external/stable-diffusion/model.cpp"
#include "external/stable-diffusion/zip.c"
#include "ldm.cpp"
#include "external/bert.cpp"
#include "bert.cpp"

// SQLite is compiled separately (it's C99.)

/* end libcodehappy.cpp */
