/***

	toy.cpp

	Native + SDL app that provides a user interface to ImgNNet functions.

	Copyright (c) 2022 Chris Street.

***/
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>
#include <thread>

#define MAX_WIDTH	900
#define MAX_HEIGHT	600

/*** App global data. ***/
struct AppData {
	SBitmap* volatile bmp;
	SBitmap* volatile erased;
	SBitmap* volatile painted;
	u32		fontsz;
	u32		nth;
	ImgNNet		nnet4;
	ImgNNet		nnet5;
	ImgNNet		nnet9;
	volatile bool	gobutton;
	volatile PredictWindow	pw;
	bool		pred_in_progress;
	UICONTROL_HANDLE tool_but;
	UICONTROL_HANDLE nnet_but;
	UICONTROL_HANDLE go_but;
	UICONTROL_HANDLE static_but;
	u32		rad;
	std::thread*	th;
	std::mutex	lockme;
	Stopwatch	sw;
	void lock()   { lockme.lock(); }
	void unlock() { lockme.unlock(); }
};

static AppData ad;

void prediction_thread(void) {
	ad.nnet4.set_max_threads(ad.nth);
	ad.nnet5.set_max_threads(ad.nth);
	ad.nnet9.set_max_threads(ad.nth);
	switch (ad.rad) {
	case 4: ad.painted = ad.nnet4.predict_from_missing(ad.bmp, ad.erased);
		break;
	case 5: ad.painted = ad.nnet5.predict_from_missing(ad.bmp, ad.erased);
		break;
	case 9: ad.painted = ad.nnet9.predict_from_missing(ad.bmp, ad.erased);
		break;
	}
}

void find_recent_checkpoint(const char* pfx, std::string& out) {
	DIR* di = opendir("nnets");
	dirent* entry;
	const size_t l = strlen(pfx);
	int maxcp = 0;

	while (entry = readdir(di)) {
		const char* w;
		if (strncmp(entry->d_name, pfx, l))
			continue;
		w = entry->d_name + strlen(entry->d_name) - 1;
		if (!isdigit(*w))
			continue;
		while (w > entry->d_name && isdigit(*w))
			--w;
		++w;
		if (atoi(w) > maxcp) {
			out = "nnets/";
			out += entry->d_name;
			maxcp = atoi(w);
		}
	}
	closedir(di);
	std::cout << "Found recent checkpoint '" << out << "'.\n";
}

void find_recent_checkpoints(std::string& fivek, std::string& chungus, std::string& doublek) {
	find_recent_checkpoint("fivek.rfn", fivek);
	find_recent_checkpoint("chungus.rfn", chungus);
	find_recent_checkpoint("doublek.rfn", doublek);
}

void hide_ui(Display* display) {
	UIButtonGroup* bgr = static_cast<UIButtonGroup*>(display->control(ad.tool_but));
	bgr->hide();
	bgr = static_cast<UIButtonGroup*>(display->control(ad.nnet_but));
	bgr->hide();
	UIButton* but = static_cast<UIButton*>(display->control(ad.go_but));
	but->hide();
	but = static_cast<UIButton*>(display->control(ad.static_but));
	but->hide();
}

void unhide_ui(Display* display) {
	UIButtonGroup* bgr = static_cast<UIButtonGroup*>(display->control(ad.tool_but));
	bgr->unhide();
	bgr = static_cast<UIButtonGroup*>(display->control(ad.nnet_but));
	bgr->unhide();
	UIButton* but = static_cast<UIButton*>(display->control(ad.go_but));
	but->set_value_bool(false);
	but->unhide();
	but = static_cast<UIButton*>(display->control(ad.static_but));
	but->unhide();
}

u32 idist(const SPoint& pt, int x, int y) {
	u32 d = 0;
	d += (pt.X() - x) * (pt.X() - x);
	d += (pt.Y() - y) * (pt.Y() - y);
	return isqrt(d);
}

void main_loop(Display* display, void* user_data) {
	Font font(&font_swansea_bold);
	SCoord rect;
	int mx = display->mouse_x(), my = display->mouse_y();
	bool lmb = display->mouse_l(), rmb = display->mouse_r();
	static SPoint m_last(0, 0);
	static bool lmb_last = false, rmb_last = false;
	static bool first = true;
	static bool stbut = false;
	static SBitmap* ecopy = nullptr;
	ad.bmp->rect_bitmap(&rect);

	if (first) {
		std::string captions[] = { "fivek", "chungus", "doublek", "Eraser", "Rects", "Circle", "Paint!", "Add Static" };
		codehappy_window_title("Image prediction neural net test application");
		ad.fontsz = font.font_size_for_height(36);
		UIButtonGroup buttons_nnet(display->bitmap(), &font_swansea, 3, captions, false, SPoint(150, POINT_PIXEL_REV, 20, POINT_PERCENT));
		UIButtonGroup buttons_tools(display->bitmap(), &font_swansea, 3, &captions[3], false, SPoint(150, POINT_PIXEL_REV, 35, POINT_PERCENT));
		UIButton gobutton(display->bitmap(), &font_swansea, captions[6], UIButton::BUTTON_PRESSES, SPoint(150, POINT_PIXEL_REV, 50, POINT_PERCENT), (bool*)&ad.gobutton);
		UIButton staticbutton(display->bitmap(), &font_swansea, captions[7], UIButton::BUTTON_PRESSES, SPoint(150, POINT_PIXEL_REV, 65, POINT_PERCENT), &stbut);
		ad.nnet_but = display->add_control(buttons_nnet);
		ad.tool_but = display->add_control(buttons_tools);
		ad.go_but = display->add_control(gobutton);
		ad.static_but = display->add_control(staticbutton);
		ad.pw.ace = -1.;
		ad.pw.pa = nullptr;
		ecopy = new SBitmap(ad.bmp->width(), ad.bmp->height());
		ecopy->clear();
		first = false;
	}

	if (ad.gobutton && !ad.pred_in_progress) {
		// Button pressed, kick off a prediction thread. 
		ad.pw.done = false;
		ad.pw.ret = nullptr;
		ad.pred_in_progress = true;
		ad.gobutton = false;
		ad.erased->blit(ecopy, SPoint(0, 0));
		hide_ui(display);
		ad.sw.start();
		ad.th = new std::thread(prediction_thread);
	}

	if (stbut) {
		stbut = false;
		for (int y = 0; y < ad.bmp->height(); ++y)
			for (int x = 0; x < ad.bmp->width(); ++x) {
				if (RandU32Range(0, 99) == 0) {
					ad.erased->put_pixel(x, y, C_WHITE);
				}
			}
	}

	if (ad.pred_in_progress && ad.pw.done) {
		// Prediction finished, we can show the controls again.
		if (ad.th->joinable())
			ad.th->join();
		delete ad.th;
		ad.th = nullptr;
		delete ad.bmp;
		ad.bmp = ad.painted;
		ad.painted = nullptr;
		ad.erased->clear();
		ecopy->blit(ad.erased, SPoint(0, 0));
		ad.pred_in_progress = false;
		unhide_ui(display);
	}

	if ((lmb && !lmb_last) || (rmb && !rmb_last)) {
		// Keep track of the mouse position the last time a mouse button went down.
		m_last = SPoint(mx, my);
	}

	int innet, ibutt;
	UIButtonGroup* control;
	control = static_cast<UIButtonGroup*>(display->control(ad.nnet_but));
	innet = control->value_int();
	switch (innet) {
	case 0:	ad.rad = 4;
		break;
	case 1: ad.rad = 5;
		break;
	case 2:	ad.rad = 9;
		break;
	}
	control = static_cast<UIButtonGroup*>(display->control(ad.tool_but));
	ibutt = control->value_int();

	if (0 == ibutt) {
		if (lmb) {
			// Erase tool with left mouse button pressed.
			for (int y = my - 3; y <= my + 3; ++y)
				for (int x = mx - 3; x <= mx + 3; ++x) {
					if (!pixel_ok(ad.erased, x, y))
						continue;
					int ds = (y - my) * (y - my) + (x - mx) * (x - mx);
					if (ds <= 9) {
						ad.erased->put_pixel(x, y, C_WHITE);
					}
				}
		} else if (rmb) {
			// Erase tool with right mouse button pressed (unerase).
			for (int y = my - 3; y <= my + 3; ++y)
				for (int x = mx - 3; x <= mx + 3; ++x) {
					if (!pixel_ok(ad.erased, x, y))
						continue;
					int ds = (y - my) * (y - my) + (x - mx) * (x - mx);
					if (ds <= 9) {
						ad.erased->put_pixel(x, y, C_BLACK);
					}
				}
		}
	} else if (1 == ibutt) {
		if (lmb_last && !lmb) {
			// Rect tool with left mouse button released.
			ad.erased->rect_fill(m_last, SPoint(mx, my), C_WHITE);
		} else if (rmb_last && !rmb) {
			// Rect tool with right mouse button released.
			ad.erased->rect_fill(m_last, SPoint(mx, my), C_BLACK);
		}
	} else if (2 == ibutt) {
		if (lmb_last && !lmb) {
			// Circle tool with left mouse button released.
			ad.erased->fillcircle(m_last, idist(m_last, mx, my), C_WHITE);
		} else if (rmb_last && !rmb) {
			// Circle tool with right mouse button released.
			ad.erased->fillcircle(m_last, idist(m_last, mx, my), C_BLACK);
		}
	}

	display->bitmap()->clear();
	ad.bmp->blit(display->bitmap(), SPoint(0, 0));
	fillsettings fs;
	fs.size = 8;
	fs.background = RGB_GRAY(192);
	fs.foreground = RGB_GRAY(128);
	for (int y = 0; y < ad.bmp->height(); ++y) {
		for (int x = 0; x < ad.bmp->width(); ++x) {
			SBitmap* bmpuse = ad.bmp, * eraseuse = ad.erased;
			if (ad.pred_in_progress && ad.pw.pass > 1) {
				if (ad.pw.erase != nullptr)
					eraseuse = ad.pw.erase;
				if (ad.pw.ret != nullptr)
					bmpuse = ad.pw.ret;
			}
			if (eraseuse->get_red(x, y) == 0) {
				display->bitmap()->put_pixel(x, y, bmpuse->get_pixel(x, y));
			} else {
				display->bitmap()->put_pixel(x, y, checkerboard_pattern(x, y, (void*)&fs));
			}
		}
	}
	if (1 == ibutt && (lmb || rmb)) {
		// Rect tool with mouse button down: show dragged rectangle.
		display->bitmap()->rect(m_last, SPoint(mx, my), C_YELLOW);
	} else if (2 == ibutt && (lmb || rmb)) {
		// Circle tool with mouse button down: show dragged circle.
		display->bitmap()->circle(m_last, idist(m_last, mx, my), C_YELLOW);
	}

	if (ad.pred_in_progress) {
		// Draw the prediction status.
		SBitmap* b = display->bitmap();
		char lines[4][64];
		int y = b->height() / 4;
		SCoord co(ad.bmp->width() + 10, ad.bmp->height() / 4, b->width(), b->height());
		sprintf(lines[0], "Inpainting pass %d", ad.pw.pass);
		sprintf(lines[1], "%d/%d pixels remain", ad.pw.nerased, ad.pw.nerased_in);
		sprintf(lines[2], "Elapsed: %s", timepr(ad.sw.stop(UNIT_MILLISECOND)));
		sprintf(lines[3], "[ETA: %s]", timepr((ad.sw.stop(UNIT_MILLISECOND) * ad.pw.nerased) / 
							((ad.pw.nerased == ad.pw.nerased_in) ? 1 : (ad.pw.nerased_in - ad.pw.nerased))));
		b->render_text(lines[0], co, &font, C_WHITE, 20, ALIGN_TOP | ALIGN_LEFT);
		co += SPoint(0, 24);
		b->render_text(lines[1], co, &font, C_WHITE, 20, ALIGN_TOP | ALIGN_LEFT);
		co += SPoint(0, 24);
		b->render_text(lines[2], co, &font, C_WHITE, 20, ALIGN_TOP | ALIGN_LEFT);
		co += SPoint(0, 24);
		if (ad.pw.pass > 1) {
			b->render_text(lines[3], co, &font, C_WHITE, 20, ALIGN_TOP | ALIGN_LEFT);
		}
		if (ad.pw.erase != nullptr) {
			ad.pw.erase->blit(ecopy, SPoint(0, 0));
		}
	} else if (ad.pw.ace > 0.) {
		SBitmap* b = display->bitmap();
		char line[64];
		sprintf(line, "Error: %f", ad.pw.ace);
		SCoord co(ad.bmp->width() + 10, ad.bmp->height() - 30, b->width() - 10, b->height());
		b->render_text(line, co, &font, C_WHITE, 20, ALIGN_TOP | CENTERED_HORIZ);
	}

	lmb_last = lmb;
	rmb_last = rmb;
}

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: toy [image file]\n";
		return 1;
	}
	SBitmap* bmp = SBitmap::load_bmp(app_argv(1));
	if (is_null(bmp) || bmp->height() < 1) {
		std::cout << "Error loading bitmap " << app_argv(1) << std::endl;
		return 2;
	}
	if (bmp->height() > MAX_HEIGHT) {
		std::cout << "Bitmap too high, rescaling to height " << MAX_HEIGHT << " pixels.\n";
		if (!bmp->resize_and_replace(0, MAX_HEIGHT)) {
			std::cout << "Resize failed.\n";
			return 3;
		}
	}
	if (bmp->width() > MAX_WIDTH) {
		std::cout << "Bitmap too wide, rescaling to width " << MAX_WIDTH << " pixels.\n";
		if (!bmp->resize_and_replace(MAX_WIDTH, 0)) {
			std::cout << "Resize failed.\n";
			return 4;
		}
	}
	ad.bmp = bmp;
	ad.erased = new SBitmap(bmp->width(), bmp->height());
	ad.gobutton = false;
	ad.painted = nullptr;
	ad.nth = 8;
	ad.erased->clear();
	std::string nnet_paths[3];
	find_recent_checkpoints(nnet_paths[0], nnet_paths[1], nnet_paths[2]);
	ad.nnet4.read_from_file(nnet_paths[0].c_str() /* "deepk.rfn" "smol.rfn" */);
	ad.nnet5.read_from_file(nnet_paths[1].c_str() /* "fivek.rfn" "img.rfn" */);
	ad.nnet9.read_from_file(nnet_paths[2].c_str() /* "img.rfn" "image9.rfn" */);
	ad.nnet4.quiet();
	ad.nnet5.quiet();
	ad.nnet9.quiet();
	ad.nnet4.set_predict_window((PredictWindow*)&ad.pw);
	ad.nnet5.set_predict_window((PredictWindow*)&ad.pw);
	ad.nnet9.set_predict_window((PredictWindow*)&ad.pw);
	codehappy_main(main_loop, nullptr, bmp->width() + 300, bmp->height());
	return 0;
}

/*** end toy.cpp ***/
