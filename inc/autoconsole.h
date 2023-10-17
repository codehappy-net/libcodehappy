/***

	autoconsole.h
	
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
#ifndef _AUTOCONSOLE_H_
#define _AUTOCONSOLE_H_
#ifdef CODEHAPPY_SDL

class AutoConsole {
public:
	/* Constructor. The default arguments will give you a text box that will, on key press, draw and gain focus,
	   but does not draw otherwise. */
	AutoConsole(Display* d_, Font* f_ = nullptr, bool isfocus = false, bool isdraw = false, bool isactive = true,
			int draw_on = -1, int focus_on = -1);
			
	~AutoConsole();
	
	/* Update -- call this in each call of the containing function/main loop. Can be done by draw(). */
	void update();

	/* Performs the render onto the member Display object. Can auto-call update() before the draw. */
	void draw(bool do_update = true);
	
	/* Characters in the buffer */
	const char* buf() const;

	/* Has the user completed a string? (ENTER key to finish) */
	bool str_finished() const;

	/* Empty the buffer */
	void empty();
	
	/* First time through? */
	bool first() const;
	
	/* Hide the control, if visible */
	void hide()  { drawon = false; focus = false; }
	/* Show the control */
	void show()  { drawon = true; }
	/* Give focus to the control */
	void give_focus() { drawon = true; focus = true; }
	/* Hide or show the cursor */
	void cursor_hide() { cursor = false; }
	void cursor_show() { cursor = true; }
	/* Set the draw region. */
	void set_draw_region(const SCoord& co_) { co = co_; }
	/* Transparency */
	void set_transparent(bool t) { transparent = t; }
	bool is_transparent() const  { return transparent; }

private:
	Display* d;		// display
	KeyLast* kl;		// keyboard state
	Scratchpad* sp;	// string buffer
	Font* f;		// font for text render
	SCoord co;		// draw region
	bool active;		// are we watching input from the keyboard?
	bool focus;		// do we currently have focus?
	bool drawon;		// do we currently draw?
	bool cursor;		// do we show a cursor?
	bool transparent;	// do we draw on top of the render, or do we clear the draw area?
	int draw_key;		// keycode we turn draw on (0 = none, -1 = any key press; if 'any', the key press will be inserted into the buffer)
	int focus_key;		// keycode we gain focus on (0 = none, -1 = any key press; if 'any', the key press will be inserted into the buffer)
	bool enterok;		// whether the enter key can be entered into the string, or whether it causes the string to finish
	bool ld_on_enter;	// do we stop drawing on enter?
	bool finished;		// current "is a string finished and waiting?" state
	// TODO: allow multiline text boxes; basically, just support ENTER, draw properly, and add interface for getting individual lines
};

#endif  // CODEHAPPY_SDL
#endif  // _AUTOCONSOLE_H_
/*** end autoconsole.h ***/
