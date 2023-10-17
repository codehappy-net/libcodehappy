/***

	optn.cpp

	Use the GeneticOptimizer to choose the best combination of neural net models
	to use to complete an image.

	Copyright (c) 2022 C. M. Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

#define	NNET_PATH_1	"nnets/fivek.rfn.checkpoint21"
#define	NNET_PATH_2	"nnets/chungus.rfn.checkpoint15"
#define	NNET_PATH_3	"nnets/doublek.rfn.checkpoint5"

struct AppData {
	ImgNNet nnets[3];
	SBitmap* borig;
	SBitmap* berase;
	SBitmap* bpaint;
	volatile PredictWindow pw;
	PredictAccum pa[3];
};

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

double img_weights(double* in, void* user_data) {
	AppData* ad = (AppData*)user_data;
	RGBOut ov, wv;
	u32 te = 0, ce = 0;

	for (int y = 0; y < ad->berase->height(); ++y) {
		for (int x = 0; x < ad->berase->width(); ++x) {
			if (ad->berase->get_red(x, y) > 0) {
				int r, g, b;
				if (ad->pa[0].get_num_predictions(x, y) < 1 ||
				    ad->pa[1].get_num_predictions(x, y) < 1 ||
				    ad->pa[2].get_num_predictions(x, y) < 1)
					continue;
				ad->pa[0].get_avg_prediction(x, y, ov);
				wv = ov * in[0];
				ad->pa[1].get_avg_prediction(x, y, ov);
				wv += (ov * in[1]);
				ad->pa[2].get_avg_prediction(x, y, ov);
				wv += (ov * in[2]);
				r = (int)floor(wv.r * 255. + 0.5);
				g = (int)floor(wv.g * 255. + 0.5);
				b = (int)floor(wv.b * 255. + 0.5);
				te += color_distance(MAKE_RGB(r, g, b), ad->borig->get_pixel(x, y));
				ce += 3;
			}
		}
	}
	if (0 == ce)
		return 0.;
	return double(te) / double(ce);
}

void paint_image(double* w, AppData* ad) {
	RGBOut ov, wv;

	ad->bpaint->clear();
	for (int y = 0; y < ad->berase->height(); ++y) {
		for (int x = 0; x < ad->berase->width(); ++x) {
			if (ad->berase->get_red(x, y) > 0) {
				int r, g, b;
				if (ad->pa[0].get_num_predictions(x, y) < 1 ||
				    ad->pa[1].get_num_predictions(x, y) < 1 ||
				    ad->pa[2].get_num_predictions(x, y) < 1)
					continue;
				ad->pa[0].get_avg_prediction(x, y, ov);
				wv = ov * w[0];
				ad->pa[1].get_avg_prediction(x, y, ov);
				wv += (ov * w[1]);
				ad->pa[2].get_avg_prediction(x, y, ov);
				wv += (ov * w[2]);
				r = (int)floor(wv.r * 255. + 0.5);
				g = (int)floor(wv.g * 255. + 0.5);
				b = (int)floor(wv.b * 255. + 0.5);
				ad->bpaint->put_pixel(x, y, MAKE_RGB(r, g, b));
			} else {
				ad->bpaint->put_pixel(x, y, ad->borig->get_pixel(x, y));
			}
		}
	}
}

int main(int argc, char* argv[]) {
	AppData ad;

	if (argc < 2) {
		std::cout << "Usage: optn [image file]\n";
		return 1;
	}

	ad.borig = SBitmap::load_bmp(argv[1]);
	if (is_null(ad.borig)) {
		std::cout << "Image load '" << argv[1] << "' failed!\n";
		return 2;
	} else {
		std::cout << "Image load '" << argv[1] << "' succeeded (dimensions " << ad.borig->width() << " x " << ad.borig->height() << ")\n";
	}
	ad.berase = ImgNNet::get_erased_bmp(ad.borig, ERASE_CENTER);
	ad.bpaint = new SBitmap(ad.borig->width(), ad.borig->height());

	std::cout << "Loading inpainting neural networks...\n";
	std::string nnet_paths[3];
	find_recent_checkpoints(nnet_paths[0], nnet_paths[1], nnet_paths[2]);
	ad.nnets[0].read_from_file(nnet_paths[0].c_str());
	ad.nnets[1].read_from_file(nnet_paths[1].c_str());
	ad.nnets[2].read_from_file(nnet_paths[2].c_str());
	ad.nnets[0].set_predict_window((PredictWindow*)&ad.pw);
	ad.nnets[1].set_predict_window((PredictWindow*)&ad.pw);
	ad.nnets[2].set_predict_window((PredictWindow*)&ad.pw);
	ad.nnets[0].set_max_threads(8);
	ad.nnets[1].set_max_threads(8);
	ad.nnets[2].set_max_threads(8);

	std::cout << "Making first prediction...\n";
	ad.pw.pa = &ad.pa[0];
	ad.nnets[0].predict_from_missing(ad.borig, ad.berase);
	std::cout << "Making second prediction...\n";
	ad.pw.pa = &ad.pa[1];
	ad.nnets[1].predict_from_missing(ad.borig, ad.berase);
	std::cout << "Making third prediction...\n";
	ad.pw.pa = &ad.pa[2];
	ad.nnets[2].predict_from_missing(ad.borig, ad.berase);

	std::cout << "Attempting to optimize weights for best image completion...\n";
	GeneticOptimizer go(3, img_weights, (void *)&ad);
	go.set_lobound(0.);
	go.set_hibound(1.);
	go.set_constraint_sum_weights(1.0);
	go.verbose(true);
	double* opt = go.optimize_min(0.01);
	std::cout << "Weights: " << opt[0] << " fivek, " << opt[1] << " chungus, " << opt[2] << " doublek.\n";

	paint_image(opt, &ad);
	ad.bpaint->save_bmp("paint.png");
	std::cout << "Weighted completion saved to 'paint.png'.\n";

	return 0;
}

/* end optn.cpp */