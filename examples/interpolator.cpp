/***

	interpolator.cpp

	An app that will create LDM generations for a supplied set of prompts, which the
	user can curate, then interpolates between the selected generations.

	June 2024, Chris Street

***/
#define CODEHAPPY_NATIVE_SDL
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

#ifdef CODEHAPPY_WINDOWS
#define APP_WIDTH	800
#define APP_HEIGHT	600
#else
#define APP_WIDTH	1200
#define APP_HEIGHT	900
#endif

using json = nlohmann::json;

struct GenInfo {
	std::string prompt;
	i64 seed;
};

struct AppData {
	AppData();
	std::vector<std::string> prompts;
	std::vector<GenInfo> interp_data;
	GenInfo cur_data;
	int iprompt;
	int w, h;
	int frames;
	SBitmap* orig, * scaled;
	std::string neg_prompt;
	double cfg_scale;
};

AppData::AppData() {
	w = 512;
	h = 512;
	orig = nullptr;
	scaled = nullptr;
	iprompt = 0;
	cfg_scale = 6.5;
	frames = 40;
}

static AppData ad;

bool selection_done() {
	return ad.iprompt >= ad.prompts.size();
}

void invalidate_image(bool advance) {
	SBitmap * o, * s;

	o = ad.orig;
	s = ad.scaled;
	if (advance)
		++ad.iprompt;
	ad.orig = nullptr;
	ad.scaled = nullptr;
	if (o != nullptr)
		delete o;
	if (s != nullptr)
		delete s;
}

void set_image(SBitmap* orig) {
	SBitmap* sc;

	ship_assert(orig != nullptr);
	sc = orig->copy();
	if (sc->height() > APP_HEIGHT)
		sc->resize_and_replace(0, APP_HEIGHT);
	if (sc->width() > APP_WIDTH)
		sc->resize_and_replace(APP_WIDTH, 0);

	ad.scaled = sc;
	ad.orig = orig;
}

void read_prompts(const std::string& filename) {
	std::ifstream i;
	std::string line;

	i.open(filename);
	forever {
		std::getline(i, line);
		if (i.eof())
			break;
		if (!line.empty())
			ad.prompts.push_back(line);
	}
	i.close();
	i.clear();
}

i64 rng_seed() {
	i64 ret;
	ret = RandI64();
	if (ret < 1)
		ret = -ret;
	return ret;
}

void save_json(const std::string& path) {
	json data_json;
	std::ofstream o;

	for (int i = 0; i < ad.interp_data.size(); ++i) {
		std::string key = std::to_string(i);
		data_json[key]["prompt"] = ad.interp_data[i].prompt;
		data_json[key]["seed"] = ad.interp_data[i].seed;
		data_json[key]["neg_prompt"] = ad.neg_prompt;
		data_json[key]["cfg"] = ad.cfg_scale;
		data_json[key]["w"] = ad.w;
		data_json[key]["h"] = ad.h;
	}

	o.open(path);
	o << data_json.dump(2, ' ', false, json::error_handler_t::replace) << std::endl;
	o.close();
	o.clear();
}

int frame_number_start() {
	int i;
	for (i = 0; i < 9000; ++i) {
		char fname[32];
		sprintf(fname, "frame%04d.png", i);
		if (!FileExists(fname))
			break;
	}
	return i;
}

void generation_thread() {
	forever {
		if (selection_done())
			break;
		if (ad.orig != nullptr) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		SBitmap ** bmp_batch;
		ad.cur_data.prompt = ad.prompts[ad.iprompt];
		ad.cur_data.seed = rng_seed();
		bmp_batch = sd_server.txt2img(ad.cur_data.prompt, ad.neg_prompt, ad.w, ad.h, ad.cfg_scale, ad.cur_data.seed);
		set_image(bmp_batch[0]);
		delete bmp_batch;
	}
}

void main_loop(Display* display, void* user_data) {
	static KeyLast kl(display);
	static int glevel = 255, gdir = -2;
	char buf[32];

	display->bitmap()->clear();

	if (selection_done()) {
		codehappy_close_window();
		return;
	}

	if (nullptr == ad.orig || selection_done()) {
		// we need to generate a new image
		display->bitmap()->render_text("Generating...", &font_swansea_bold, RGB_NO_CHECK(glevel, glevel, glevel), 80, CENTERED_BOTH);
		glevel += gdir;
		if (glevel < 0) {
			gdir = -gdir;
			glevel = 1;
		}
		if (glevel > 255) {
			gdir = -gdir;
			glevel = 254;
		}
		kl.save(display);
		return;
	}

	ship_assert(ad.scaled != nullptr);
	const int x1 = (APP_WIDTH - ad.scaled->width()) / 2;
	const int y1 = (APP_HEIGHT - ad.scaled->height()) / 2;
	ad.scaled->blit(display->bitmap(), SPoint(x1, y1));

	sprintf(buf, "%d/%d", ad.iprompt + 1, (int) ad.prompts.size());
	display->bitmap()->render_text(buf, &font_swansea_bold, H_ORANGE, 40, ALIGN_TOP | ALIGN_LEFT);
	display->bitmap()->render_text(ad.prompts[ad.iprompt].c_str(), &font_swansea_bold, H_ORANGE, 16, ALIGN_BOTTOM | ALIGN_LEFT);

	if (kl.now_down('y') || kl.now_down('Y')) {
		// accept the image, move to the next in the prompt list
		ad.interp_data.push_back(ad.cur_data);

		invalidate_image(true);
	}
	if (kl.now_down('n') || kl.now_down('N')) {
		// reject the image, try again on the same prompt
		invalidate_image(false);
	}
	if (kl.now_down('s') || kl.now_down('S')) {
		// reject the image, and skip this prompt; continue to the next prompt
		invalidate_image(true);
	}
	if (kl.now_down('x') || kl.now_down('X')) {
		// stop the selection phase now; add no further candidate generations.
		ad.iprompt = ad.prompts.size();
		invalidate_image(false);
	}
	if (kl.now_down('z') || kl.now_down('Z')) {
		// accept the image, and stop the selection phase.
		ad.interp_data.push_back(ad.cur_data);
		ad.iprompt = ad.prompts.size();
		invalidate_image(false);
	}

	kl.save(display);
}

int app_main() {
	ArgParse ap;
	std::string prompt_list, model, vae, out_json, single_prompt;
	int steps = 30;

	ap.add_argument("prompts", type_string, "a text file containing the desired prompts, one to each line");
	ap.add_argument("w", type_int, "Width in pixels (default is 512)", &ad.w);
	ap.add_argument("h", type_int, "Height in pixels (default is 512)", &ad.h);
	ap.add_argument("neg-prompt", type_string, "the negative prompt (if desired) to use in interpolation");
	ap.add_argument("model", type_string, "Path to Stable Diffusion model");
	ap.add_argument("vae", type_string, "Path to VAE (if a separate VAE is desired)");
	ap.add_argument("steps", type_int, "Number of diffusion iterations to perform per generation (default is 30)", &steps);
	ap.add_argument("cfg", type_double, "Classifier-free guidance scale (default is 6.5)", &ad.cfg_scale);
	ap.add_argument("frames", type_int, "Number of interpolation frames between selected generations (default is 40)", &ad.frames);
	ap.add_argument("out-json", type_string, "Path to a .JSON file to write the generation parameters.");
	ap.add_argument("single-prompt", type_string, "A single prompt that you want to interpolate between many times");
	ap.ensure_args(argc, argv);
	ap.value_str("prompts", prompt_list);
	ap.value_str("neg-prompt", ad.neg_prompt);
	ap.value_str("model", model);
	ap.value_str("vae", vae);
	ap.value_str("out-json", out_json);
	ap.value_str("single-prompt", single_prompt);

	sd_server.set_steps((u32) steps);
	if (!prompt_list.empty()) {
		read_prompts(prompt_list);
	}
	if (!single_prompt.empty()) {
		for (int i = 0; i < 100; ++i)
			ad.prompts.push_back(single_prompt);
	}

	if (ad.prompts.size() < 2) {
		codehappy_cerr << "Please supply a file containing at least two prompts.\n";
		return 1;
	}
	if (!sd_server.load_from_file(model, vae)) {
		codehappy_cerr << "Error loading model file " << model << "!\n";
		return 2;
	}

	std::cout << "Controls:\n";
	std::cout << "[Y] accept the image, move to the next in the prompt list\n";
	std::cout << "[N] reject the image, try again on the same prompt\n";
	std::cout << "[S] reject the image, and skip this prompt\n";
	std::cout << "[X] stop the selection phase now; add no further candidate generations\n";
	std::cout << "[Z] accept the current image and stop the selection phase now\n";

	std::thread* th = new std::thread(generation_thread);

	codehappy_main(main_loop, nullptr, APP_WIDTH, APP_HEIGHT);

	if (!out_json.empty()) {
		save_json(out_json);
	}

	/*** now perform the interpolation ***/
	std::cout << ad.interp_data.size() << " accepted generations.\n";
	for (int i = 1; i < ad.interp_data.size(); ++i) {
		SBitmap ** ret;

		std::cout << "Interpolating index " << i << " to " << i + 1 << "...\n";

		ret = sd_server.txt2img_slerp(ad.frames, ad.interp_data[i - 1].prompt, ad.interp_data[i].prompt, ad.neg_prompt, ad.neg_prompt, 
						ad.interp_data[i - 1].seed, ad.interp_data[i].seed, (u32) ad.w, (u32) ad.h, ad.cfg_scale);
		if (is_null(ret)) {
			codehappy_cerr << "no images returned from SDServer::txt2img_slerp()?\n";
			return 3;
		}

		std::cout << "Writing images to frame%04d.png...\n";
		int fs = frame_number_start();
		for (int i = 0; i < ad.frames; ++i) {
			char fname[32];
			sprintf(fname, "frame%04d.png", i + fs);
			ret[i]->save_bmp(fname);
		}
		free_batch_bmps(ret, ad.frames);
	}

	return 0;
}

/*** end interpolator.cpp ***/
