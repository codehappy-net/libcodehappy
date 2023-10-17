/***

	paint.cpp

	Use the ImgNNet to 'paint' on a canvas.

	Copyright (c) 2022 Chris Street.

***/
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>
#include <thread>

#define APP_WIDTH	640
#define APP_HEIGHT	480
#define MAX_WIDTH	900
#define MAX_HEIGHT	600

const u32 NUM_BRUSHES = 16;
const u32 HITRATE_BUFFER_SZ = 1000;
static SBitmap* bmp = nullptr;
static ImgNNet nnet;
static ImgDiscrim dis;
static PredictAccum pa;
static CircBuffer<int> hitrate(HITRATE_BUFFER_SZ);

void prediction_thread(void) {
	ImgBrush* brushes[NUM_BRUSHES];
	u32 e;
	for (e = 0; e < NUM_BRUSHES; ++e) {
		brushes[e] = new ImgBrush(nnet.radius());
		brushes[e]->x = (double)RandU32Range(0, bmp->width() - 1);
		brushes[e]->y = (double)RandU32Range(0, bmp->height() - 1);
	}

	// Now the main improv painting loop.
	forever {
		for (e = 0; e < NUM_BRUSHES; ++e) {
			/* Does the discriminator like what we're doing? */
			float gain = dis.eval(brushes[e], pa);
			if (gain > 0.) {
				nnet.prediction_for_brush(brushes[e], pa);
				hitrate.insert(100);
			} else {
				hitrate.insert(0);
			}
			brushes[e]->brush_update(bmp->width(), bmp->height());
			// Uncomment to have the brushes use the pixels in rad-circle for predictions.
			/* brushes[e]->set_from_predictions(pa); */
		}
	}
}

const int PANEL_HEIGHT = 40;

void main_loop(Display* display, void* user_data) {
	RGBOut ro;
	static bool first = true;
	static u32 frcount = 0;
	Font font(&font_swansea_bold);
	static char text[256] = { 0 };

	if (first) {
		codehappy_window_title("Phosphenes -- Painterly Ponderings");
		display->bitmap()->clear();
		first = false;
	}

	bmp->clear(C_WHITE);
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			int r, g, b;
			if (pa.get_num_predictions_lock(x, y) == 0)
				continue;
			pa.get_avg_prediction_lock(x, y, ro);
			r = (int)floor(ro.r * 255. + 0.5);
			g = (int)floor(ro.g * 255. + 0.5);
			b = (int)floor(ro.b * 255. + 0.5);
			bmp->put_pixel(x, y, MAKE_RGB(r, g, b));
		}
	}

	char path[32];
	bmp->blit(display->bitmap(), SPoint(0, 0));
	++frcount;
	if (frcount % 100 == 0) {
		sprintf(path, "frame%06d.png", frcount / 100);
		bmp->save_bmp(path);
		double entropy = double(filelen(path)) / double(bmp->width() * bmp->height() * 4 + 733 /* for the PNG header */);
		entropy = std::min(entropy, (double)1.0);
		int hr = hitrate.mean();
		sprintf(text, "Entropy: %0.4f  Discriminator pass rate (last %d): %d%%", entropy, hr, HITRATE_BUFFER_SZ);
		SCoord co(SPoint(0, bmp->height()), SPoint(bmp->width(), display->bitmap()->height()));
		display->bitmap()->rect_fill(co, C_BLACK);
		display->bitmap()->render_text(text, co, &font, C_WHITE, 12, CENTERED_BOTH);
		display->bitmap()->save_bmp(path);
	}
}

int app_main() {
	int x, y;

	/* Create initial bitmap and predictions */
	if (app_argc() > 1) {
		bmp = SBitmap::load_bmp(app_argv(1));
		if (is_null(bmp) || bmp->height() < 1) {
			std::cout << "Error loading bitmap " << app_argv(1) << std::endl;
			return 1;
		}
		if (bmp->height() > MAX_HEIGHT) {
			std::cout << "Bitmap too high, rescaling to height " << MAX_HEIGHT << " pixels.\n";
			if (!bmp->resize_and_replace(0, MAX_HEIGHT)) {
				std::cout << "Resize failed.\n";
				return 2;
			}
		}
		if (bmp->width() > MAX_WIDTH) {
			std::cout << "Bitmap too wide, rescaling to width " << MAX_WIDTH << " pixels.\n";
			if (!bmp->resize_and_replace(MAX_WIDTH, 0)) {
				std::cout << "Resize failed.\n";
				return 3;
			}
		}
		for (y = 0; y < (int)bmp->height(); ++y) {
			for (x = 0; x < (int)bmp->width(); ++x) {
				RGBColor c = bmp->get_pixel(x, y);
				RGBOut ro;
				ro.r = double(RGB_RED(c)) / 255.;
				ro.g = double(RGB_GREEN(c)) / 255.;
				ro.b = double(RGB_BLUE(c)) / 255.;
				pa.add_prediction(x, y, ro);
			}
		}
	} else {
		bmp = new SBitmap(APP_WIDTH, APP_HEIGHT);
		/* We need initial prediction values for the discriminator: let's fill them randomly at start. */
		for (y = -20; y < (int)bmp->height() + 20; ++y) {
			for (x = -20; x < (int)bmp->width() + 20; ++x) {
				RGBOut ro;
				ro.r = RandDouble(0., 1.);
				ro.g = RandDouble(0., 1.);
				ro.b = RandDouble(0., 1.);
				pa.add_prediction(x, y, ro);
			}
		}
	}

	/* Load the necessary models. */
	std::cout << "Reading inpainting neural network...\n";
	nnet.read_from_file("nnets/fivek.rfn.checkpoint21");
//	nnet.read_from_file("nnets/chungus.rfn.checkpoint16");
	nnet.quiet();
	nnet.set_out_erased(false);
	std::cout << "Reading discriminator neural network...\n";
	dis.load_from_file("nnets/discriminator.fivek21");
//	dis.load_from_file("nnets/discriminator.chungus16");

	/* Begin the prediction thread and the main painting thread. */
	std::thread* th = new std::thread(prediction_thread);
	codehappy_main(main_loop, nullptr, bmp->width(), bmp->height() + PANEL_HEIGHT);

	return 0;
}
