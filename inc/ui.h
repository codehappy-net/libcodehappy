/***

	ui.h

	UI controls for libcodehappy.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __UI_H__
#define __UI_H__

#include <functional>

#ifdef CODEHAPPY_SDL

/*******

	UI control notes:

	Each UI control contains a subbitmap, representing its boundaries on the display.

	libcodehappy sends messages, such as mouse clicks, key presses, or redraw requests, to the controls
	affected by them.

	To communicate with the client app, a callback (std::function) may be provided, or the control's
	state may be set in a structure whose pointer is provided.

	User draw of controls is permitted.

*******/

#define	CALLBACK_LMCLICK	1	
#define CALLBACK_RMCLICK	2

// The base Control class.
class UIControl {
public:
	UIControl() {
		subbmp = nullptr;
		font = nullptr;
		active = true;
		hidden = false;
		in_display = false;
		may_destroy = false;
		ui_focus = false;
		callback_flags = 0UL;
	}

	// Abstract draw method -- every derived class needs one, and they each need to implement it.
	virtual void draw();

	// Return the coordinates of the control, on the top-level (display) bitmap. Can be
	// overriden, but usually doesn't have to be.
	virtual void display_coord(SCoord& coord_out);
	// The "location" of the control, by default, is the upper-left corner of the display SCoord.
	// Returns "false" if the control is not visible on the display. Can be overriden.
	virtual bool location(SPoint& loc_out);
	// Change the location of the control. Override for controls that need work besides adjusting
	// the display_coord.
	virtual void set_location(const SPoint& new_loc);

	// Set the default font to use for rendering captions, etc. Descendants that need to do
	// their own handling will override.
	// Note that, if nullptr, controls will render with the 6x8 KrisKwant bitfont.
	virtual void set_font(ttfont* font_ptr) { font = font_ptr; }

	// Called by libcodehappy when there's a left mouse click in the control's region.
	virtual void on_left_click_down(const SPoint& p) {
		if (callback_flags & CALLBACK_LMCLICK) {
			lmd_callback(this, p);
		}
	}
	virtual void on_left_click_up(const SPoint& p) {}
	// Called by libcodehappy when there's a right mouse click in the control's region.
	virtual void on_right_click_down(const SPoint& p) {
		if (callback_flags & CALLBACK_RMCLICK) {
			rmd_callback(this, p);
		}
	}
	virtual void on_right_click_up(const SPoint& p) {}
	// Called by libcodehappy on any frame the mouse moves (or hovers!) over the control's region.
	virtual void on_mouse_motion(const SPoint& p);

	// Returns whether or not the control wants to know about keypresses.
	virtual bool needs_keypresses(void) { return false; }
	virtual void on_key_down(int ascii, int scancode, u32 flags) {}
	virtual void on_key_up(int ascii, int scancode, u32 flags) {}

	// Activation and hidden state.
	virtual void activate(void)         { active = true; }
	virtual void deactivate(void)       { active = false; }
	virtual void hide(void)             { hidden = true; }
	virtual void unhide(void)           { hidden = false; }
	virtual bool is_active(void) const  { return active; }
        virtual bool is_hidden(void) const  { return hidden; }

	// Set or request whether the control is currently active.
	virtual bool is_active(void) { return active; }

	// Setting the subbitmap is generally done in the control constructor.
	void set_bitmap(SBitmap* bmp)   { subbmp = bmp; }
	SBitmap* bitmap(void) const     { return subbmp; }

	// Does the control currently have focus? Can be overridden to handle different definitions.
	virtual bool has_focus(void);

	// The display is giving this control focus, or taking focus away from it.
	void give_ui_focus(void) { ui_focus = true; }
	void lost_ui_focus(void) { ui_focus = false; }
	bool has_ui_focus() const { return ui_focus; }

	// Return a Font-rendered bitmap (with alpha channel) for the specified caption using the default font.
	SBitmap* render_caption(const std::string& caption, u32 desired_height);

	// Get the value of a control, of various types.
	virtual int value_int(void)   { return 0; }
	virtual bool value_bool(void) { return false; }
	virtual void value_str(std::string& str_out) { str_out.clear(); }

	// If setting the value of the control is permitted, override these.
	virtual void set_value_int(int i)   {}
	virtual void set_value_bool(bool b) {}
	virtual void set_value_str(const std::string& str) {}

	// Accessing child controls, if any. Controls should handle passing messages to their children.
	void iterate_begin() { cit = 0; }
	UIControl* next_child() {
		if (cit >= children.size())
			return nullptr;
		return children[cit++];
	}
	void add_child(UIControl* child) {
		NOT_NULL_OR_RETURN_VOID(child);
		children.push_back(child);
	}
	u32 count_children() {
		return children.size();
	}
	void destruct_children() {
		for (auto c : children)
			delete c;
		children.clear();
	}
	UIControl* child_idx(u32 idx) {
		if (idx >= children.size())
			return nullptr;
		return children[idx];
	}

	void callback_on_left_mouse_down(std::function<void(UIControl*, const SPoint&)>& callback) {
		lmd_callback = callback;
		callback_flags |= CALLBACK_LMCLICK;
	}

	void callback_on_right_mouse_down(std::function<void(UIControl*, const SPoint&)>& callback) {
		rmd_callback = callback;
		callback_flags |= CALLBACK_RMCLICK;
	}

	bool is_in_display() const { return in_display; }
	void set_in_display(bool b) { in_display = b; }
	bool ready_to_destroy() const { return may_destroy; }

private:
	SBitmap* subbmp;
	ttfont* font;
	u32 frame_last_focus;
	bool active;
	bool hidden;
	bool in_display;
	bool may_destroy;
	bool ui_focus;
	std::vector<UIControl*> children;
	u32 cit;
	u32 callback_flags;
	std::function<void(UIControl*, const SPoint&)> lmd_callback;
	std::function<void(UIControl*, const SPoint&)> rmd_callback;
};

// A push button. Can either simply spring back when released, or can lock into position.
class UIButton : public UIControl {
public:
	enum ButtonType { BUTTON_LOCKS = 0, BUTTON_PRESSES };
	enum ButtonStyle { BUTTON_DEFAULT = 0, BUTTON_USERDRAW };

	// Button centered at pos; when pressed, the function is called with one argument (bool: whether the button's down.)
	UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SPoint& pos, std::function<void(bool)>& callback);

	// Button contained within the region reg; when pressed, the function is called with one argument (bool: whether the button's down.)
	UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SCoord& reg, std::function<void(bool)>& callback);

	// When pressed, the bool pointer is updated to the current state.
	UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SPoint& pos, bool* state_out);
	UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SCoord& reg, bool* state_out);

	// Button contained within a region with a provided user draw function. Arguments to the draw callback are the
	// SBitmap to draw on, and a boolean indicating the button's state (is down).
	UIButton(SBitmap* display, enum ButtonType typ_, const SCoord& reg, std::function<void(bool)>& push_callback, std::function<void(SBitmap*, bool)>& draw_callback);
	UIButton(SBitmap* display, enum ButtonType typ_, const SCoord& reg, bool* state_out, std::function<void(SBitmap*, bool)>& draw_callback);

	// Copy constructor.
	UIButton(const UIButton& but);

	~UIButton();

	// Overrides.
	void draw(void) override;
	void set_font(ttfont* font_ptr) override;

	void on_left_click_down(const SPoint& p) override;
	void on_left_click_up(const SPoint& p) override;

	int value_int(void) override   { return isdown ? 1 : 0; }
	bool value_bool(void) override { return isdown; }
	void value_str(std::string& str_out) override { str_out = cap; }

	void set_value_int(int i) override   { isdown = (i != 0); }
	void set_value_bool(bool b) override { isdown = b; }

	void set_height(u32 nh);

private:
	// Render the up/down button.
	void render_buttons(SBitmap* display);
	// Our button style.
	ButtonStyle style;
	// Bounding region of the button.
	SCoord co;
	// Center point, if the button is centered.
	SPoint center;
	// Is the button down?
	bool isdown;
	// Caption of the button.
	std::string cap;
	// Pointer to return button state (or nullptr if not requested).
	bool* state;
	// Keep track of whether we're centered: if we need to recalculate button size (font or
	// caption changes, say) this is important to know.
	bool centered;
	// Callback to use if the button is pushed. The current down state will be passed along.
	// We only use this if state is nullptr.
	std::function<void(bool)> push_call;
	// Is this a lock button?
	ButtonType typ;
	// Callback to use for user draw. Only used if style is BUTTON_USERDRAW.
	std::function<void(SBitmap*, bool)> draw_call;
	// Do we currently need to re-render the control?
	bool rerender;
	// Pre-rendered bitmap of the button in "up" state.
	SBitmap* render_up;
	// Pre-rendered bitmap of the button in "down" state.
	SBitmap* render_down;
};


// A checkbox.
class UICheckbox : public UIControl {
public:
	enum CheckboxStyle { CHECKBOX_DEFAULT = 0, CHECKBOX_USERDRAW };

	// Checkbox centered at pos. Size is determined by the height of the caption. Call the passed-in function or set
	// the boolean value when the checkbox state changes.
	UICheckbox(SBitmap* display, const SPoint& pos, ttfont* ft, u32 desired_height, const std::string& caption, std::function<void(bool)>& callback);
	UICheckbox(SBitmap* display, const SPoint& pos, ttfont* ft, u32 desired_height, const std::string& caption, bool* checkbox_state);

	// Checkbox; size (of the box and the caption) is determined by the passed-in region.
	UICheckbox(SBitmap* display, const SCoord& region, ttfont* ft, const std::string& caption, std::function<void(bool)>& callback);
	UICheckbox(SBitmap* display, const SCoord& region, ttfont* ft, const std::string& caption, bool* checkbox_state);

	// Checkbox contained within a region with a provided user draw function. No caption. Arguments to the draw callback are the
	// SBitmap to draw on, and a boolean indicating the checkbox's state (is checked).
	UICheckbox(SBitmap* display, const SCoord& region, std::function<void(bool)>& push_callback, std::function<void(SBitmap*, bool)>& draw_callback);
	UICheckbox(SBitmap* display, const SCoord& region, bool* checkbox_state, std::function<void(SBitmap*, bool)>& draw_callback);

	// Copy constructor.
	UICheckbox(const UICheckbox& chkbox);

	~UICheckbox();

	// Overrides.
	void draw(void) override;
	void set_font(ttfont* font_ptr) override;

	void on_left_click_down(const SPoint& p) override;

	int value_int(void) override   { return is_checked ? 1 : 0; }
	bool value_bool(void) override { return is_checked; }
	void value_str(std::string& str_out) override { str_out = cap; }

	void set_value_int(int i) override   { is_checked = (i != 0); }
	void set_value_bool(bool b) override { is_checked = b; }
	void set_value_str(const std::string& str) override {
		cap = str;
		rerender = true;
	}

private:
	// Render the caption.
	void render_caption(SBitmap* display, u32 desired_height);
	// Our checkbox style.
	CheckboxStyle style;
	// Rectangle representing the checkbox.
	SCoord co;
	// User-passed draw region. For non-user draw regions, this may be extended to fit the caption.
	SCoord draw_region;
	// Center point, if the control is centered.
	SPoint center;
	// Is the checkbox checked?
	bool is_checked;
	// Caption for the checkbox.
	std::string cap;
	// Pointer to return checkbox state (or nullptr if not requested).
	bool* state_out;
	// Keep track of whether we're centered.
	bool centered;
	// Callback to use if the checkbox state changes.
	// We only use this if state_out is nullptr.
	std::function<void(bool)> push_call;
	// Callback to use for user draw. Only used if style is CHECKBOX_USERDRAW.
	std::function<void(SBitmap*, bool)> draw_call;
	// Do we currently need to re-render the control?
	bool rerender;
	// Pre-rendered caption.
	SBitmap* render_cap;
};

// A scrollbar (vertical or horizontal.) Can also function as a slider.
class UIScrollbar : public UIControl {
public:
	enum ScrollbarStyle { SCROLLBAR_DEFAULT, SCROLLBAR_SLIDER };

	// Creates a scrollbar along one side of the passed-in display. The scrollbar extends along that entire
	// side of the display, and its width/height is specified by size.
	// The scrollbar represents the values [top_value, bottom_value] interpolated linearly from top to bottom.
	UIScrollbar(SBitmap* display, enum ScrollbarStyle sty, u32 side, int top_value, int bottom_value, u32 size = 20);

	// Creates a scrollbar along one side of a region of a passed-in display. The scrollbar extends along that entire 
	// side of the region, and its width/height is specified by size.
	// The scrollbar represents the values [top_value, bottom_value] interpolated linearly from top to bottom.
	UIScrollbar(SBitmap* display, enum ScrollbarStyle sty, const SCoord& region, int top_value, int bottom_value, u32 side, u32 size = 20);

	// Copy constructor.
	UIScrollbar(const UIScrollbar& scrollbar);

	~UIScrollbar();

	// Overrides.
	void draw(void) override;
	void on_left_click_down(const SPoint& p) override;
	void on_left_click_up(const SPoint& p) override;
	void on_mouse_motion(const SPoint& p) override;
	int value_int(void) override;
	void set_value_int(int i) override;

private:
	// Draw helper
	void draw_arrow(const SPoint& p1, const SPoint& p2, const SPoint& p3, const SPoint& p4, const SCoord& but, bool but_state);
	// Calculate current cursor location.
	void ensure_cursor_coord(void);
	// Advance cursor location a step.
	void step_cursor(int button_idx);
	// Are we a vertical scrollbar?
	bool vertical;
	// Our style.
	ScrollbarStyle style;
	// Top and bottom values.
	int val[2];
	// Region representing the scroll area. (In the subbmp's coordinates.)
	SCoord scroll_area;
	// Regions representing the first (top/left) and second (bottom/right) buttons. (In the subbmp's coordinates.)
	// For slider style, this instead represents the region that can be dragged.
	SCoord buttons[2];
	// Booleans representing whether the buttons are depressed.
	bool butstate[2];
	// Cursor location (for scrollbox style.)
	SCoord cursor_coord;
	// Is the slider/cursor currently being dragged?
	bool drag;
	// Amount to increment/decrement c_lock with each click.
	int click_step;
	// Y (vertical) or X (horizonal) location of the cursor.
	int c_loc;
};

// A set of scrollbars, one vertical, one horizontal.
class UIScrollbarSet : public UIControl {
public:
	// Creates a scrollbar set along two sides of the passed-in display. The scrollbars extend along those entire
	// sides of the display, and its width/height is specified by size.
	UIScrollbarSet(SBitmap* display, u32 sides, int top_value_v, int bottom_value_v, int top_value_h, int bottom_value_h, u32 size = 20);

	// Creates a scrollbar along two sided of a region of a passed-in display. The scrollbars extend along those entire 
	// sides of the region, and the width/height is specified by size.
	// The scrollbar represents the values [top_value, bottom_value] interpolated linearly from top to bottom.
	UIScrollbarSet(SBitmap* display, const SCoord& region, u32 sides, int top_value_v, int bottom_value_v, int top_value_h, int bottom_value_h, u32 size = 20);

	// Copy constructor.
	UIScrollbarSet(const UIScrollbarSet& scrollbars);

	~UIScrollbarSet();

	// Overrides.
	void draw(void) override;
	void on_left_click_down(const SPoint& p) override;
	void on_left_click_up(const SPoint& p) override;
	void on_mouse_motion(const SPoint& p) override;
	int value_int_h(void);
	int value_int_v(void);
	void set_value_int_h(int i);
	void set_value_int_v(int i);
};

// A group of lock/radio buttons. Only one of each group may be pressed at a time.
class UIButtonGroup : public UIControl {
public:
	// Creates a button group centered at the specified point of the bitmap.
	UIButtonGroup(SBitmap* display, ttfont* ft, u32 nbuttons, const std::string* captions, bool isvert, const SPoint& center_pos);
	~UIButtonGroup();

	// Copy constructor.
	UIButtonGroup(const UIButtonGroup& ubg);

	// Overrides.
	void draw(void) override;
	void on_left_click_down(const SPoint& p) override;
	void on_left_click_up(const SPoint& p) override;
	int value_int(void) override;
	void set_value_int(int i) override;

	// Do we permit no button to be selected?
	void permit_no_button_selected(bool t);
	bool no_button_selected(void) const    { return no_select; }

private:
	// Which button is currently down.
	int ibutt;
	// Iff true, we allow all the buttons to be up. Otherwise one must be down.
	bool no_select;
	// State bool. This is just given to the child buttons know they aren't to make the std::function call.
	bool state;
};

#endif  // CODEHAPPY_SDL
#endif  // __UI_H__