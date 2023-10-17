/***

	nnet.cpp

	Example of neural net training. This application trains a neural network to
	predict the perimeter of pixels at distance (5, 6] from a point from the pixels
	at distance [0, 5]. It can do this using 3 neural networks (one for each chroma
	channel) or from one extra-large neural network.

	Copyright (c) 2022 Chris Street. 

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

/***

	This code calculates that there are 81 pixels within 5 of center, and the
	6-perimeter of pixels around it contains 32 pixels. Thus our neural net
	will have 81 inputs and 32 outputs:

	int x, y, c5 = 0, c6 = 0;
	for (x = -6; x <= 6; ++x) {
		for (y = -6; y <= 6; ++y) {
			int d = (x * x) + (y * y);
			if (d <= 25)
				++c5;
			if (d > 25 && d <= 36)
				++c6;
		}
	}
	std::cout << "Within 5: " << c5 << "; the number within the 6-perimeter is " << c6 << std::endl;

***/
#define NNET_3CHANNELS

const u32 NNET_INPUTS = 81;
const u32 NNET_OUTPUTS = 32;
const u32 NNET_HIDDEN_LAYERS = 2;
#ifdef NNET_3CHANNELS
const u32 NNET_HIDDEN_NEURONS = 1200;
#else
const u32 NNET_HIDDEN_NEURONS = 200;
#endif
double NNET_LEARNING_RATE = 0.01; // 0.05; // 0.01; // 0.05;
const double NNET_LEARNING_HIGH = 0.01;
const double NNET_LEARNING_LOW = 0.005;
const u32 PREDICT_PIXEL_STEP = 1;
bool half_learning_rate = false;
bool double_learning_rate = false;

#define NNET_RED	"imgnnet.red"
#define NNET_GREEN	"imgnnet.green"
#define NNET_BLUE	"imgnnet.blue"
#define NNET_ALL	"img.nnet"

#ifdef NNET_3CHANNELS
u32 neurons = 1200;
const char* nnet_name(void) {
	if (400 == neurons)
		return "img.nnet.400";
	return NNET_ALL;
}
#endif

double channel_intensity(SBitmap* bmp, u32 ch, int x, int y) {
	u32 v = 0;
	switch (ch) {
	case CHANNEL_RED:
		v = bmp->get_red(x, y);
		break;
	case CHANNEL_GREEN:
		v = bmp->get_green(x, y);
		break;
	case CHANNEL_BLUE:
		v = bmp->get_blue(x, y);
		break;
	}
	return double(v) / 255.0;
}

genann* read_from_file(const char* fname) {
	FILE* f = fopen(fname, "r");
	genann* ret = genann_read(f);
	fclose(f);
	return ret;
}

void out_to_file(genann* nnet, const char* fname) {
	FILE* f = fopen(fname, "w");
	genann_write(nnet, f);
	fclose(f);
}

const char* timepr(u64 mills) {
	static char st[2][32];
	static u32 idx = 0;
	char* ret;
	int min = mills / 60000;
	int sec = mills / 1000;
	int mil = int(mills - sec * 1000);
	sec -= min * 60;
	ret = st[idx++];
	if (idx > 1)
		idx = 0;
	sprintf(ret, "%02d:%02d.%03d", min, sec, mil);
	return ret;
}


/***

	If "fast", then only every 16th pixel (x,y congruent to citer mod 4) is used for training.
	x,y congruent to 0 mod 4 are used for validation. The 'small' flag is set, too.

***/
static bool fast = false;
static bool skiperr = false;

double iterative_error(genann* nnet, SBitmap* bmp, Stopwatch& sw) {
	double in[NNET_INPUTS * 3];
	int x, y;
	u32 ci, co;
	bool skip;
#define	CALC_ERROR(x)	{ int ca = (int)(x); \
			  double v = pout[co++]; \
			  v = CLAMP(v, 0., 1.); \
			  int vi = (int)floor(v * 255. + 0.5); \
			  comp_error += std::abs(vi - ca); \
			  cd++; }
	skip = true;
	i64 comp_error = 0, cd = 0;
	const double* pout;

	std::cout << "Calculating error...\n";
	for (y = 6; y < bmp->height() - 7; ++y) {
		if ((y & 7) == 0) {
			printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp->height() - 6, 
				timepr(sw.stop(UNIT_MILLISECOND)),
				timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - 6 - y)) / ((y == 0) ? 1 : y)));
		}
		for (x = 6; x < bmp->width() - 7; ++x) {
			int dx, dy, ds;
			ci = 0;
			co = 0;
			if (!fast && skip) {
				skip = false;
				continue;
			}
			if (fast && ((x & 3) != 0 || (y & 3) != 0)) {
				continue;
			}
			for (dy = -5; dy <= 5; ++dy) {
				for (dx = -5; dx <= 5; ++dx) {
					ds = (dy * dy) + (dx * dx);
					ds = (dy * dy) + (dx * dx);
					if (ds <= 25) {
						/* Within 5 pixels, one of the inputs. */
#ifdef NNET_3CHANNELS
						in[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
						in[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
						in[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
#else
						in[ci++] = channel_intensity(bmp, channel_flag, x + dx, y + dy);
#endif
					};
				}
			}
#ifdef NNET_3CHANNELS
			ship_assert(ci == NNET_INPUTS * 3);
#else
			ship_assert(ci == NNET_INPUTS);
#endif
			pout = genann_run(nnet, in);
			skip = !skip;
			for (dy = -6; dy <= 6; ++dy) {
				for (dx = -6; dx <= 6; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 36 && ds > 25) {
						/* Within the 6-perimeter, one of the outputs. */
#ifdef NNET_3CHANNELS
						CALC_ERROR(bmp->get_red(x + dx, y + dy));
						CALC_ERROR(bmp->get_green(x + dx, y + dy));
						CALC_ERROR(bmp->get_blue(x + dx, y + dy));
#else
						if (channel_flag == CHANNEL_RED)
							CALC_ERROR(bmp->get_red(x + dx, y + dy));
						if (channel_flag == CHANNEL_GREEN)
							CALC_ERROR(bmp->get_green(x + dx, y + dy));
						if (channel_flag == CHANNEL_BLUE)
							CALC_ERROR(bmp->get_blue(x + dx, y + dy));
#endif
					}
				}
			}
#ifdef NNET_3CHANNELS
			ship_assert(co == NNET_OUTPUTS * 3);
#else
			ship_assert(co == NNET_OUTPUTS);
#endif
		}
	}
	double retval = double(comp_error) / double(cd);
	std::cout << "Done in [" << timepr(sw.stop(UNIT_MILLISECOND)) << "] / Total error: " << comp_error << "                \n";
	std::cout << "Error per component: " << retval << "\n";
	return retval;
}

#ifdef NNET_3CHANNELS
u32 train_on_image(genann* nnet, SBitmap* bmp, bool iterative) {
	double in[NNET_INPUTS * 3];
	double out[NNET_OUTPUTS * 3];
#else
u32 train_on_image(genann* nnet, SBitmap* bmp, u32 channel_flag, bool iterative) {
	double in[NNET_INPUTS];
	double out[NNET_OUTPUTS];
#endif
	Stopwatch sw;
	int x, y;
	u32 ci, co;
	bool skip;
	u32 citer = 1;
	u32 retry = 0;
	double n_err, err_next;
	const u32 MAX_ITERATIONS = 3; // = 4;

	ship_assert(NNET_LEARNING_LOW < NNET_LEARNING_HIGH);
	if (skiperr)
		n_err = 999999.;
	else
		n_err = iterative_error(nnet, bmp, sw);
	// Use a higher learning rate for the first pass: if it doesn't
	// work out, we will lower it.
	NNET_LEARNING_RATE = NNET_LEARNING_HIGH;
	if (half_learning_rate)
		NNET_LEARNING_RATE /= 2.0;
	if (double_learning_rate)
		NNET_LEARNING_RATE *= 2.0;

	// The iterative training loop.
	forever {

	sw.start();
	skip = false;
	for (y = 6; y < bmp->height() - 7; ++y) {
		if ((y & 1) == 0) {
			printf("Row %d of %d (%s) [ETA: %s]...\r", y, bmp->height() - 6, 
				timepr(sw.stop(UNIT_MILLISECOND)),
				timepr((sw.stop(UNIT_MILLISECOND) * (bmp->height() - 6 - y)) / ((y == 0) ? 1 : y)));
		}
		for (x = 6; x < bmp->width() - 7; ++x) {
			int dx, dy, ds;
			ci = 0;
			co = 0;
			if (iterative && !fast && skip) {
				skip = false;
				continue;
			}
			if (fast && ((x & 3) != (citer & 3) || (y & 3) != (citer & 3))) {
				continue;
			}
			for (dy = -6; dy <= 6; ++dy) {
				for (dx = -6; dx <= 6; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 25) {
						/* Within 5 pixels, one of the inputs. */
#ifdef NNET_3CHANNELS
						in[ci++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
						in[ci++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
						in[ci++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
#else
						in[ci++] = channel_intensity(bmp, channel_flag, x + dx, y + dy);
#endif
					} else if (ds <= 36) {
						/* Within the 6-perimeter, one of the outputs. */
#ifdef NNET_3CHANNELS
						out[co++] = channel_intensity(bmp, CHANNEL_RED, x + dx, y + dy);
						out[co++] = channel_intensity(bmp, CHANNEL_GREEN, x + dx, y + dy);
						out[co++] = channel_intensity(bmp, CHANNEL_BLUE, x + dx, y + dy);
#else
						out[co++] = channel_intensity(bmp, channel_flag, x + dx, y + dy);
#endif
					}
				}
			}
#ifdef NNET_3CHANNELS
			ship_assert(ci == NNET_INPUTS * 3);
			ship_assert(co == NNET_OUTPUTS * 3);
#else
			ship_assert(ci == NNET_INPUTS);
			ship_assert(co == NNET_OUTPUTS);
#endif
			genann_train(nnet, in, out, NNET_LEARNING_RATE);
			skip = !skip;
		}
	}
	printf("Completed in [%s]                         \n", timepr(sw.stop(UNIT_MILLISECOND)));
	sw.start();

	/* Now try verifying the training. */
	err_next = iterative_error(nnet, bmp, sw);
	if (err_next < n_err) {
		std::cout << "Error improved! Saving the neural network to file.\n";
		out_to_file(nnet, nnet_name());
		n_err = err_next;
		if (citer >= MAX_ITERATIONS) {
			std::cout << "Maximum number of iterations performed.\n";
			break;
		}
	} else {
		if (iterative && 1 == citer && 0 == retry /* && NNET_LEARNING_RATE >= NNET_LEARNING_HIGH */) {
			std::cout << "Error did not improve: reloading neural net, lowering the learning rate, and retrying first iteration.\n";
			nnet = read_from_file(nnet_name());
			NNET_LEARNING_RATE = NNET_LEARNING_LOW;
			if (half_learning_rate)
				NNET_LEARNING_RATE /= 2.0;
			if (double_learning_rate)
				NNET_LEARNING_RATE *= 2.0;
			++retry;
			continue;
		}
		std::cout << "Error did not improve, stopping.\n";
		return (citer - 1);
	}

	if (!iterative)
		return citer;

	if (NNET_LEARNING_RATE >= NNET_LEARNING_HIGH) {
		NNET_LEARNING_RATE = NNET_LEARNING_LOW;
		if (half_learning_rate)
			NNET_LEARNING_RATE /= 2.0;
		if (double_learning_rate)
			NNET_LEARNING_RATE *= 2.0;
		std::cout << "Setting learning rate to " << NNET_LEARNING_RATE << " for subsequent iterations.\n";
	}
	++citer;
	std::cout << "Starting iteration #" << citer << "...\n";
	}  // forever (iterative training loop)

	return citer;
}

struct RGBOut {
	double r;
	double g;
	double b;
	RGBOut operator+=(const RGBOut& rhs);
};

RGBOut RGBOut::operator+=(const RGBOut& rhs) {
	r += rhs.r;
	g += rhs.g;
	b += rhs.b;
	return *this;
}

class PredictAccum {
public:
#ifdef NNET_3CHANNELS
	void add_prediction(int x, int y, RGBOut& predict);
	void get_avg_prediction(int x, int y, RGBOut& p);
	void get_total_prediction(int x, int y, RGBOut& p);
#else
	void add_prediction(int x, int y, double predict);
	double get_avg_prediction(int x, int y);
	double get_total_prediction(int x, int y);
#endif
	u32 get_num_predictions(int x, int y);
	void reset();

	struct PredictHash {
	    size_t operator()(std::pair<int, int> p) const noexcept {
	        return (size_t)(p.first * 3557 + p.second);
	    }
	};

private:
#ifdef NNET_3CHANNELS
	std::unordered_map< std::pair<int, int>, std::pair<RGBOut, int>, PredictHash > predictions;
#else
	std::unordered_map< std::pair<int, int>, std::pair<double, int>, PredictHash > predictions;
#endif
};

#ifdef NNET_3CHANNELS

void PredictAccum::add_prediction(int x, int y, RGBOut& predict) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		predictions[pr] = std::make_pair(predict, 1L);
		return;
	}
	auto p = predictions[pr];
	p.first += predict;
	p.second++;
	predictions[pr] = p;
}

void PredictAccum::get_avg_prediction(int x, int y, RGBOut& p) {
	u32 c = get_num_predictions(x, y);
	if (0 == c) {
		p.r = 0.;
		p.g = 0.;
		p.b = 0.;
		return;
	}
	get_total_prediction(x, y, p);
	p.r /= (double)c;
	p.g /= (double)c;
	p.b /= (double)c;
	p.r = CLAMP(p.r, 0.0, 1.0);
	p.g = CLAMP(p.g, 0.0, 1.0);
	p.b = CLAMP(p.b, 0.0, 1.0);
}

void PredictAccum::get_total_prediction(int x, int y, RGBOut& p) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		p.r = 0.;
		p.g = 0.;
		p.b = 0.;
		return;
	}
	p = predictions[pr].first;
	return;
}

#else  // !NNET_3CHANNELS

void PredictAccum::add_prediction(int x, int y, double predict) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end()) {
		predictions[pr] = std::make_pair(predict, 1L);
		return;
	}
	auto p = predictions[pr];
	p.first += predict;
	p.second++;
	predictions[pr] = p;
}

double PredictAccum::get_avg_prediction(int x, int y) {
	u32 c = get_num_predictions(x, y);
	if (0 == c)
		return 0.0;
	return get_total_prediction(x, y) / (double)c;
}

double PredictAccum::get_total_prediction(int x, int y) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end())
		return 0.;
	return predictions[pr].first;
}
#endif  // !NNET_3CHANNELS

u32 PredictAccum::get_num_predictions(int x, int y) {
	auto pr = std::make_pair(x, y);
	if (predictions.find(pr) == predictions.end())
		return 0;
	return predictions[pr].second;
}

void PredictAccum::reset() {
	predictions.clear();
}

#ifndef NNET_3CHANNELS
void predict_channel_from_nnet(SBitmap* bin, SBitmap* bout, genann* nnet, u32 channel) {
	PredictAccum pa;
	double in[NNET_INPUTS];
	double const* out;
	int x, y, dx, dy;
	u32 ci, co;

	std::cout << "Accumulating predictions...\n";
	for (y = 5; y < bin->height() - 5; y += PREDICT_PIXEL_STEP) {
		if ((y % 200) == 0)
			std::cout << "Row " << y << " of " << bin->height() - 5 << std::endl;
		for (x = 5; x < bin->width() - 5; x += PREDICT_PIXEL_STEP) {
			/* Get the inputs in order. */
			int dx, dy, ds;
			ci = 0;
			for (dy = -5; dy <= 5; ++dy) {
				for (dx = -5; dx <= 5; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 25) {
						/* Within 5 pixels, one of the inputs. */
						in[ci++] = channel_intensity(bin, channel, x + dx, y + dy);
					}
				}
			}
			/* Inputs filled, now calculate outputs and populate predictions for 6-perimeter. */
			co = 0;
			ship_assert(ci == NNET_INPUTS);
			out = genann_run(nnet, in);
			for (dy = -6; dy <= 6; ++dy) {
				for (dx = -6; dx <= 6; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 36 && ds > 25) {
						/* Part of the 6-perimeter. */
						pa.add_prediction(x + dx, y + dy, out[co++]);
					}
				}
			}
			ship_assert(co == NNET_OUTPUTS);
		}
	}

	/* Now fill the desired channel in the output bitmap with the average of predictions. */
	std::cout << "Rendering channel...\n";
	for (y = 0; y < bin->height(); ++y) {
		for (x = 0; x < bin->width(); ++x) {
			double v = pa.get_avg_prediction(x, y);
			v = CLAMP(v, 0.0, 1.0);
			v *= 255.0;
			v = floor(v + 0.5);
			switch (channel) {
			case CHANNEL_RED:
				bout->set_red(x, y, (u32)v);
				break;
			case CHANNEL_BLUE:
				bout->set_blue(x, y, (u32)v);
				break;
			case CHANNEL_GREEN:
				bout->set_green(x, y, (u32)v);
				break;
			}
		}
	}
}

SBitmap* predict_from_nnets(SBitmap* bmp, genann* red, genann* green, genann* blue) {
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	NOT_NULL_OR_RETURN(ret, ret);
	ret->clear();
	std::cout << "Predicting red channel.\n";
	predict_channel_from_nnet(bmp, ret, red, CHANNEL_RED);
	std::cout << "Predicting green channel.\n";
	predict_channel_from_nnet(bmp, ret, green, CHANNEL_GREEN);
	std::cout << "Predicting blue channel.\n";
	predict_channel_from_nnet(bmp, ret, blue, CHANNEL_BLUE);
	return ret;
}
#endif  // !NNET_3CHANNELS

enum EraseType {
	ERASE_CENTER,
	ERASE_BANDS,
	ERASE_STATIC,
	ERASE_RANDOM_RECTS,
	ERASE_STATIC2,
	ERASE_STATIC3,
	ERASE_DICE,
	ERASE_HALFSAW,
};

void erase_type(SBitmap* e, EraseType et) {
	e->clear();
	switch (et) {
	case ERASE_CENTER:
		std::cout << "Using erase type center.\n";
		e->rect_fill(SPoint(40, POINT_PERCENT, 40, POINT_PERCENT), SPoint(60, POINT_PERCENT, 60, POINT_PERCENT), C_WHITE);
		break;
	case ERASE_DICE:
		std::cout << "Using erase type dice.\n";
		e->rect_fill(SPoint(40, POINT_PERCENT, 40, POINT_PERCENT), SPoint(60, POINT_PERCENT, 60, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(0, 0), SPoint(20, POINT_PERCENT, 20, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(80, POINT_PERCENT, 0, POINT_PERCENT), SPoint(100, POINT_PERCENT, 20, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(0, POINT_PERCENT, 80, POINT_PERCENT), SPoint(20, POINT_PERCENT, 100, POINT_PERCENT), C_WHITE);
		e->rect_fill(SPoint(80, POINT_PERCENT, 80, POINT_PERCENT), SPoint(100, POINT_PERCENT, 100, POINT_PERCENT), C_WHITE);
		break;
	case ERASE_BANDS:
		std::cout << "Using erase type regular bands.\n";
		for (int x = 3; x < e->width(); x += 14)
			e->vline(x, 0, e->height(), C_WHITE);
		for (int y = 5; y < e->height(); y += 14)
			e->hline(0, e->width(), y, C_WHITE);
		break;
	case ERASE_RANDOM_RECTS:
		std::cout << "Using erase type random rects.\n";
		for (int c = 0; c < 20; ) {
			SCoord co;
			e->random_rect(&co);
			if (co.dx() * co.dy() > 4000)
				continue;
			e->rect_fill(co, C_WHITE);
			++c;
		}
		break;
	case ERASE_STATIC:
		std::cout << "Using erase type static (5% random).\n";
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 19) == 0)
					e->put_pixel(x, y, C_WHITE);
			}
		}
		break;
	case ERASE_STATIC2:
		std::cout << "Using erase type static #2 (2.5% random, 2x2 rects).\n";
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 39) == 0)
					e->rect_fill(x, y, x + 1, y + 1, C_WHITE);
			}
		}
		break;
	case ERASE_STATIC3:
		std::cout << "Using erase type static #3 (5% random, 2x2 rects).\n";
		for (int x = 0; x < e->width(); ++x) {
			for (int y = 0; y < e->height(); ++y) {
				if (RandU32Range(0, 19) == 0)
					e->rect_fill(x, y, x + 1, y + 1, C_WHITE);
			}
		}
		break;
	case ERASE_HALFSAW:
		std::cout << "Using erase type half saw.\n";
		for (int x = 0; x < 10; ++x) {
			e->line(SPoint(x * 10, POINT_PERCENT, 55, POINT_PERCENT), SPoint(x * 10 + 5, POINT_PERCENT, 45, POINT_PERCENT), C_WHITE);
			e->line(SPoint(x * 10 + 5, POINT_PERCENT, 45, POINT_PERCENT), SPoint(x * 10 + 10, POINT_PERCENT, 55, POINT_PERCENT), C_WHITE);
		}
		e->floodfill(SPoint(50, POINT_PERCENT, 90, POINT_PERCENT), C_WHITE);
		break;
	}
}

u32 color_distance(RGBColor c1, RGBColor c2) {
	int ret = 0;
	ret += std::abs((int)RGB_RED(c1) - (int)RGB_RED(c2));
	ret += std::abs((int)RGB_GREEN(c1) - (int)RGB_GREEN(c2));
	ret += std::abs((int)RGB_BLUE(c1) - (int)RGB_BLUE(c2));
	return (u32)ret;
}

bool use_neighbors = false;
u32 max_neighbors = 6;
int MAX_PASS_NEIGHBORS = 8;
bool nearest_neighbor(SBitmap* bin, SBitmap* berase, int x, int y, RGBColor& c_out) {
	RGBColor n[8], avg;
	u32 cn = 0, bs, dm;
	int dx, dy;
	int r = 0, g = 0, b = 0;

	for (dy = -1; dy <= 1; ++dy) {
		for (dx = -1; dx <= 1; ++dx) {
			if (dx == 0 && dy == 0)
				continue;
			if (!pixel_ok(berase, x + dx, y + dy))
				continue;
			if (berase->get_red(x + dx, y + dy) != 0)
				continue;
			n[cn] = bin->get_pixel(x + dx, y + dy);
			r += RGB_RED(n[cn]);
			g += RGB_GREEN(n[cn]);
			b += RGB_BLUE(n[cn]);
			++cn;
		}
	}

	if (0 == cn)
		return false;
	c_out = n[0];
	if (1 == cn)
		return true;

	r /= cn;
	g /= cn;
	b /= cn;
	avg = MAKE_RGB(r, g, b);
	bs = color_distance(c_out, avg);
	for (dx = 1; dx < cn; ++dx) {
		dm = color_distance(n[dx], avg);
		if (dm < bs) {
			bs = dm;
			c_out = n[dx];
		}
	}

	return true;
}

/* Compare against replacing missing pixels with the best average pixel (or last valid, that failing). */
double error_bmp_best_neighbor_pixel(SBitmap* o, SBitmap* e) {
	RGBColor last = RGB_GRAY(127);
	u64 ret = 0;
	u32 c = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1, cn;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				if (nearest_neighbor(o, e, x, y, cn))
					ret += color_distance(c1, cn);
				else
					ret += color_distance(c1, last);
				c += 3;
			} else {
				last = c1;
			}
		}
	}
	return double(ret) / double(c);
}

/* Compare against replacing missing pixels with the last valid pixel. */
double error_bmp_last_pixel(SBitmap* o, SBitmap* e) {
	RGBColor last = RGB_GRAY(127);
	u64 ret = 0;
	u32 c = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				ret += color_distance(c1, last);
				c += 3;
			} else {
				last = c1;
			}
		}
	}
	return double(ret) / double(c);
}

/* Compare against the average color of erased pixels. */
double error_bmp_avg(SBitmap* o, SBitmap* e) {
	u64 ret = 0;
	u64 r = 0, g = 0, b = 0;
	u32 ce = 0;
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			RGBColor c1;
			c1 = o->get_pixel(x, y);
			if (e->get_red(x, y) > 0) {
				++ce;
				r += RGB_RED(c1);
				g += RGB_GREEN(c1);
				b += RGB_BLUE(c1);
			}
		}
	}
	if (0 == ce)
		return 0ULL;
	r /= ce;
	g /= ce;
	b /= ce;
	RGBColor ca = RGB_NO_CHECK(r, g, b);
	for (int y = 0; y < o->height(); ++y) {
		for (int x = 0; x < o->width(); ++x) {
			if (e->get_red(x, y) > 0) {
				RGBColor c1;
				c1 = o->get_pixel(x, y);
				ret += std::abs((int)RGB_RED(c1) - (int)RGB_RED(ca)); // * (RGB_RED(c1) - RGB_RED(ca));
				ret += std::abs((int)RGB_GREEN(c1) - (int)RGB_GREEN(ca)); // * (RGB_GREEN(c1) - RGB_GREEN(ca));
				ret += std::abs((int)RGB_BLUE(c1) - (int)RGB_BLUE(ca)); // * (RGB_BLUE(c1) - RGB_BLUE(ca));
			}
		}
	}

	return double(ret) / double(ce * 3);
}

double error_bmp(SBitmap* b1, SBitmap* b2, SBitmap* e) {
	u64 ret = 0;
	u32 ce = 0;
	for (int y = 0; y < b1->height(); ++y) {
		for (int x = 0; x < b1->width(); ++x) {
			RGBColor c1, c2;
			if (e->get_red(x, y) > 0)
				ce += 3;
			c1 = b1->get_pixel(x, y);
			c2 = b2->get_pixel(x, y);
			ret += color_distance(c1, c2);
		}
	}
	return double(ret) / double(ce);
}

bool low_min_predict = false;

#ifdef NNET_3CHANNELS
bool predict_pass_from_nnet_missing(SBitmap* bin, SBitmap* berase, SBitmap* bout, genann* nnet, u32 min_predict) {
	PredictAccum pa;
	static u32 pass = 0;
	bool ret = false;
	RGBOut ov;
	double in[NNET_INPUTS * 3];
	double const* out;
	int x, y, dx, dy, ds;
	u32 ci, co, ce = 0, cneighbors;

	if (low_min_predict)
		min_predict = 2;

	++pass;
	std::cout << "Pass " << pass << "... ";

	for (y = 5; y < bin->height() - 5; y += PREDICT_PIXEL_STEP) {
		for (x = 5; x < bin->width() - 5; x += PREDICT_PIXEL_STEP) {
			bool any_erased = false;
			for (dy = -6; dy <= 6 && !any_erased; ++dy) {
				for (dx = -6; dx <= 6 && !any_erased; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds > 36 || ds < 26)
						continue;
					if (!pixel_ok(berase, x + dx, y + dy))
						continue;
					if (berase->get_red(x + dx, y + dy) != 0)
						any_erased = true;
				}
			}
			if (!any_erased)
				continue;

			/* At least one pixel on our 6-perimeter was erased; let's calculate our predictions. */
			ci = 0;
			cneighbors = 0;
			for (dy = -5; dy <= 5; ++dy) {
				for (dx = -5; dx <= 5; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 25) {
						/* Within 5 pixels, one of the inputs. */
						if (berase->get_red(x + dx, y + dy) != 0) {
							/* If we're allowed to use neighbors, we might still be able to make this prediction. */
							if (!use_neighbors || pass > MAX_PASS_NEIGHBORS)
								continue;
							RGBColor cn;
							bool gotn = nearest_neighbor(bin, berase, x + dx, x + dy, cn);
							if (!gotn)
								continue;
							/* Won't you be my neighbor? */
							in[ci++] = double(RGB_RED(cn)) / 255.0;
							in[ci++] = double(RGB_GREEN(cn)) / 255.0;
							in[ci++] = double(RGB_BLUE(cn)) / 255.0;
							++cneighbors;
							continue;
						}
						in[ci++] = channel_intensity(bin, CHANNEL_RED, x + dx, y + dy);
						in[ci++] = channel_intensity(bin, CHANNEL_GREEN, x + dx, y + dy);
						in[ci++] = channel_intensity(bin, CHANNEL_BLUE, x + dx, y + dy);
					}
				}
			}
			if (ci < NNET_INPUTS * 3) {
				/* Not enough valid inputs, must have some erased with no neighbors permitted. Skip. */
				continue;
			}
			if (use_neighbors && cneighbors > max_neighbors) {
				/* Too many neighbors? That's not very Mr. Rogers! */
				continue;
			}
			/* Inputs filled, now calculate outputs and populate predictions for 6-perimeter. */
			co = 0;
			ship_assert(ci == NNET_INPUTS * 3);
			out = genann_run(nnet, in);
			for (dy = -6; dy <= 6; ++dy) {
				for (dx = -6; dx <= 6; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 36 && ds > 25) {
						/* Part of the 6-perimeter. */
						ov.r = out[co++];
						ov.g = out[co++];
						ov.b = out[co++];
						pa.add_prediction(x + dx, y + dy, ov);
					}
				}
			}
			ship_assert(co == NNET_OUTPUTS * 3);
		}
	}

	/* Now fill in any erased pixel with at least min_predict predictions. */
	for (y = 0; y < bin->height(); ++y) {
		for (x = 0; x < bin->width(); ++x) {
			if (berase->get_red(x, y) == 0) {
				/* Not erased. If this is the first pass, it's OK to set the pixel.
				   Successive passes shouldn't do it because we change berase. */
				if (1 == pass) {
					bout->put_pixel(x, y, bin->get_pixel(x, y));
				}
				continue;
			}
			++ce;
			if (pa.get_num_predictions(x, y) < min_predict)
				continue;
			pa.get_avg_prediction(x, y, ov);
			bout->set_red(x, y, (u32)floor(ov.r * 255. + 0.5));
			bout->set_green(x, y, (u32)floor(ov.g * 255. + 0.5));
			bout->set_blue(x, y, (u32)floor(ov.b * 255. + 0.5));
			bin->set_red(x, y, (u32)floor(ov.r * 255. + 0.5));
			bin->set_green(x, y, (u32)floor(ov.g * 255. + 0.5));
			bin->set_blue(x, y, (u32)floor(ov.b * 255. + 0.5));
			/* Mark this pixel as no longer erased. */
			berase->put_pixel(x, y, C_BLACK);
			ret = true;
			--ce;
		}
	}

	std::cout << "(" << ce << " pixels remain erased.) \r";
	if (!ret)
		std::cout << std::endl;

	return ret;
}

SBitmap* predict_from_nnets_missing(SBitmap* bmp, genann* nnet, EraseType et) {
	SBitmap* orig = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* erased = new SBitmap(bmp->width(), bmp->height());
	SBitmap* e1 = new SBitmap(bmp->width(), bmp->height());
	u32 min_pred = 4;
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(erased, erased);
	ret->clear();
	bmp->blit(orig, SPoint(0, 0));

	erase_type(e1, et);
	if (et == ERASE_BANDS)
		min_pred = 2;

	std::cout << "Predicting erased portions of bitmap on 6-perimeter.\n";
	e1->blit(erased, SPoint(0, 0));
	while (predict_pass_from_nnet_missing(bmp, erased, ret, nnet, min_pred));

	fillsettings fs;
	fs.size = 8;
	fs.background = RGB_GRAY(192);
	fs.foreground = RGB_GRAY(128);
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (e1->get_red(x, y) != 0)
				bmp->put_pixel(x, y, checkerboard_pattern(x, y, (void*)&fs));
		}
	}
	std::cout << "Error between original and erased   : " << error_bmp(orig, bmp, e1) << std::endl;
	std::cout << "Error between original and avg.pixel: " << error_bmp_avg(orig, e1) << std::endl;
	std::cout << "Error between original and lastpixel: " << error_bmp_last_pixel(orig, e1) << std::endl;
	std::cout << "Error between original and neighbors: " << error_bmp_best_neighbor_pixel(orig, e1) << std::endl;
	std::cout << "Error between original and predicted: " << error_bmp(orig, ret, e1) << std::endl;
	bmp->save_bmp("predict_in.png");
	std::cout << "The erased bitmap is saved as predict_in.png\n";
	orig->blit(bmp, SPoint(0, 0));

	return ret;
}


/*** Produce a neural net whole image 'filter'. The image is divided into 5x5 blocks, and four
     erase restores are done in sequence:

	.#.#.    #.#.#    .....    .....
	.....    .....    .#.#.    #.#.#
	#.#.#    .#.#.    .....    .....
	.....    .....    .#.#.    #.#.#
	.#.#.    #.#.#    .....    .....
***/

void image_5x5_filter(SBitmap* bmp, genann* nnet) {
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* erased = new SBitmap(bmp->width(), bmp->height());
	NOT_NULL_OR_RETURN_VOID(ret);
	NOT_NULL_OR_RETURN_VOID(erased);
	ret->clear();

	std::cout << "Applying neural net filter -- first pass!\n";
	erased->clear();
#define ERASE_FILL(p1, p2)	erased->rect_fill(SPoint(p1, POINT_PERCENT, p2, POINT_PERCENT), SPoint(p1 + 20, POINT_PERCENT, p2 + 20, POINT_PERCENT), C_WHITE);
	ERASE_FILL(20, 0);
	ERASE_FILL(60, 0);
	ERASE_FILL(0, 40);
	ERASE_FILL(40, 40);
	ERASE_FILL(80, 40);
	ERASE_FILL(20, 80);
	ERASE_FILL(60, 80);
	while (predict_pass_from_nnet_missing(bmp, erased, ret, nnet, 2));

	std::cout << "Applying neural net filter -- second pass!\n";
	erased->clear();
	ERASE_FILL(0, 0);
	ERASE_FILL(40, 0);
	ERASE_FILL(80, 0);
	ERASE_FILL(20, 40);
	ERASE_FILL(60, 40);
	ERASE_FILL(0, 80);
	ERASE_FILL(40, 80);
	ERASE_FILL(80, 80);
	while (predict_pass_from_nnet_missing(bmp, erased, ret, nnet, 2));

	std::cout << "Applying neural net filter -- third pass!\n";
	erased->clear();
	ERASE_FILL(20, 20);
	ERASE_FILL(60, 20);
	ERASE_FILL(20, 60);
	ERASE_FILL(60, 60);
	while (predict_pass_from_nnet_missing(bmp, erased, ret, nnet, 2));

	std::cout << "Applying neural net filter -- fourth pass!\n";
	erased->clear();
	ERASE_FILL(0, 20);
	ERASE_FILL(40, 20);
	ERASE_FILL(80, 20);
	ERASE_FILL(0, 60);
	ERASE_FILL(40, 60);
	ERASE_FILL(80, 60);
	while (predict_pass_from_nnet_missing(bmp, erased, ret, nnet, 2));

	ret->save_bmp("filter.png");
	std::cout << "Filtered image saved to filter.png.\n";
}


#else  // !NNET_3CHANNELS
bool predict_channel_from_nnet_missing(SBitmap* bin, SBitmap* berase, SBitmap* bout, genann* nnet, u32 channel, u32 min_predict) {
	static PredictAccum pa;
	static u32 last_channel = 0;
	static u32 pass = 0;
	bool ret = false;
	double in[NNET_INPUTS];
	double const* out;
	int x, y, dx, dy, ds;
	u32 ci, co, ce = 0;

	if (last_channel != channel) {
		pa.reset();
		last_channel = channel;
		pass = 1;
	} else {
		++pass;
	}
	std::cout << "Pass " << pass << "... ";

	for (y = 5; y < bin->height() - 5; y += PREDICT_PIXEL_STEP) {
		for (x = 5; x < bin->width() - 5; x += PREDICT_PIXEL_STEP) {
			bool any_erased = false;
			for (dy = -6; dy <= 6 && !any_erased; ++dy) {
				for (dx = -6; dx <= 6 && !any_erased; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds > 36 || ds < 26)
						continue;
					if (!pixel_ok(berase, x + dx, y + dy))
						continue;
					if (berase->get_red(x + dx, y + dy) != 0)
						any_erased = true;
				}
			}
			if (!any_erased)
				continue;
			/* Some pixel on the 6-perimeter is erased. Let's try and predict it. */
			ci = 0;
			for (dy = -5; dy <= 5; ++dy) {
				for (dx = -5; dx <= 5; ++dx) {
					if (berase->get_red(x + dx, y + dy) != 0)
						continue;
					ds = (dy * dy) + (dx * dx);
					if (ds <= 25) {
						/* Within 5 pixels, one of the inputs. */
						in[ci++] = channel_intensity(bin, channel, x + dx, y + dy);
					}
				}
			}
			if (ci < NNET_INPUTS) {
				/* Not enough valid inputs, must have some erased. Skip. */
				continue;
			}
			/* Inputs filled, now calculate outputs and populate predictions for 6-perimeter. */
			co = 0;
			ship_assert(ci == NNET_INPUTS);
			out = genann_run(nnet, in);
			for (dy = -6; dy <= 6; ++dy) {
				for (dx = -6; dx <= 6; ++dx) {
					ds = (dy * dy) + (dx * dx);
					if (ds <= 36 && ds > 25) {
						/* Part of the 6-perimeter. */
						pa.add_prediction(x + dx, y + dy, out[co++]);
					}
				}
			}
			ship_assert(co == NNET_OUTPUTS);
		}
	}

	/* Now fill in any erased pixel with at least min_predict predictions. */
	for (y = 0; y < bin->height(); ++y) {
		for (x = 0; x < bin->width(); ++x) {
			if (berase->get_red(x, y) == 0) {
				/* Not erased. If this is the first pass, it's OK to set the pixel.
				   Successive passes shouldn't do it because we change berase. */
				if (1 == pass) {
					switch (channel) {
					case CHANNEL_RED:
						bout->set_red(x, y, bin->get_red(x, y));
						break;
					case CHANNEL_BLUE:
						bout->set_blue(x, y, bin->get_blue(x, y));
						break;
					case CHANNEL_GREEN:
						bout->set_green(x, y, bin->get_green(x, y));
						break;
					}
				}
				continue;
			}
			++ce;
			if (pa.get_num_predictions(x, y) < min_predict)
				continue;
			double v = pa.get_avg_prediction(x, y);
			v = CLAMP(v, 0.0, 1.0);
			v *= 255.0;
			v = floor(v + 0.5);
			switch (channel) {
			case CHANNEL_RED:
				bout->set_red(x, y, (u32)v);
				bin->set_red(x, y, (u32)v);
				break;
			case CHANNEL_BLUE:
				bout->set_blue(x, y, (u32)v);
				bin->set_blue(x, y, (u32)v);
				break;
			case CHANNEL_GREEN:
				bout->set_green(x, y, (u32)v);
				bin->set_green(x, y, (u32)v);
				break;
			}
			/* Mark this pixel as no longer erased for this channel. */
			berase->put_pixel(x, y, C_BLACK);
			ret = true;
			--ce;
		}
	}

	std::cout << "(" << ce << " erased.) \r";
	if (!ret)
		std::cout << std::endl;

	return ret;
}

SBitmap* predict_from_nnets_missing(SBitmap* bmp, genann* red, genann* green, genann* blue, EraseType et) {
	SBitmap* orig = new SBitmap(bmp->width(), bmp->height());
	SBitmap* ret = new SBitmap(bmp->width(), bmp->height());
	SBitmap* erased = new SBitmap(bmp->width(), bmp->height());
	SBitmap* e1 = new SBitmap(bmp->width(), bmp->height());
	u32 min_pred = 4;
	NOT_NULL_OR_RETURN(ret, ret);
	NOT_NULL_OR_RETURN(erased, erased);
	ret->clear();
	bmp->blit(orig, SPoint(0, 0));

	erase_type(e1, et);
	if (et == ERASE_BANDS)
		min_pred = 2;

	std::cout << "Predicting red channel.\n";
	e1->blit(erased, SPoint(0, 0));
	while (predict_channel_from_nnet_missing(bmp, erased, ret, red, CHANNEL_RED, min_pred));
	std::cout << "Predicting green channel.\n";
	e1->blit(erased, SPoint(0, 0));
	while (predict_channel_from_nnet_missing(bmp, erased, ret, green, CHANNEL_GREEN, min_pred));
	std::cout << "Predicting blue channel.\n";
	e1->blit(erased, SPoint(0, 0));
	while (predict_channel_from_nnet_missing(bmp, erased, ret, blue, CHANNEL_BLUE, min_pred));

	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			if (e1->get_red(x, y) != 0)
				bmp->put_pixel(x, y, C_BLACK);
		}
	}
	std::cout << "Error between original and erased   : " << error_bmp(orig, bmp) << std::endl;
	std::cout << "Error between original and avg.pixel: " << error_bmp_avg(orig, e1) << std::endl;
	std::cout << "Error between original and lastpixel: " << error_bmp_last_pixel(orig, e1) << std::endl;
	std::cout << "Error between original and predicted: " << error_bmp(orig, ret) << std::endl;
	bmp->save_bmp("predict_in.png");
	std::cout << "The erased bitmap is saved as predict_in.png\n";

	return ret;
}

void predict_from_random(genann* red, genann* green, genann* blue) {
	SBitmap* rbmp = new SBitmap(1024, 1024);
	for (int y = 0; y < 1024; ++y) {
		for (int x = 0; x < 1024; ++x) {
			RGBColor c = (RGBColor)RandU32Range(0, 0xFFFFFF);
			rbmp->put_pixel(x, y, c);
		}
	}
	rbmp->save_bmp("predict_in.png");
	SBitmap* pbmp = predict_from_nnets(rbmp, red, green, blue);
	pbmp->save_bmp("predict.png");
	std::cout << "Predicted bitmap (average of all predictions per pixel) output to predict.png.\n";
}

#endif  // !NNET_3CHANNELS

int main(int argc, char* argv[]) {
#ifdef NNET_3CHANNELS
	genann* nnet;
#else
	genann* red, * green, * blue;
#endif
	bool predict = false;
	bool prandom = false;
	bool iterative = false;
	bool small = false;
	bool flip = false;
	bool halfsize = false;
	bool filter = false;
	EraseType et = ERASE_STATIC;
	if (argc < 2) {
		printf("Usage: nnet [image file] {Operation}\n");
		printf("\tOperations:\n\ttrain\n\tpredict\n\titerative\n\tsmall\n\tsmalli\n\tfast\n\tfasti\n\tstatic\n\tstatic2\n\tstatic3\n\trects\n\tcenter\n\tbands\n\tdice\n\thalfsaw\n\tflip\n\tskiperr\n\tlowpredict\n\thalfsize\n\thalflearn\n\tdoublelearn\n\tfilter\n\tneighbors\n\tallneighbors\nAdd 400 to use the 400-neuron ANN, else the large 1200-neuron ANN will be used.\n");
		return 1;
	}
	if (2 == argc && !stricmp(argv[1], "random")) {
		prandom = true;
	}
	if (3 <= argc)
		predict = stricmp(argv[2], "train");
	for (int ag = 2; (ag + 1) <= argc; ++ag) {
		if (!stricmp(argv[ag], "rects"))
			et = ERASE_RANDOM_RECTS;
		if (!stricmp(argv[ag], "center"))
			et = ERASE_CENTER;
		if (!stricmp(argv[ag], "bands"))
			et = ERASE_BANDS;
		if (!stricmp(argv[ag], "static2"))
			et = ERASE_STATIC2;
		if (!stricmp(argv[ag], "static3"))
			et = ERASE_STATIC3;
		if (!stricmp(argv[ag], "train"))
			predict = false;
		if (!stricmp(argv[ag], "iterative")) {
			predict = false;
			iterative = true;
			std::cout << "Will perform iterative training with this image.\n";
		}
		if (!stricmp(argv[ag], "smalli")) {
			predict = false;
			small = true;
			iterative = true;
			std::cout << "Will perform iterative training with this image scaled to at most 800 px/side.\n";
		}
		if (!stricmp(argv[ag], "small")) {
			predict = false;
			small = true;
			std::cout << "Will train this image scaled to at most 800 px/side.\n";
		}
		if (!stricmp(argv[ag], "dice"))
			et = ERASE_DICE;
#ifdef NNET_3CHANNELS
		if (!stricmp(argv[ag], "400")) {
			std::cout << "Will use the 400-neuron image ANN.\n";
			neurons = 400;
		}	
#endif
		if (!stricmp(argv[ag], "fast")) {
			fast = true;
			small = true;
			predict = false;
			std::cout << "Will train image using x,y mod 4 for training set or validation set membership.\n";
		}
		if (!stricmp(argv[ag], "fasti")) {
			fast = true;
			small = true;
			predict = false;
			iterative = true;
			std::cout << "Will train image iteratively using x,y mod 4 for training set or validation set membership.\n";
		}
		if (!stricmp(argv[ag], "skiperr")) {
			skiperr = true;
			std::cout << "Will skip the initial error calculation.\n";
		}
		if (!stricmp(argv[ag], "flip")) {
			flip = true;
			std::cout << "Will flip input bitmap horizontally.\n";
		}
		if (!stricmp(argv[ag], "halfsize")) {
			halfsize = true;
			std::cout << "Will train on half size image, or resize input image.\n";
		}
		if (!stricmp(argv[ag], "lowpredict")) {
			low_min_predict = true;
			std::cout << "Will use a minimum of 2 predictions for all pixels in output.\n";
		}
		if (!stricmp(argv[ag], "halflearn")) {
			half_learning_rate = true;
			std::cout << "Will use a halved learning rate.\n";
		}
		if (!stricmp(argv[ag], "doublelearn")) {
			double_learning_rate = true;
			std::cout << "Will use a doubled learning rate.\n";
		}
		if (!stricmp(argv[ag], "filter")) {
			filter = true;
			predict = false;
			std::cout << "Will perform a 5x5 filter on the image.\n";
		}
		if (!stricmp(argv[ag], "neighbors")) {
			use_neighbors = true;
			std::cout << "Will permit up to " << max_neighbors << " neighboring pixels to be used in nnet inputs.\n";
		}
		if (!stricmp(argv[ag], "allneighbors")) {
			use_neighbors = true;
			MAX_PASS_NEIGHBORS = 999999;
			max_neighbors = 1000;
			std::cout << "Mister Rogers' Neighborhood activated. The world is our playground.\n";
		}
		if (!stricmp(argv[ag], "halfsaw")) {
			et = ERASE_HALFSAW;
		}
	}

	SBitmap* bmp = nullptr;
	if (!prandom) {
		bmp = SBitmap::load_bmp(argv[1]);
		if (is_null(bmp)) {
			printf("Error loading bitmap %s.\n", argv[1]);
			return 2;
		}
		if (predict && halfsize) {
			bmp = bmp->scale_rational(1, 2);
		}
		if (small) {
			u32 big_side = std::max(bmp->width(), bmp->height());
			if (big_side > 800)
				bmp = bmp->scale_rational(800, big_side);
		}
		if (flip) {
			bmp->flip_horiz();
		}
	}

#ifdef NNET_3CHANNELS
	std::cout << "Loading the neural network...\n";
	if (FileExists(nnet_name())) {
		nnet = read_from_file(nnet_name());
	} else {
		std::cout << "(Neural net doesn't exist, creating.)\n";
		nnet = genann_init(NNET_INPUTS * 3, NNET_HIDDEN_LAYERS, NNET_HIDDEN_NEURONS, NNET_OUTPUTS * 3);
	}

	if (predict) {
		std::cout << "Predicting from input bitmap...\n";
		SBitmap* predicted = predict_from_nnets_missing(bmp, nnet, et);
		if (is_null(predicted)) {
			std::cout << "Error predicting bitmap!\n";
			return 3;
		}
		predicted->save_bmp("predict.png");
		std::cout << "Predicted bitmap (average of all predictions per pixel) output to predict.png.\n";
	} else if (filter) {
		image_5x5_filter(bmp, nnet);
	} else {
		std::cout << "Training the neural network... (image dimensions " << bmp->width() << " x " << bmp->height() << ", " << bmp->width() * bmp->height() << " pixels)\n";
		u32 niter = train_on_image(nnet, bmp, iterative);

		if (halfsize) {
			// Reload the neural network from file, so I know I got the best-fit version.
			nnet = read_from_file(nnet_name());

			/* Now, train on a half-size version of the same image. */
			SBitmap* halfsize = bmp->scale_rational(1, 2);
			if (niter > 0 && halfsize->height() > 40 && halfsize->width() > 40) {
				halfsize->save_bmp("halfsize.png");
				std::cout << "Now training a half-size version of the image.\n";
				train_on_image(nnet, halfsize, false);
				nnet = read_from_file(nnet_name());
				halfsize->flip_horiz();
				std::cout << "Now training a horizontally flipped half-size version.\n";
				train_on_image(nnet, halfsize, false);
			}
		}
	}
#else   // !NNET_3CHANNELS

	std::cout << "Loading the neural networks...\n";
	if (FileExists(NNET_RED)) {
		red = read_from_file(NNET_RED);
	} else {
		std::cout << "(Red doesn't exist, creating.)\n";
		red = genann_init(NNET_INPUTS, NNET_HIDDEN_LAYERS, NNET_HIDDEN_NEURONS, NNET_OUTPUTS);
	}
	if (FileExists(NNET_GREEN)) {
		green  = read_from_file(NNET_GREEN);
	} else {
		std::cout << "(Green doesn't exist, creating.)\n";
		green = genann_init(NNET_INPUTS, NNET_HIDDEN_LAYERS, NNET_HIDDEN_NEURONS, NNET_OUTPUTS);
	}
	if (FileExists(NNET_BLUE)) {
		blue = read_from_file(NNET_BLUE);
	} else {
		std::cout << "(Blue doesn't exist, creating.)\n";
		blue = genann_init(NNET_INPUTS, NNET_HIDDEN_LAYERS, NNET_HIDDEN_NEURONS, NNET_OUTPUTS);
	}

	if (predict) {
		std::cout << "Predicting from input bitmap...\n";
		SBitmap* predicted = predict_from_nnets_missing(bmp, red, green, blue, et);
		if (is_null(predicted)) {
			std::cout << "Error predicting bitmap!\n";
			return 3;
		}
		predicted->save_bmp("predict.png");
		std::cout << "Predicted bitmap (average of all predictions per pixel) output to predict.png.\n";
	} else if (prandom) {
		std::cout << "Predicting from random bitmap...\n";
		predict_from_random(red, green, blue);
		return 0;
	} else {
		std::cout << "Training the red neural network...\n";
		train_on_image(red, bmp, CHANNEL_RED, iterative);
		std::cout << "Training the green neural network...\n";
		train_on_image(green, bmp, CHANNEL_GREEN, iterative);
		std::cout << "Training the blue neural network...\n";
		train_on_image(blue, bmp, CHANNEL_BLUE, iterative);
	}
#endif  // !NNET_3CHANNELS

	return 0;	
}
