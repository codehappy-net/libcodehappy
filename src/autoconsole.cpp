/***

	autoconsole.cpp
	
	Easy-to-use way to take text input in a libcodehappy GUI application, without having
	to create UI controls. AutoConsoles are objects you can create in your main loop (have
	them hang around as static variables, as user data passed into the function, or as
	objects you create and store a pointer/reference to as needed.) These can function
	essentially as text boxes/form inputs; they collect the keyboard input (and mouse
	input, if needed) from the display.

	AutoConsoles can get focus when a key is pressed, or can be given focus on a mouse
	click in their region. You can set where on the display they draw, and whether they
	are active/draw at all. 

	Copyright (c) 2022 C. M. Street.

***/
#ifdef CODEHAPPY_SDL

// TODO: this and the chyrons are the biggest UI asks for the library right now.

AutoConsole::AutoConsole(Display* d_, Font* f_, bool isfocus, bool isdraw, bool isactive, int draw_on, int focus_on) {
	d = d_;
	f = f_;
	kl = new KeyLast(d);
	sp = new Scratchpad(32);
	d->bitmap()->rect_bitmap(&co);
	active = isactive;
	focus = isfocus;
	drawon = isdraw;
	cursor = false;
	transparent = false;
	draw_key = draw_on;
	focus_key = focus_on;
	enterok = false;
	ld_on_enter = true;
	finished = false;
}
			
AutoConsole::~AutoConsole() {
	if (kl != nullptr)
		delete kl;
	if (sp != nullptr)
		delete sp;
	kl = nullptr;
	sp = nullptr;
	d = nullptr;
	f = nullptr;
}

void AutoConsole::update() {
	int addkey = -1;
	// did we just gain focus?
	if (!focus && -1 == focus_key) {
		for (int e = 1; e < SKEY_TRACK_MAX; ++e) {
			if (kl->now_down(e)) {
				focus = true;
				addkey = e;
				break;
			}
		}
	}
	if (!focus && kl->now_down(focus_key)) {
		focus = true;
	}
	// update the keyboard buffer
	if (!is_null(kl))
		kl->save(d);
	// if we don't have focus or a keypress, return
	if (!focus) {
		return;
	}
	if (addkey <= 0) {
		return;
	}
	// handle the key press
}

bool AutoConsole::first() const {
	if (is_null(kl))
		return true;
	return kl->first();
}

void AutoConsole::draw(bool do_update) {
	if (do_update)
		update();
	if (!drawon)
		return;
	if (!transparent)
		d->bitmap()->rect_fill(co, C_BLACK);
}
	
const char* AutoConsole::buf() const {
	if (!is_null(sp))
		return (const char*)sp->buffer();
	return nullptr;
}

bool AutoConsole::str_finished() const {
	if (is_null(kl))
		return false;
	return finished;
}

void AutoConsole::empty() {
	finished = false;
	if (!is_null(sp))
		sp->clear();
}

#endif  // CODEHAPPY_SDL
/* end autoconsole.cpp */
