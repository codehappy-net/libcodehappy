/***

	quant.cpp
	
	Quantize an image from the command line.
	
	C. M. Street
	
***/
#define CODEHAPPY_NATIVE	
#include <libcodehappy.h>

double variance_rect(SBitmap* bmp, int x1, int x2, int y1, int y2, double* mean, RGBColor* mean_col) {
	std::vector<double> i;
	int r = 0, g = 0, b = 0;
	RGBColor c;
	for (int y = y1; y <= y2; ++y) {
		for (int x = x1; x <= x2; ++x) {
			c = bmp->get_pixel(x, y);
			i.push_back(double(rgb_intensity(c)));
			r += RGB_RED(c);
			g += RGB_GREEN(c);
			b += RGB_BLUE(c);
		}
	}
	if (!is_null(mean_col)) {
		r /= ((x2 - x1 + 1) * (y2 - y1 + 1));
		g /= ((x2 - x1 + 1) * (y2 - y1 + 1));
		b /= ((x2 - x1 + 1) * (y2 - y1 + 1));
		*mean_col = RGB_NO_CHECK(r, g, b);
	}
	return standard_deviation(i, mean);
}

/* Add the signature to the picture. */
void sign_picture(SBitmap* bmp) {
	SBitmap* sig = SBitmap::load_bmp("sig.png");
	if (is_null(sig)) {
		codehappy_cerr << "Unable to load the signature image.\n";
		return;
	}
	sig->scale_rational_and_replace(3, 4);
	// set the alpha channel for a transparent blit.
	for (int y = 0; y < sig->height(); ++y) {
		for (int x = 0; x < sig->width(); ++x) {
			u32 intsy = rgb_intensity(sig->get_pixel(x, y));
			sig->set_alpha(x, y, ALPHA_OPAQUE - intsy);
		}
	}
	
	// position the signature in the lower left or lower right, whichever corner has the lower variance in intensity.
	double v1, v2, m1, m2;
	int xs, ys;
	RGBColor c1, c2;

	v1 = variance_rect(bmp, 0, sig->width() - 1, bmp->height() - sig->height(), bmp->height() - 1, &m1, &c1);
	v2 = variance_rect(bmp, bmp->width() - sig->width(), bmp->width() - 1, bmp->height() - sig->height(), bmp->height() - 1, &m2, &c2);
	xs = 0;
	if (v2 < v1) {
		v1 = v2;
		m1 = m2;
		c1 = c2;
		xs = bmp->width() - sig->width();
	}
	ys = bmp->height() - sig->height();

#if 0
	// if the mean intensity is dark, invert the signature image to white-on-black.
	if (m1 < 127.5) {
		sig->negative();
	}
#else
	sig->rect_fill(0, 0, sig->width() - 1, sig->height() - 1, complementary_color(c1));
#endif	

	// apply the signature and return.
	sig->blit_blend(bmp, xs, ys);
	delete sig;
}

int app_main() {
	ArgParse ap;
	std::string path_in, path_out = "output.png";
	SBitmap* bmp, * bmp_q;
	int ncol = 256, noise = 4;
	bool use_noise = false, ai_special = false;

	ap.add_argument("img", type_string, "path to sample to quantize (required)");
	ap.add_argument("out", type_string, "path to output quantized image (default 'output.png')");
	ap.add_argument("ncol", type_int, "number of desired colors in the quantized image (default 256)", &ncol);
	ap.add_argument("noise", type_none, "add some noise to the image before quantizing", &use_noise);
	ap.add_argument("noise_mag", type_int, "specify the magnitude of noise (only used iff --noise, default is 4)", &noise);
	ap.add_argument("ai", type_none, "AI special: add signature and quantize", &ai_special);
	ap.ensure_args(argc, argv);

	if (ap.flag_present("out")) {
		path_out = ap.value_str("out");
	}
	if (!ap.flag_present("img")) {
		codehappy_cerr << "Error: no input image specified with --img argument.\n";
		return 1;
	}
	path_in = ap.value_str("img");

	bmp = SBitmap::load_bmp(path_in);
	if (is_null(bmp)) {
		codehappy_cerr << "Error: unable to open file '" << path_in << "' as an image file.\n";
		return 2;
	}

	if (ai_special) {
		std::cout << "AI special: quantizing to 256 colors with noise = 8 and signing the image in the corner.\n";
		noise = 8;
		ncol = 256;
	}

	if (use_noise) {
		std::cout << "Adding noise of magnitude " << noise << " to the image.\n";
		bmp->noise_rgb(noise);
	}

	if (ai_special) {
		sign_picture(bmp);
	}

	bmp_q = quantize_bmp_greedy(bmp, (u32) ncol, nullptr, dither_floyd_steinberg, colorspace_rgb);
	if (has_extension(path_out, "gif")) {
		bmp_q->save_bmp(path_out);
	} else {
		bmp_q->blit(bmp);
		bmp->save_bmp(path_out);
	}
	std::cout << "Quantized image output to " << path_out << ".\n";

	return 0;
}
