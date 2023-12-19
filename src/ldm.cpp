/***

	ldm.cpp
	
	Latent diffusion model (LDM) code. The Stable Diffusion server lives
	here, capable of traditional image generation as well as various kinds
	of interpolation, variations, progressive sampling animations, or classifier
	free guidance strength animations.

	Copyright 2023, C. M. Street

***/

const std::string sd_sampler_names[sd_sampler_count] = {
    "Euler ancestral",
    "Euler",
    "Heun",
    "DPM2",
    "DPM++ 2s ancestral",
    "DPM++ 2m",
    "DPM++ 2m v2",
    "LCM",
};

const std::string sd_schedule_names[sd_scheduler_count] = {
    "default",
    "discrete",
    "Karras"
};

#define PUZZLEBOX_URI	"https://www.codehappy.net/puzzlebox/"

static const std::string sd_default_model_names[2] = {
    // based on SD 1.5
    "puzzlebox-v2e14.ckpt",
    // based on SD 2.1
    "puzzlebox-v3e5.ckpt",
};

SDServer sd_server;

static void validate_wh(u32& w, u32& h) {
	/* ensure these values are multiples of 8 */
	if ((w & 7) != 0)
		w += (8 - (w & 7));
	if ((h & 7) != 0)
		h += (8 - (h & 7));
	ship_assert((w != 0) && (h != 0));
}

SDServer::SDServer() {
	sd_model = nullptr;
	nthreads = std::max((int) std::thread::hardware_concurrency() / 2, 1);
	steps = 20;
	last_seed = -1;
	variation_seed = 43;
	sampler = sd_euler;
	scheduler = sd_karras;
}

SDServer::~SDServer() {
	if (sd_model != nullptr) {
		delete sd_model;
		sd_model = nullptr;
	}
}

SBitmap** SDServer::txt2img(const std::string& prompt, const std::string& neg_prompt, u32 w, u32 h, double cfg_scale,
                            i64 rng_seed, double variation_weight, i64* seed_return, int batch_count) {
	std::vector<uint8_t*> output;
	SBitmap** ret = nullptr;
	i64 seed;

	validate_wh(w, h);
	if (!ensure_model())
		return ret;

	if (rng_seed != -1) {
		seed = rng_seed;
	} else {
		seed = RandI64();
		if (seed < 0)
			seed = -seed;
	}

	if (variation_weight == 0.0) {
		output = sd_model->txt2img(prompt, neg_prompt, cfg_scale, w, h, (SampleMethod) sampler, steps, seed, batch_count);
		ret = bmps_from_vecu8(output, w, h);
		if (!is_null(ret)) {
			last_seed = seed;
			if (!is_null(seed_return)) {
				*seed_return = seed;
			}
		}
	} else {
		// TBI
	}

	return ret;
}

SBitmap* SDServer::txt2img_slerp(double v, const std::string& prompt_1, const std::string& prompt_2, const std::string& neg_prompt_1,
                                 const std::string& neg_prompt_2, i64 rng_seed_1, i64 rng_seed_2, u32 w, u32 h, double cfg_scale,
                                 i64* seed_return_1, i64* seed_return_2) {
	// TBI
	return nullptr;
}

void SDServer::txt2img_slerp(std::vector<SBitmap*>& imgs_out, int n_images, const std::string& prompt_1, const std::string& prompt_2,
                             const std::string& neg_prompt_1, const std::string& neg_prompt_2, i64 rng_seed_1, i64 rng_seed_2,
                             u32 w, u32 h, double cfg_scale, i64* seed_return_1, i64* seed_return_2) {
	int e;
	std::vector<double> vs;
	if (n_images < 1)
		return;
	if (1 == n_images)
		vs.push_back(0.5);
	else {
		for (e = 0; e < n_images; ++e)
			vs.push_back(double(e) / double(n_images - 1));
	}
	for (const double v : vs) {
		imgs_out.push_back(txt2img_slerp(v, prompt_1, prompt_2, neg_prompt_1, neg_prompt_2, rng_seed_1, rng_seed_2, w, h,
		                                 cfg_scale, seed_return_1, seed_return_2));
	}
}

SBitmap** SDServer::img2img(SBitmap* init_img, double img_strength, const std::string& prompt, const std::string& neg_prompt,
                            double cfg_scale, i64 rng_seed, double variation_weight, i64* seed_return) {
	std::vector<uint8_t*> output, init_img_v;
	SBitmap** ret = nullptr;
	i64 seed;
	u32 w, h;

	if (!ensure_model())
		return ret;
	NOT_NULL_OR_RETURN(init_img, nullptr);
	if (!legal_img2img(init_img)) {
		codehappy_cerr << "invalid input image for img2img -- dimensions must be multiples of 64\n";
		return ret;
	}
	w = init_img->width();
	h = init_img->height();

	if (rng_seed != -1) {
		seed = rng_seed;
	} else {
		seed = RandI64();
		if (seed < 0)
			seed = -seed;
	}

	vecu8_from_bmp(init_img_v, &init_img, 1);

	output = sd_model->img2img(init_img_v[0], prompt, neg_prompt, cfg_scale, w, h, (SampleMethod) sampler, steps, img_strength, seed);
	ret = bmps_from_vecu8(output, w, h);
	if (!is_null(ret)) {
		last_seed = seed;
		if (!is_null(seed_return)) {
			*seed_return = seed;
		}
	}

	return ret;
}

bool SDServer::ensure_model() {
	if (sd_model != nullptr)
		return true;
	return load_default_model();
}

static ggml_type wtype_from_path(const std::string& path, ggml_type wtype) {
	const char* w = path.c_str();
	ggml_type ret = GGML_TYPE_UNK;

	if (wtype != GGML_TYPE_UNK)
		return wtype;

	if (!__stristr(w, "f32"))
		ret = GGML_TYPE_F32;
	if (!__stristr(w, "f16"))
		ret = GGML_TYPE_F16;
	if (!__stristr(w, "q4_0"))
		ret = GGML_TYPE_Q4_0;
	if (!__stristr(w, "q4_1"))
		ret = GGML_TYPE_Q4_1;
	if (!__stristr(w, "q5_0"))
		ret = GGML_TYPE_Q5_0;
	if (!__stristr(w, "q5_1"))
		ret = GGML_TYPE_Q5_1;
	if (!__stristr(w, "q8_0"))
		ret = GGML_TYPE_Q8_0;

	if (ret == GGML_TYPE_UNK) {
		// unknown from the file name, let's take a guess from the file size.
		u64 fl = flength_64(path.c_str());
		ret = GGML_TYPE_F32;
		if (fl < 4000000000ULL)
			ret = GGML_TYPE_F16;
		if (fl < 2000000000ULL)
			ret = GGML_TYPE_Q8_0;
		if (fl < 1300000000ULL)
			ret = GGML_TYPE_Q4_0;
	}

	return ret;
}

bool SDServer::load_from_file(const std::string& path, ggml_type wtype) {
	if (sd_model != nullptr) {
		delete sd_model;
	}
	model_path = path;
	sd_model = new StableDiffusion((int) nthreads);
	return sd_model->load_from_file(path, "", wtype_from_path(path, wtype), (Schedule) scheduler);
}

bool SDServer::load_default_model(int sd_version, bool download_if_missing) {
	std::string URI;
	bool default_exists[2], check_ckpt = false;

	default_exists[0] = FileExists(sd_default_model_names[0]);
	default_exists[1] = FileExists(sd_default_model_names[1]);
	if ((sd_version == 0 && !default_exists[0] && !default_exists[1]) ||
	    (sd_version == 1 && !default_exists[0]) ||
	    (sd_version == 2 && !default_exists[1]))
		check_ckpt = true;

	if (check_ckpt) {
		// look for a .ckpt model in the current directory
		DIR* di = opendir(".");
		dirent* entry;
		while (entry = readdir(di)) {
			if (strstr(entry->d_name, ".ckpt") != nullptr) {
				if (load_from_file(entry->d_name)) {
					closedir(di);
					return true;
				}
			}
		}
		closedir(di);
	}

	switch (sd_version) {
	case 0:
		if (!load_default_model(2, download_if_missing))
			return load_default_model(1, download_if_missing);
		break;
	case 1:
		if (!FileExists(sd_default_model_names[0])) {
			if (!download_if_missing)
				return false;
			URI = PUZZLEBOX_URI;
			URI += sd_default_model_names[0];
			if (!FetchURIToFile(URI, sd_default_model_names[0]))
				return false;
		}
		return load_from_file(sd_default_model_names[0]);
	case 2:
		if (!FileExists(sd_default_model_names[1])) {
			if (!download_if_missing)
				return false;
			URI = PUZZLEBOX_URI;
			URI += sd_default_model_names[1];
			if (!FetchURIToFile(URI, sd_default_model_names[1]))
				return false;
		}
		return load_from_file(sd_default_model_names[1]);
	default:
		codehappy_cerr << "*** Error: unknown default model version: " << sd_version << "?\n";
		return false;
	}

	return true;
}

void SDServer::set_nthreads(u32 nt) {
	bool reload = false;
	if (nt != nthreads && sd_model != nullptr)
		reload = true;
	nthreads = nt;
	if (reload)
		load_from_file(model_path);
}

void SDServer::set_sampler_type(SDSamplerType sampler_type) {
	sampler = sampler_type;
}

void SDServer::set_scheduler_type(SDSchedulerType scheduler_type) {
	scheduler = scheduler_type;
}

void SDServer::set_steps(u32 st) {
	steps = st;
}

void SDServer::set_variation_seed(i64 seed) {
	variation_seed = seed;
}

SBitmap* single_bmp_from_u8array(uint8_t* data, u32 w, u32 h) {
	SBitmap* ret = nullptr;
	u32 i = 0;

	ret = new SBitmap(w, h);

	for (int y = 0; y < (int) h; ++y) {
		for (int x = 0; x < (int) w; ++x) {
			unsigned int r, g, b;
			r = data[i++];
			g = data[i++];
			b = data[i++];
			ret->put_pixel(x, y, RGB_NO_CHECK(r, g, b));
		}
	}

	return ret;
}

uint8_t* u8array_from_bmp(SBitmap* bmp) {
	uint8_t* ret = nullptr;
	ret = new uint8_t [bmp->width() * bmp->height() * 3];
	NOT_NULL_OR_RETURN(ret, ret);
	int i = 0;

	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			RGBColor c = bmp->get_pixel(x, y);
			ret[i++] = (uint8_t) RGB_RED(c);
			ret[i++] = (uint8_t) RGB_GREEN(c);
			ret[i++] = (uint8_t) RGB_BLUE(c);
		}
	}
	
	return ret;
}

SBitmap** bmps_from_vecu8(const std::vector<uint8_t*>& invec, u32 w, u32 h) {
	if (invec.empty())
		return nullptr;
	SBitmap** ret = new SBitmap * [invec.size()];
	NOT_NULL_OR_RETURN(ret, ret);

	for (int i = 0; i < invec.size(); ++i) {
		ret[i] = single_bmp_from_u8array(invec[i], w, h);
	}

	return ret;
}

void vecu8_from_bmp(std::vector<uint8_t*>& outvec, SBitmap** bmp, int n_imgs) {
	for (int i = 0; i < n_imgs; ++i) {
		outvec.push_back(u8array_from_bmp(bmp[i]));
	}
}

void free_batch_bmps(SBitmap** bmps, int n_imgs) {
	for (int i = 0; i < n_imgs; ++i) {
		delete bmps[i];
	}
	delete bmps;
}

SBitmap* stretch_for_img2img(SBitmap* bmp) {
	NOT_NULL_OR_RETURN(bmp, nullptr);
	if (legal_img2img(bmp)) {
		return bmp->copy();
	}

	u32 w, h;
	w = bmp->width();
	h = bmp->height();
	if ((w & 63) != 0)
		w += (64 - (w & 63));
	if ((h & 63) != 0)
		h += (64 - (h & 63));
	return bmp->resize(w, h);
}

void stretch_for_img2img_replace(SBitmap* bmp) {
	NOT_NULL_OR_RETURN_VOID(bmp);
	if (legal_img2img(bmp)) {
		return;
	}

	u32 w, h;
	w = bmp->width();
	h = bmp->height();
	if ((w & 63) != 0)
		w += (64 - (w & 63));
	if ((h & 63) != 0)
		h += (64 - (h & 63));
	bmp->resize_and_replace(w, h);
}

bool legal_img2img(SBitmap* bmp) {
	return ((bmp->width() & 63) == 0) && ((bmp->height() & 63) == 0);
}

/*** end ldm.cpp ***/
