/***

	testui.cpp

	Test application for UI controls.

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"

void main_loop(Display* display, void* user_data) {
	static bool init = false;
	static UICONTROL_HANDLE button[2], chkbox[2], scroll, slider;
	bool button_state[2], chkbox_state[2];
	display->bitmap()->clear(C_GREEN);
	if (!init) {
		std::string caption = "OK", caption2 = "LOCK", caption3 = "This is a checkbox", caption4 = "This one, too";
		UIButton but(display->bitmap(), &font_oregon, caption, UIButton::BUTTON_PRESSES, SPoint(400, 450), &button_state[0]);
		button[0] = display->add_control(but);
		button[1] = display->add_control(UIButton(display->bitmap(), nullptr, caption2, UIButton::BUTTON_LOCKS, SPoint(150, 450), &button_state[1]));
		chkbox[0] = display->add_control(UICheckbox(display->bitmap(), SPoint(150, 50), &font_oregon, 16, caption3, &chkbox_state[0]));
		chkbox[1] = display->add_control(UICheckbox(display->bitmap(), SCoord(150, 100, 200, 120), &font_oregon, caption4, &chkbox_state[1]));
		scroll = display->add_control(UIScrollbarSet(display->bitmap(), SIDE_LEFT | SIDE_BOTTOM, 0, 100, 200, 1000));
		slider = display->add_control(UIScrollbar(display->bitmap(), UIScrollbar::SCROLLBAR_SLIDER, SCoord(300, 200, 550, 240), 20, 480, SIDE_TOP));
		codehappy_window_title("libcodehappy UI demonstration");
		init = true;
	}
}

int app_main() {
	codehappy_main(main_loop, nullptr, 800, 512);
	return 0;
}
