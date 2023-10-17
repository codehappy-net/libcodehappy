/***

	clip.cpp
	
	A simple image clipping application -- give it a directory containing images and you can cycle through
	them, selecting snippets for your image dataset.
	
	2023, C. M. Street

***/
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>

#ifdef CODEHAPPY_WINDOWS
#define APP_WIDTH	800
#define APP_HEIGHT	600
#else
#define APP_WIDTH	1200
#define APP_HEIGHT	900
#endif

/* App coordinate space to bitmap coordinate space */
bool translate_coord(const SPoint& app, SPoint& bmp, SBitmap* orig_img, SBitmap* scaled_img) {
	int x, y, x1, y1;
	if (is_null(orig_img) || is_null(scaled_img))
		return false;
	x1 = (APP_WIDTH - scaled_img->width()) / 2;
	y1 = (APP_HEIGHT - scaled_img->height()) / 2;
	x = app.X() - x1;
	y = app.Y() - y1;
	if (x < 0 || x >= scaled_img->width())
		return false;
	if (y < 0 || y >= scaled_img->height())
		return false;

	x *= orig_img->width();
	x /= scaled_img->width();
	y *= orig_img->height();
	y /= scaled_img->height();
	bmp = SPoint(x, y);

	return true;
}

/* Bitmap coordinate space to app coordinate space */
void coord_back(const SPoint& bmp, SPoint& app, SBitmap* orig_img, SBitmap* scaled_img) {
	int x = bmp.X(), y = bmp.Y(), x1, y1;
	x1 = (APP_WIDTH - scaled_img->width()) / 2;
	y1 = (APP_HEIGHT - scaled_img->height()) / 2;
	x *= scaled_img->width();
	x /= orig_img->width();
	y *= scaled_img->height();
	y /= orig_img->height();
	app = SPoint(x + x1, y + y1);
}

void fill_directory(std::vector<std::string>& files, const std::string& dir, bool app) {
	DIR* di = opendir(dir.c_str());
	dirent* entry;

	while (entry = readdir(di)) {
		if (entry->d_name[0] == '.')
			continue;
		if (strstr(entry->d_name, ".CLIPPED") != nullptr)
			continue;
		if (app) {
			// Append (really, prepend) the folder name.
			std::string fn = dir;
			fn += entry->d_name;
			files.push_back(fn);
		} else {
			files.push_back(entry->d_name);
		}
	}
	closedir(di);
}

struct AppData {
	std::vector<std::string> files;
	int cur_idx;
	SBitmap* orig;
	SBitmap* scaled;
};

#ifdef CODEHAPPY_WINDOWS
#define MOVE_CMD	"ren "
#else
#define MOVE_CMD	"mv "
#endif

void select_new(AppData* ad) {
	SBitmap* b1 = ad->orig, * b2 = ad->scaled;
	if (!is_null(ad->orig)) {
		std::string last = ad->files[ad->cur_idx];
		std::string rename_last = last + ".CLIPPED";
		std::string sys_cmd;
		rename(last.c_str(), rename_last.c_str());
		//sys_cmd = MOVE_CMD + last + " " + rename_last;
		//system(sys_cmd.c_str());
	}

	forever {
		++ad->cur_idx;
		if (ad->cur_idx >= ad->files.size())
			exit(0);
		const std::string& fn = ad->files[ad->cur_idx];
		ad->orig = nullptr;
		ad->scaled = nullptr;
		ad->orig = SBitmap::load_bmp(fn.c_str());
		if (is_null(ad->orig))
			continue;
		ad->scaled = ad->orig->copy();
		if (is_null(ad->scaled)) {
			delete ad->orig;
			ad->orig = nullptr;
			continue;
		}
		if (ad->scaled->height() > APP_HEIGHT)
			ad->scaled->resize_and_replace(0, APP_HEIGHT);
		if (ad->scaled->width() > APP_WIDTH)
			ad->scaled->resize_and_replace(APP_WIDTH, 0);
		break;
	}

	if (!is_null(b1))
		delete b1;
	if (!is_null(b2))
		delete b2;
}

void save_curated(SBitmap* bmp, const SPoint& ul, const SPoint& lr, const std::string& prefix) {
	SBitmap* blt;
	SCoord co(ul, lr);
	if (co.width() < 100 && co.height() < 100)
		return;
	blt = new SBitmap(co.width(bmp), co.height(bmp));
	bmp->blit(co, blt);
	char* fname = temp_file_name(prefix.c_str(), ".png");
	blt->save_bmp(fname);
	delete fname;
	delete blt;
}

void revert(AppData* ad) {
	const std::string& fn = ad->files[ad->cur_idx];
	SBitmap* b1 = ad->orig, * b2 = ad->scaled;

	ad->orig = nullptr;
	ad->scaled = nullptr;
	ad->orig = SBitmap::load_bmp(fn.c_str());
	if (is_null(ad->orig))
		exit(1);
	ad->scaled = ad->orig->copy();
	if (is_null(ad->scaled)) {
		delete ad->orig;
		ad->orig = nullptr;
		exit(1);
	}
	if (ad->scaled->height() > APP_HEIGHT)
		ad->scaled->resize_and_replace(0, APP_HEIGHT);
	if (ad->scaled->width() > APP_WIDTH)
		ad->scaled->resize_and_replace(APP_WIDTH, 0);
}

RGBColor erase_color(SBitmap* bmp) {
	std::unordered_map<RGBColor, u32> col_count;
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			RGBColor c = bmp->get_pixel(x, y);
			int r, g, b;
			r = RGB_RED(c);
			g = RGB_GREEN(c);
			b = RGB_BLUE(c);
			r &= (~7);
			g &= (~7);
			b &= (~7);
			c = RGB_NO_CHECK(r, g, b);
			col_count[c]++;
		}
	}

	RGBColor leader = C_WHITE;
	int lead_count = 0;
	for (const auto& ey : col_count) {
		if (rgb_intensity(ey.first) < 10) {
			continue;
		}
		if (ey.second > lead_count) {
			leader = ey.first;
			lead_count = ey.second;
		}
	}

	return leader;
}

void erase(AppData* ad, const SPoint& ul, const SPoint& lr, RGBColor erase_col) {
	SBitmap* b = ad->scaled;
	SCoord co(ul, lr);

	ad->orig->rect_fill(co, erase_col);
	ad->scaled = ad->orig->copy();
	if (is_null(ad->scaled))
		exit(1);
	if (ad->scaled->height() > APP_HEIGHT)
		ad->scaled->resize_and_replace(0, APP_HEIGHT);
	if (ad->scaled->width() > APP_WIDTH)
		ad->scaled->resize_and_replace(APP_WIDTH, 0);

	delete b;
}

void rotate_clockwise(AppData* ad) {
	SBitmap* b1 = ad->orig, *b2 = ad->scaled;
	ad->orig = b1->rotate_clockwise_90();
	ad->scaled = ad->orig->copy();
	if (is_null(ad->scaled))
		exit(1);
	if (ad->scaled->height() > APP_HEIGHT)
		ad->scaled->resize_and_replace(0, APP_HEIGHT);
	if (ad->scaled->width() > APP_WIDTH)
		ad->scaled->resize_and_replace(APP_WIDTH, 0);
	delete b1;
	delete b2;
}

void main_loop(Display* display, void* user_data) {
	/* these static variables could be held in the AppData... */
	static KeyLast kl(display);
	static bool in_select = false;
	static bool in_erase = false;
	static SPoint ul, lr;
	static RGBColor erase_col = C_WHITE;
	static std::string prefix = "i";
	AppData* ad = (AppData *) user_data;
	int mx = display->mouse_x(), my = display->mouse_y();

	if (is_null(ad->orig) || is_null(ad->scaled)) {
		select_new(ad);
		if (is_null(ad->orig) || is_null(ad->scaled))
			return;
	}

	if (kl.first()) {
		codehappy_window_title("Image snippet app");
	}

	const int x1 = (APP_WIDTH - ad->scaled->width()) / 2;
	const int y1 = (APP_HEIGHT - ad->scaled->height()) / 2;

	display->bitmap()->clear();
	ad->scaled->blit(display->bitmap(), SPoint(x1, y1));

	bool inbmp = translate_coord(SPoint(mx, my), lr, ad->orig, ad->scaled);
	
	if (in_select && inbmp) {
		if (kl.mouse_now_up(MOUSEBUTTON_LEFT)) {
			in_select = false;
			if (in_erase) {
				erase(ad, ul, lr, erase_col);
			} else {
				// save the image
				save_curated(ad->orig, ul, lr, prefix);
				speaker_beep(2000, 100);
			}
		} else if (kl.mouse_now_down(MOUSEBUTTON_RIGHT)) {
			// right-click cancels a snippet selection in progress
			in_select = false;
		} else {
			// translate ul/lr to app space and draw the rectangle
			SPoint ul_app, lr_app;
			coord_back(ul, ul_app, ad->orig, ad->scaled);
			coord_back(lr, lr_app, ad->orig, ad->scaled);
			display->bitmap()->rect(ul_app, lr_app, in_erase ? C_RED : C_YELLOW);
		}
	} else if (in_select && !inbmp && kl.mouse_now_up(MOUSEBUTTON_LEFT)) {
		// releasing the mouse button out of bounds
		in_select = false;
	} else if (!in_select && inbmp) {
		if (kl.mouse_now_down(MOUSEBUTTON_LEFT)) {
			// we're now in a snippet selection!
			ul = lr;
			in_select = true;
		}
	}
	
	if (kl.now_down(' ') || kl.now_down(SKEY_ENTER) || kl.now_down(SKEY_ESCAPE)) {
		if (in_select) {
			// cancel the selection
			in_select = false;
		} else {
			// select a new image
			select_new(ad);
		}
	}
	
	if (kl.now_down('r') || kl.now_down('R')) {
		// rotate 90 degrees clockwise
		rotate_clockwise(ad);
	}
	if (kl.now_down('e') || kl.now_down('E')) {
		// toggle erase mode
		in_erase = !in_erase;
		if (in_erase) {
			erase_col = erase_color(ad->scaled);
		}
	}
	if (kl.now_down('v') || kl.now_down('V')) {
		// revert to the original file (undoes rotations or erasures)
		revert(ad);
	}
	if (kl.now_down('i') || kl.now_down('I')) {
		// set the output file prefix (nice if there are multiple categories in the dataset)
		prefix = "i";
	}
	if (kl.now_down('j') || kl.now_down('J')) {
		// set the output file prefix (nice if there are multiple categories in the dataset)
		prefix = "j";
	}
	if (kl.now_down('k') || kl.now_down('K')) {
		// set the output file prefix (nice if there are multiple categories in the dataset)
		prefix = "k";
	}
	if (kl.now_down('1')) {
		// a special case: just clip the entire image
		ul = SPoint(0, 0);
		lr = SPoint(ad->orig->width() - 1, ad->orig->height() - 1);
		save_curated(ad->orig, ul, lr, prefix);
		select_new(ad);
	}
	if (kl.now_down('2')) {
		// a special case: clip the image in two halves (horizontally)
		ul = SPoint(0, 0);
		lr = SPoint(ad->orig->width() / 2, ad->orig->height() - 1);
		save_curated(ad->orig, ul, lr, prefix);
		ul = SPoint(ad->orig->width() / 2, 0);
		lr = SPoint(ad->orig->width() - 1, ad->orig->height() - 1);
		save_curated(ad->orig, ul, lr, prefix);
		select_new(ad);
	}
	if (in_erase) {
		display->bitmap()->render_text("Erase Mode", &font_swansea_bold, H_ORANGE, 80, ALIGN_BOTTOM | ALIGN_LEFT);
	}

	kl.save(display);
}

int app_main() {
	ArgParse ap;
	AppData ad;
	std::string dir = ".";

	ap.add_argument("dir", type_string, "directory/folder containing the images to clip");
	ap.ensure_args(argc, argv);
	if (ap.flag_present("dir")) {
		dir = ap.value_str("dir");
	}

	fill_directory(ad.files, dir, true);
	if (ad.files.empty()) {
		codehappy_cerr << "No images found in folder '" << dir << "'.\n";
	} else {
		std::cout << ad.files.size() << " eligible files found in folder.\n";
	}
	ad.cur_idx = 0;
	ad.orig = nullptr;
	ad.scaled = nullptr;
	codehappy_main(main_loop, &ad, APP_WIDTH, APP_HEIGHT);

	return 0;
}

/* end clip.cpp */
