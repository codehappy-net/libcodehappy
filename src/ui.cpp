/***

	ui.cpp

	UI controls for libcodehappy.

	Copyright (c) 2022 Chris Street.

***/
#ifdef CODEHAPPY_SDL

bool UIControl::location(SPoint& loc_out) {
	if (hidden)
		return false;
	SCoord dc;
	display_coord(dc);
	loc_out = dc.p1;
	return true;
}

void UIControl::set_location(const SPoint& new_loc) {
	if (is_null(subbmp))
		return;
	SPoint oloc;
	location(oloc);
	SPoint adj = new_loc - oloc;
	subbmp->subbmpdata()->co += adj;
}

SBitmap* UIControl::render_caption(const std::string& caption, u32 desired_height) {
	SBitmap* ret = nullptr;
	u32 sz;
	if (nullptr == font) {
		// Render with the bitfont, in the format expected by Font::blit.
		u32 sx, sy;
		sz = desired_height >> 3;
		sx = (9 * caption.length() * sz);
		sy = (8 * sz);
		ret = new SBitmap(sx, sy, BITMAP_GRAYSCALE);
		ret->clear(C_BLACK);
		ret->putstr_bitfont(caption.c_str(), 0, 0, sz, C_WHITE);
	} else {
		Font font_o(font);
		sz = font_o.font_size_for_height(desired_height);
		ret = font_o.render_cstr(caption.c_str(), sz, false, nullptr);
	}

	return ret;
}

void UIControl::display_coord(SCoord& coord_out) {
	SCoord reg;
	if (nullptr == subbmp) {
		coord_out = SCoord(0, 0, 0, 0);
		return;
	}
	subbmp->rect_bitmap(&reg);
	subbmp->top_level(reg, &coord_out);
}

void UIControl::on_mouse_motion(const SPoint& p) {
	frame_last_focus = codehappy_frame();
}

bool UIControl::has_focus(void) {
	return codehappy_frame() <= frame_last_focus + 1;
}

void UIControl::draw() {
}

// Button centered at pos; when pressed, the function is called with one argument (bool: whether the button's down.)
UIButton::UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SPoint& pos, std::function<void(bool)>& callback) {
	style = BUTTON_DEFAULT;
	isdown = false;
	cap = caption;
	state = nullptr;
	push_call = callback;
	rerender = false;
	centered = true;
	center = pos;
	typ = typ_;
	UIControl::set_font(ft);
	render_buttons(display);
	// co should be set now, so create the sub-bitmap.
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

// Button contained within the region reg; when pressed, the function is called with one argument (bool: whether the button's down.)
UIButton::UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SCoord& reg, std::function<void(bool)>& callback) {
	style = BUTTON_DEFAULT;
	isdown = false;
	co = reg;
	cap = caption;
	state = nullptr;
	push_call = callback;
	rerender = false;
	centered = false;
	typ = typ_;
	UIControl::set_font(ft);
	render_buttons(display);
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

// When pressed, the bool pointer is updated to the current state.
UIButton::UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SPoint& pos, bool* state_out) {
	style = BUTTON_DEFAULT;
	isdown = false;
	cap = caption;
	state = state_out;
	rerender = false;
	centered = true;
	center = pos;
	typ = typ_;
	UIControl::set_font(ft);
	render_buttons(display);
	// co should be set now, so create the sub-bitmap.
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

UIButton::UIButton(SBitmap* display, ttfont* ft, const std::string& caption, enum ButtonType typ_, const SCoord& reg, bool* state_out) {
	style = BUTTON_DEFAULT;
	isdown = false;
	co = reg;
	cap = caption;
	state = state_out;
	rerender = false;
	centered = false;
	typ = typ_;
	UIControl::set_font(ft);
	render_buttons(display);
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

// Button contained within a region with a provided user draw function. Arguments to the draw callback are the
// SBitmap to draw on, and a boolean indicating the button's state (is down).
UIButton::UIButton(SBitmap* display, enum ButtonType typ_, const SCoord& reg, std::function<void(bool)>& push_callback, std::function<void(SBitmap*, bool)>& draw_callback) {
	style = BUTTON_USERDRAW;
	isdown = false;
	state = nullptr;
	push_call = push_callback;
	rerender = false;
	centered = false;
	co = reg;
	typ = typ_;
	draw_call = draw_callback;
	render_buttons(display);	// does the right thing
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

UIButton::UIButton(SBitmap* display, enum ButtonType typ_, const SCoord& reg, bool* state_out, std::function<void(SBitmap*, bool)>& draw_callback) {
	style = BUTTON_USERDRAW;
	isdown = false;
	state = state_out;
	rerender = false;
	centered = false;
	co = reg;
	typ = typ_;
	draw_call = draw_callback;
	render_buttons(display);
	set_bitmap(display->subbitmap(co));
	// Free the rendered buttons -- we'll re-render on first draw. This allows
	// us to copy the object around w/o worrying about wild pointers.
	if (render_up != nullptr) {
		delete render_up;
		delete render_down;
	}
	render_up = nullptr;
	render_down = nullptr;
	rerender = true;
}

// Copy constructor.
UIButton::UIButton(const UIButton& button) {
	*this = button;
}

UIButton::~UIButton() {
	/***
		Since we only allocate render_up/render_down and keep them around on draw,
		it's OK to free them here.
	***/
	if (!is_null(render_up))
		delete render_up;
	if (!is_null(render_down))
		delete render_down;
	render_up = nullptr;
	render_down = nullptr;
}

void UIButton::draw(void) {
	if (BUTTON_USERDRAW == style) {
		draw_call(bitmap(), isdown);
		return;
	}
	if (rerender) {
		// We're dirty and have to make clean.
		if (!is_null(render_down))
			delete render_down;
		if (!is_null(render_up))
			delete render_up;
		render_buttons(nullptr);
		rerender = false;
	}
	if (isdown && render_down != nullptr)
		render_down->blit_blend(bitmap(), 0, 0);
	else if (render_up != nullptr)
		render_up->blit_blend(bitmap(), 0, 0);
}

void UIButton::set_font(ttfont* font_ptr) {
	// We may have to re-render the buttons.
	rerender = true;
	UIControl::set_font(font_ptr);
}

void UIButton::set_height(u32 nh) {
	SBitmap* d = bitmap()->top_level();
	co.min_height(nh, d);
	rerender = true;
}

void UIButton::render_buttons(SBitmap* display) {
	SBitmap* d;
	if (is_null(display))
		d = bitmap()->top_level();
	else
		d = display;
	render_down = nullptr;
	render_up = nullptr;
	if (BUTTON_USERDRAW == style) {
		// don't care about rendered buttons...
		return;
	}
	u32 height, width;
	if (centered) {
		height = 24;
	} else {
		height = co.Y2(d) - co.Y1(d) - 7;
		if (height < 8)
			height = 8;
	}
	SBitmap* capbmp = render_caption(cap, height);
	if (centered) {
		height = capbmp->height() + 16;
		width = capbmp->width() + 16;
		if (0 == (height & 1))
			++height;
		if (0 == (width & 1))
			++width;
		co = SCoord(center.X(d) - width / 2, center.Y(d) - height / 2,
			center.X(d) + width / 2, center.Y(d) + height / 2);
	} else {
		height = co.Y2(d) - co.Y1(d) + 1;
		width = co.X2(d) - co.X1(d) + 1;
	}

	render_up = new SBitmap(width, height);
	render_up->clear(C_GRAY);
	Font::blit(capbmp, render_up, ((int)render_up->width() - (int)capbmp->width()) / 2, ((int)render_up->height() - (int)capbmp->height()) / 2, C_BLACK);
	render_up->rect(SPoint(1, 1), SPoint(width - 2, height - 2), C_BLACK);
	render_up->alpha_opaque();

	render_down = new SBitmap(width, height);
	render_down->clear(C_GRAY);
	Font::blit(capbmp, render_down, ((int)render_down->width() - (int)capbmp->width()) / 2, ((int)render_down->height() - (int)capbmp->height()) / 2, C_BRIGHT_GREEN);
	render_down->rect(SPoint(1, 1), SPoint(width - 2, height - 2), C_BLACK);
	render_down->alpha_opaque();

	delete capbmp;
}

void UIButton::on_left_click_down(const SPoint& p) {
	if (typ == BUTTON_LOCKS) {
		// Lock button press.
		isdown = !isdown;
	} else {
		// Spring button press.
		isdown = true;
	}
	if (state != nullptr) {
		*state = isdown;
	} else {
		push_call(isdown);
	}
	// if there's a user-registered callback at the superclass level, call that.
	UIControl::on_left_click_down(p);
}

void UIButton::on_left_click_up(const SPoint& p) {
	if (typ == BUTTON_LOCKS) {
		// Lock button release; a no-op.
	} else {
		// Spring button release.
		isdown = false;
		if (state != nullptr) {
			*state = isdown;
		}
	}
}

// Checkbox centered at pos. Size is determined by the height of the caption. Call the passed-in function or set
// the boolean value when the checkbox state changes.
UICheckbox::UICheckbox(SBitmap* display, const SPoint& pos, ttfont* ft, u32 desired_height, const std::string& caption, std::function<void(bool)>& callback) {
	style = CHECKBOX_DEFAULT;
	centered = true;
	center = pos;
	cap = caption;
	is_checked = false;
	state_out = nullptr;
	push_call = callback;
	UIControl::set_font(ft);
	rerender = false;
	render_cap = nullptr;

	render_caption(display, desired_height);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

UICheckbox::UICheckbox(SBitmap* display, const SPoint& pos, ttfont* ft, u32 desired_height, const std::string& caption, bool* checkbox_state) {
	style = CHECKBOX_DEFAULT;
	centered = true;
	center = pos;
	cap = caption;
	is_checked = false;
	state_out = checkbox_state;
	UIControl::set_font(ft);
	rerender = false;
	render_cap = nullptr;

	render_caption(display, desired_height);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

// Checkbox; size (of the box and the caption) is determined by the passed-in region.
UICheckbox::UICheckbox(SBitmap* display, const SCoord& region, ttfont* ft, const std::string& caption, std::function<void(bool)>& callback) {
	style = CHECKBOX_DEFAULT;
	draw_region = region;
	co = SCoord(region.X1(display), region.Y1(display), region.X1(display) + region.dy(display), region.Y2(display));
	centered = false;
	cap = caption;
	is_checked = false;
	state_out = nullptr;
	push_call = callback;
	UIControl::set_font(ft);
	rerender = false;
	render_cap = nullptr;

	render_caption(display, 0);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

UICheckbox::UICheckbox(SBitmap* display, const SCoord& region, ttfont* ft, const std::string& caption, bool* checkbox_state) {
	style = CHECKBOX_DEFAULT;
	draw_region = region;
	centered = false;
	co = SCoord(region.X1(display), region.Y1(display), region.X1(display) + region.dy(display), region.Y2(display));
	cap = caption;
	is_checked = false;
	state_out = checkbox_state;
	UIControl::set_font(ft);
	rerender = false;
	render_cap = nullptr;

	render_caption(display, 0);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

// Checkbox contained within a region with a provided user draw function. No caption. Arguments to the draw callback are the
// SBitmap to draw on, and a boolean indicating the checkbox's state (is checked).
UICheckbox::UICheckbox(SBitmap* display, const SCoord& region, std::function<void(bool)>& push_callback, std::function<void(SBitmap*, bool)>& draw_callback) {
	style = CHECKBOX_USERDRAW;
	draw_region = region;
	is_checked = false;
	state_out = nullptr;
	centered = false;
	push_call = push_callback;
	draw_call = draw_callback;
	rerender = false;
	render_cap = nullptr;

	render_caption(display, 0);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

UICheckbox::UICheckbox(SBitmap* display, const SCoord& region, bool* checkbox_state, std::function<void(SBitmap*, bool)>& draw_callback) {
	style = CHECKBOX_USERDRAW;
	draw_region = region;
	is_checked = false;
	state_out = checkbox_state;
	centered = false;
	draw_call = draw_callback;
	rerender = false;
	render_cap = nullptr;

	render_caption(display, 0);
	set_bitmap(display->subbitmap(draw_region));
	if (render_cap)	delete render_cap;
	render_cap = nullptr;
	rerender = true;
}

// Copy constructor.
UICheckbox::UICheckbox(const UICheckbox& chkbox) {
	*this = chkbox;
}

UICheckbox::~UICheckbox() {
	if (render_cap != nullptr)
		delete render_cap;
	render_cap = nullptr;
}

void UICheckbox::draw(void) {
	if (CHECKBOX_USERDRAW == style) {
		draw_call(bitmap(), is_checked);
		return;
	}
	if (rerender) {
		// We're dirty and have to make clean.
		if (!is_null(render_cap))
			delete render_cap;
		render_caption(nullptr, 0);
		rerender = false;
	}

	// Default checkbox render.
	int box_dx = co.dx(bitmap()->top_level());
	bitmap()->rect_fill(0, 0, box_dx, box_dx, C_WHITE);
	bitmap()->rect(0, 0, box_dx, box_dx, C_BLACK);
	bitmap()->rect(1, 1, box_dx - 1, box_dx - 1, C_BLACK);
	if (is_checked) {
		bitmap()->line(0, 0, box_dx, box_dx, C_BLACK);
		bitmap()->line(0, box_dx, box_dx, 0, C_BLACK);
	}
	// Blit in the caption.
	Font::blit(render_cap, bitmap(), SCoord(box_dx + 4, 0, bitmap()->width() - 1, bitmap()->height() - 1), C_BLACK, CENTERED_VERT | ALIGN_LEFT);
}

void UICheckbox::set_font(ttfont* font_ptr) {
	rerender = true;
	UIControl::set_font(font_ptr);
}

void UICheckbox::on_left_click_down(const SPoint& p) {
	is_checked = !is_checked;
	if (state_out != nullptr) {
		*state_out = is_checked;
	} else {
		push_call(is_checked);
	}
	// if there's a user-registered callback at the superclass level, call that.
	UIControl::on_left_click_down(p);
}

void UICheckbox::render_caption(SBitmap* display, u32 desired_height) {
	SBitmap* d;
	render_cap = nullptr;
	if (CHECKBOX_USERDRAW == style) {
		return;
	}
	if (is_null(display))
		d = bitmap()->top_level();
	else
		d = display;
	if (centered && desired_height > 0) {
		// Populate co from center.
		co = SCoord(center.X(d) - desired_height / 2, center.Y(d) - desired_height / 2,
				center.X(d) + desired_height / 2, center.Y(d) + desired_height / 2);
		// Set draw region from co.
		draw_region = co;
	} else {
		desired_height = co.Y2(d) - co.Y1(d);
	}
	render_cap = UIControl::render_caption(cap, desired_height);

	// We may need to extend the draw region.
	draw_region.min_height(render_cap->height(), d);
	draw_region.min_width(render_cap->width() + co.dx(d) + 8, d);
}

UIScrollbar::UIScrollbar(SBitmap* display, enum ScrollbarStyle sty, u32 side_flag, int top_value, int bottom_value, u32 size) {
	SCoord full_reg;

	val[0] = top_value;
	val[1] = bottom_value;

	switch (side_flag) {
	case SIDE_LEFT:
		vertical = true;
		full_reg = SCoord(SPoint(0, 0), SPoint(size, display->height() - 1));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(0, full_reg.height() - size - 1), SPoint(size, full_reg.height() - 1));
		scroll_area = SCoord(0, buttons[0].Y2() + 1, full_reg.X2(), buttons[1].Y1() - 1);
		break;
	case SIDE_RIGHT:
		vertical = true;
		full_reg = SCoord(SPoint(display->width() - size - 1, 0), SPoint(display->width() - 1, display->height() - 1));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(0, full_reg.height() - size - 1), SPoint(size, full_reg.height() - 1));
		scroll_area = SCoord(0, buttons[0].Y2() + 1, full_reg.X2(), buttons[1].Y1() - 1);
		break;
	case SIDE_TOP:
		full_reg = SCoord(SPoint(0, 0), SPoint(display->width() - 1, size));
		vertical = false;
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(full_reg.width() - size - 1, 0), SPoint(full_reg.width() - 1, size));
		scroll_area = SCoord(buttons[0].X2() + 1, 0, buttons[1].X1() - 1, full_reg.dy());
		break;
	case SIDE_BOTTOM:
		vertical = false;
		full_reg = SCoord(SPoint(0, display->height() - size - 1), SPoint(display->width() - 1, display->height() - 1));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(full_reg.width() - size - 1, 0), SPoint(full_reg.width() - 1, size));
		scroll_area = SCoord(buttons[0].X2() + 1, 0, buttons[1].X1() - 1, full_reg.dy());
		break;
	default:
		return;
	}

	if (sty == SCROLLBAR_SLIDER) {
		scroll_area = full_reg - full_reg.p1;
	}
	c_loc = vertical ? scroll_area.Y1() : scroll_area.X1();
	style = sty;
	butstate[0] = false;
	butstate[1] = false;
	drag = false;
	click_step = (vertical ? scroll_area.height() : scroll_area.width()) / (std::abs(top_value - bottom_value) + 1);
	if (click_step < 1)
		click_step = 1;
	ensure_cursor_coord();
	set_bitmap(display->subbitmap(full_reg));
}

UIScrollbar::UIScrollbar(SBitmap* b, enum ScrollbarStyle sty, const SCoord& region, int top_value, int bottom_value, u32 side_flag, u32 size) {
	SCoord full_reg;

	val[0] = top_value;
	val[1] = bottom_value;

	switch (side_flag) {
	case SIDE_LEFT:
		vertical = true;
		full_reg = SCoord(SPoint(region.X1(b), region.Y1(b)), SPoint(region.X1(b) + size, region.Y2(b)));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(0, full_reg.height() - size - 1), SPoint(size, full_reg.height() - 1));
		scroll_area = SCoord(0, buttons[0].Y2() + 1, full_reg.dx(), buttons[1].Y1() - 1);
		break;
	case SIDE_RIGHT:
		vertical = true;
		full_reg = SCoord(SPoint(region.X2(b) - size, region.Y1(b)), SPoint(region.X2(b), region.Y2(b)));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(0, full_reg.height() - size - 1), SPoint(size, full_reg.height() - 1));
		scroll_area = SCoord(0, buttons[0].Y2() + 1, full_reg.dx(), buttons[1].Y1() - 1);
		break;
	case SIDE_TOP:
		full_reg = SCoord(SPoint(region.X1(b), region.Y1(b)), SPoint(region.X2(b), region.Y1(b) + size));
		vertical = false;
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(full_reg.width() - size - 1, 0), SPoint(full_reg.width() - 1, size));
		scroll_area = SCoord(buttons[0].X2() + 1, 0, buttons[1].X1() - 1, full_reg.Y2());
		break;
	case SIDE_BOTTOM:
		vertical = false;
		full_reg = SCoord(SPoint(region.X1(b), region.Y2(b) - size), SPoint(region.X2(b), region.Y2(b)));
		buttons[0] = SCoord(SPoint(0, 0), SPoint(size, size));
		buttons[1] = SCoord(SPoint(full_reg.width() - size - 1, 0), SPoint(full_reg.width() - 1, size));
		scroll_area = SCoord(buttons[0].X2() + 1, 0, buttons[1].X1() - 1, full_reg.dy());
		break;
	}

	if (sty == SCROLLBAR_SLIDER) {
		scroll_area = full_reg - full_reg.p1;
	}
	c_loc = vertical ? scroll_area.Y1() : scroll_area.X1();
	style = sty;
	butstate[0] = false;
	butstate[1] = false;
	drag = false;
	click_step = (vertical ? scroll_area.height() : scroll_area.width()) / (std::abs(top_value - bottom_value) + 1);
	if (click_step < 1)
		click_step = 1;
	ensure_cursor_coord();
	set_bitmap(b->subbitmap(full_reg));
}

UIScrollbar::UIScrollbar(const UIScrollbar& scrollbar) {
	*this = scrollbar;
}

UIScrollbar::~UIScrollbar() {
}

void UIScrollbar::draw(void) {
	if (style == SCROLLBAR_DEFAULT) {
		bitmap()->clear(C_GRAY);
		bitmap()->rect_fill(buttons[0], RGB_GRAY(160));
		bitmap()->rect_fill(buttons[1], RGB_GRAY(160));
		bitmap()->rect(0, 0, bitmap()->width() - 1, bitmap()->height() - 1, C_BLACK);
		bitmap()->rect(buttons[0], butstate[0] ? C_BRIGHT_GREEN : C_BLACK);
		bitmap()->rect(buttons[1], butstate[1] ? C_BRIGHT_GREEN : C_BLACK);

		// Draw the arrows on the buttons.
		if (vertical) {
			SPoint p1(50, POINT_PERCENT, 33, POINT_PERCENT),
				p2(33, POINT_PERCENT, 67, POINT_PERCENT),
				p3(67, POINT_PERCENT, 67, POINT_PERCENT),
				p4(50, POINT_PERCENT, 50, POINT_PERCENT),
				p5(50, POINT_PERCENT, 67, POINT_PERCENT),
				p6(33, POINT_PERCENT, 33, POINT_PERCENT),
				p7(67, POINT_PERCENT, 33, POINT_PERCENT);
			draw_arrow(p1, p2, p3, p4, buttons[0], butstate[0]);
			draw_arrow(p5, p6, p7, p4, buttons[1], butstate[1]);
		} else {
			SPoint p1(33, POINT_PERCENT, 50, POINT_PERCENT),
				p2(67, POINT_PERCENT, 33, POINT_PERCENT),
				p3(67, POINT_PERCENT, 67, POINT_PERCENT),
				p4(50, POINT_PERCENT, 50, POINT_PERCENT),
				p5(67, POINT_PERCENT, 50, POINT_PERCENT),
				p6(33, POINT_PERCENT, 33, POINT_PERCENT),
				p7(33, POINT_PERCENT, 67, POINT_PERCENT);
			draw_arrow(p1, p2, p3, p4, buttons[0], butstate[0]);
			draw_arrow(p5, p6, p7, p4, buttons[1], butstate[1]);
		}
	} else if (style == SCROLLBAR_SLIDER) {
		if (vertical)
			bitmap()->rect_fill(bitmap()->width() / 2 - 3, 0, bitmap()->width() / 2 + 3, bitmap()->height() - 1, C_BLACK);
		else
			bitmap()->rect_fill(0, bitmap()->height() / 2 - 3, bitmap()->width() - 1, bitmap()->height() / 2 + 3, C_BLACK);
	}

	// Draw the cursor.
	bitmap()->rect_fill(cursor_coord, RGB_GRAY(160));
	bitmap()->rect(cursor_coord, C_BLACK);
}

void UIScrollbar::ensure_cursor_coord(void) {
	int csz = vertical ? scroll_area.width() : scroll_area.height();	
	int x1, x2, y1, y2;
	if (vertical) {
		x1 = scroll_area.X1();
		x2 = scroll_area.X2();
		y1 = c_loc - csz / 2;
		if (y1 < scroll_area.Y1())
			y1 = scroll_area.Y1();
		y2 = y1 + csz;
		if (y2 > scroll_area.Y2()) {
			y2 = scroll_area.Y2();
			if (y2 - csz >= scroll_area.Y1())
				y1 = y2 - csz;
		}
	} else {
		y1 = scroll_area.Y1();
		y2 = scroll_area.Y2();
		x1 = c_loc - csz / 2;
		if (x1 < scroll_area.X1())
			x1 = scroll_area.X1();
		x2 = x1 + csz;
		if (x2 > scroll_area.X2()) {
			x2 = scroll_area.X2();
			if (x2 - csz >= scroll_area.X1())
				x1 = x2 - csz;
		}
	}
	cursor_coord = SCoord(x1, y1, x2, y2);
}

void UIScrollbar::draw_arrow(const SPoint& p1, const SPoint& p2, const SPoint& p3, const SPoint& p4, const SCoord& but, bool but_state) {
	SPoint flat[2];
	RGBColor c = but_state ? C_BRIGHT_GREEN : C_BLACK;

	flat[0] = p1;
	flat[1] = p2;
	flat[0].Flatten(but);
	flat[1].Flatten(but);
	bitmap()->line(flat[0], flat[1], c);
	flat[0] = p3;
	flat[0].Flatten(but);
	bitmap()->line(flat[1], flat[0], c);
	flat[1] = p1;
	flat[1].Flatten(but);
	bitmap()->line(flat[0], flat[1], c);
	flat[0] = p4;
	flat[0].Flatten(but);
	bitmap()->floodfill(flat[0], c);
}

void UIScrollbar::on_left_click_down(const SPoint& p) {
	// Are we clicking down on a button?
	if (style != SCROLLBAR_SLIDER && buttons[0].Contains(p)) {
		butstate[0] = true;
		step_cursor(0);
	}
	if (style != SCROLLBAR_SLIDER && buttons[1].Contains(p)) {
		butstate[1] = true;
		step_cursor(1);
	}
	// Are we clicking on the cursor?
	drag = cursor_coord.Contains(p);
	// if there's a user-registered callback at the superclass level, call that.
	UIControl::on_left_click_down(p);
}

void UIScrollbar::on_left_click_up(const SPoint& p) {
	butstate[0] = false;
	butstate[1] = false;
	drag = false;
}

void UIScrollbar::step_cursor(int button_index) {
	if (button_index == 0) {
		if (val[0] < val[1])
			c_loc -= click_step;
		else
			c_loc += click_step;
	} else if (button_index == 1) {
		if (val[0] < val[1])
			c_loc += click_step;
		else
			c_loc -= click_step;
	}
	ensure_cursor_coord();
}

void UIScrollbar::on_mouse_motion(const SPoint& p) {
	// If either button is pressed, scroll some more. It's another frame.
	if (butstate[0])
		step_cursor(0);
	if (butstate[1])
		step_cursor(1);
	// Check for a drag.
	if (drag && scroll_area.Contains(p)) {
		c_loc = vertical ? p.Y() : p.X();
		ensure_cursor_coord();
	}
}

int UIScrollbar::value_int(void) {
	int scroll_l = vertical ? scroll_area.dy() : scroll_area.dx();
	if (0 == scroll_l)
		return val[0];
	int cc = c_loc - (vertical ? scroll_area.Y1() : scroll_area.X1());
	cc = CLAMP(cc, 0, scroll_l);
	int v = val[0] + (cc * (val[1] - val[0])) / scroll_l;

	return v;
}

void UIScrollbar::set_value_int(int i) {
	int scroll_l = vertical ? scroll_area.dy() : scroll_area.dx();
	int cc = (scroll_l * (i - val[0])) / (val[1] - val[0]);
	cc = CLAMP(cc, 0, scroll_l);
	c_loc = cc + (vertical ? scroll_area.Y1() : scroll_area.X1());
}

UIScrollbarSet::UIScrollbarSet(SBitmap* display, u32 sides, int top_value_v, int bottom_value_v, int top_value_h, int bottom_value_h, u32 size) {
	UIScrollbar* vert, * horiz;

	/*** Create two scrollbars for the set, one vertical (index 0) and the other horizontal (child index 1). If the 
		display's height is smaller than the width, the vertical scrollbar gets the entire height, else the horizontal 
		scrollbar gets the entire width. ***/
	// TODO: implement the case where width() < height()
	switch (sides) {
	case (SIDE_LEFT | SIDE_BOTTOM):
//		if (display->height() < display->width()) {
			vert = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SIDE_LEFT, top_value_v, bottom_value_v, size);
			horiz = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(size + 1, 0, display->width() - 1, display->height() - 1), top_value_h, bottom_value_h, SIDE_BOTTOM, size);
//		} else {
//		}
		break;

	case (SIDE_RIGHT | SIDE_BOTTOM):
		vert = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SIDE_RIGHT, top_value_v, bottom_value_v, size);
		horiz = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(0, 0, display->width() - size - 2, display->height() - 1), top_value_h, bottom_value_h, SIDE_BOTTOM, size);
		break;

	case (SIDE_LEFT | SIDE_TOP):
		vert = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SIDE_LEFT, top_value_v, bottom_value_v, size);
		horiz = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(size + 1, 0, display->width() - 1, display->height() - 1), top_value_h, bottom_value_h, SIDE_TOP, size);
		break;

	case (SIDE_RIGHT | SIDE_TOP):
		vert = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SIDE_RIGHT, top_value_v, bottom_value_v, size);
		horiz = new UIScrollbar(display, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(0, 0, display->width() - size - 2, display->height() - 1), top_value_h, bottom_value_h, SIDE_TOP, size);
		break;

	default:
		return;
	}

	add_child(vert);
	add_child(horiz);
	SCoord full_bmp;
	display->rect_bitmap(&full_bmp);
	set_bitmap(display->subbitmap(full_bmp));
}

UIScrollbarSet::UIScrollbarSet(SBitmap* b, const SCoord& region, u32 sides, int top_value_v, int bottom_value_v, int top_value_h, int bottom_value_h, u32 size) {
	UIScrollbar* vert, * horiz;
	// TODO: implement the case where width() < height()
	switch (sides) {
	case (SIDE_LEFT | SIDE_BOTTOM):
		vert = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b), region.Y2(b)), top_value_v, bottom_value_v, SIDE_LEFT, size);
		horiz = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b) + size + 1, region.Y1(b), region.X2(b), region.Y2(b)), top_value_h, bottom_value_h, SIDE_BOTTOM, size);
		break;
	case (SIDE_RIGHT | SIDE_BOTTOM):
		vert = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b), region.Y2(b)), top_value_v, bottom_value_v, SIDE_RIGHT, size);
		horiz = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b) - size - 2, region.Y2(b)), top_value_h, bottom_value_h, SIDE_BOTTOM, size);
		break;
	case (SIDE_LEFT | SIDE_TOP):
		vert = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b), region.Y2(b)), top_value_v, bottom_value_v, SIDE_LEFT, size);
		horiz = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b) + size + 1, region.Y1(b), region.X2(b), region.Y2(b)), top_value_h, bottom_value_h, SIDE_TOP, size);
		break;
	case (SIDE_RIGHT | SIDE_TOP):
		vert = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b), region.Y2(b)), top_value_v, bottom_value_v, SIDE_RIGHT, size);
		horiz = new UIScrollbar(b, UIScrollbar::SCROLLBAR_DEFAULT, SCoord(region.X1(b), region.Y1(b), region.X2(b) - size - 2, region.Y2(b)), top_value_h, bottom_value_h, SIDE_TOP, size);
		break;
	default:
		return;
	}

	add_child(vert);
	add_child(horiz);
	set_bitmap(b->subbitmap(region));
}

UIScrollbarSet::UIScrollbarSet(const UIScrollbarSet& scrollbars) {
	*this = scrollbars;
}

UIScrollbarSet::~UIScrollbarSet() {
	if (is_in_display())
		destruct_children();
}

void UIScrollbarSet::draw(void) {
	UIScrollbar* child;
	iterate_begin();
	while (child = static_cast<UIScrollbar*>(next_child())) {
		child->draw();
	}
}

void UIScrollbarSet::on_left_click_down(const SPoint& p) {
	UIScrollbar* child;
	SPoint display_p;
	bitmap()->top_level(p, &display_p);
	iterate_begin();
	while (child = static_cast<UIScrollbar*>(next_child())) {
		SCoord reg;
		child->display_coord(reg);
		if (reg.Contains(display_p)) {
			SPoint p_adj(display_p.X() - reg.X1(), display_p.Y() - reg.Y1());
			child->on_left_click_down(p_adj);
		}
	}
	// if there's a user-registered callback at the superclass level, call that.
	UIControl::on_left_click_down(p);
}

void UIScrollbarSet::on_left_click_up(const SPoint& p) {
	UIScrollbar* child;
	SPoint display_p;
	bitmap()->top_level(p, &display_p);
	iterate_begin();
	while (child = static_cast<UIScrollbar*>(next_child())) {
		SCoord reg;
		child->display_coord(reg);
		if (reg.Contains(display_p)) {
			SPoint p_adj(display_p.X() - reg.X1(), display_p.Y() - reg.Y1());
			child->on_left_click_up(p_adj);
		}
	}
}

void UIScrollbarSet::on_mouse_motion(const SPoint& p) {
	UIScrollbar* child;
	SPoint display_p;
	bitmap()->top_level(p, &display_p);
	iterate_begin();
	while (child = static_cast<UIScrollbar*>(next_child())) {
		SCoord reg;
		child->display_coord(reg);
		if (reg.Contains(display_p)) {
			SPoint p_adj(display_p.X() - reg.X1(), display_p.Y() - reg.Y1());
			child->on_mouse_motion(p_adj);
		}
	}
}

int UIScrollbarSet::value_int_h(void) {
	UIScrollbar* sb = static_cast<UIScrollbar*>(child_idx(1));
	if (is_null(sb))
		return 0;
	return sb->value_int();
}

int UIScrollbarSet::value_int_v(void) {
	UIScrollbar* sb = static_cast<UIScrollbar*>(child_idx(0));
	if (is_null(sb))
		return 0;
	return sb->value_int();
}

void UIScrollbarSet::set_value_int_h(int i) {
	UIScrollbar* sb = static_cast<UIScrollbar*>(child_idx(1));
	if (is_null(sb))
		return;
	sb->set_value_int(i);
}

void UIScrollbarSet::set_value_int_v(int i) {
	UIScrollbar* sb = static_cast<UIScrollbar*>(child_idx(0));
	if (is_null(sb))
		return;
	sb->set_value_int(i);
}

UIButtonGroup::UIButtonGroup(SBitmap* display, ttfont* ft, u32 nbuttons, const std::string* captions, bool isvert, const SPoint& center_pos) {
	UIButton* nb;
	u32 e;
	u32 our_w, our_h;

	// First create the buttons.
	std::vector<u32> i;
	SCoord co;
	u32 sum = 0;
	u32 maxh = 0;
	for (e = 0; e < nbuttons; ++e) {
		nb = new UIButton(display, ft, captions[e], UIButton::BUTTON_LOCKS, center_pos, &state);
		nb->display_coord(co);
		if (isvert) {
			i.push_back(co.height());
			sum += co.height();
		} else {
			i.push_back(co.width());
			sum += co.width();
		}
		maxh = std::max(maxh, (u32)co.height());
		add_child(nb);
	}
	// Now adjust their locations.
	SPoint uleft, new_pos;
	if (isvert) {
		uleft = SPoint(center_pos.X(display) - co.width() / 2, center_pos.Y(display) - sum / 2);
		our_h = sum;
		our_w = co.width();
	} else {
		uleft = SPoint(center_pos.X(display) - sum / 2, center_pos.Y(display) - co.height() / 2);
		our_h = co.height();
		our_w = sum;
	}

	UIButton* child;
	new_pos = uleft;
	e = 0;
	iterate_begin();
	ibutt = 0;
	no_select = false;
	while (child = static_cast<UIButton*>(next_child())) {
		child->set_location(new_pos);
		child->set_value_bool(e == ibutt);
		child->set_height(maxh);
		if (isvert)
			new_pos.y += i[e];
		else
			new_pos.x += i[e];
		++e;
	}

	SCoord bound_rect(center_pos.X(display) - our_w / 2, center_pos.Y(display) - our_h / 2, 
			  center_pos.X(display) + our_w / 2, center_pos.Y(display) + our_h / 2);
	set_bitmap(display->subbitmap(bound_rect));
}

UIButtonGroup::~UIButtonGroup() {
	if (is_in_display())
		destruct_children();
}

UIButtonGroup::UIButtonGroup(const UIButtonGroup& ubg) {
	*this = ubg;
}

void UIButtonGroup::draw(void) {
	UIButton* child;
	u32 e = 0;
	iterate_begin();
	while (child = static_cast<UIButton*>(next_child())) {
		child->set_value_bool(e == ibutt);
		child->draw();
		++e;
	}
}

void UIButtonGroup::on_left_click_down(const SPoint& p) {
	UIButton* child;
	SPoint display_p;
	u32 e = 0;
	bitmap()->top_level(p, &display_p);
	iterate_begin();
	while (child = static_cast<UIButton*>(next_child())) {
		SCoord reg;
		child->display_coord(reg);
		if (reg.Contains(display_p)) {
			if (ibutt != e || no_select) {
				SPoint p_adj(display_p.X() - reg.X1(), display_p.Y() - reg.Y1());
				child->on_left_click_down(p_adj);
				if (ibutt == e)
					ibutt = -1;
				else
					ibutt = e;
			}
			break;
		}
		++e;
	}
	// if there's a user-registered callback at the superclass level, call that.
	UIControl::on_left_click_down(p);
}

void UIButtonGroup::on_left_click_up(const SPoint& p) {
	UIButton* child;
	SPoint display_p;
	bitmap()->top_level(p, &display_p);
	iterate_begin();
	while (child = static_cast<UIButton*>(next_child())) {
		SCoord reg;
		child->display_coord(reg);
		if (reg.Contains(display_p)) {
			SPoint p_adj(display_p.X() - reg.X1(), display_p.Y() - reg.Y1());
			child->on_left_click_up(p_adj);
		}
	}
}

int UIButtonGroup::value_int(void) {
	return ibutt;
}

void UIButtonGroup::set_value_int(int i) {
	if (i >= count_children())
		return;
	if (i < 0 && !no_select)
		return;
	if (i < 0)
		i = -1;
	ibutt = i;
}

void UIButtonGroup::permit_no_button_selected(bool t) {
	no_select = t;
	if (!no_select && ibutt < 0)
		ibutt = 0;
}

#endif  // CODEHAPPY_SDL
/* end ui.cpp */