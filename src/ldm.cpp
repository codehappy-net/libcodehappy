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
    "Karras",
    "AlignYourSteps",
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

static std::mutex __ldm_mtx;

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
		free_sd_ctx(sd_model);
		sd_model = nullptr;
	}
}

SBitmap** SDServer::txt2img(const std::string& prompt, const std::string& neg_prompt, u32 w, u32 h, double cfg_scale,
                            i64 rng_seed, double variation_weight, i64* seed_return, int batch_count) {
	sd_image_t* output;
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
		output = ::txt2img(sd_model, 
			(prompt.empty() ? nullptr : prompt.c_str()),
			(neg_prompt.empty() ? nullptr : neg_prompt.c_str()),
			0, /* clip skip */
			cfg_scale,
			w,
			h,
			(sample_method_t) sampler,
			steps,
			seed,
			batch_count,
			nullptr, /* control_cond */
			0., /* control_strength */
			0., /* style strength */
			false, /* normalize_input */
			nullptr /* input_id_images_path */ );

		ret = new SBitmap * [batch_count];
		for (int e = 0; e < batch_count; ++e) {
			ret[e] = sdimg_to_bmp(output, e);
		}
		free_sdimg(output, batch_count);
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
                            double cfg_scale, i64 rng_seed, double variation_weight, i64* seed_return, int batch_count) {
	sd_image_t* output, * in_img;
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

	in_img = bmp_to_sdimg(init_img);

	output = ::img2img(sd_model,
			*in_img,
			(prompt.empty() ? nullptr : prompt.c_str()),
			(neg_prompt.empty() ? nullptr : neg_prompt.c_str()),
			0, /* clip skip */
			cfg_scale,
			w,
			h,
			(sample_method_t) sampler,
			steps,
			(float) img_strength,
			seed,
			batch_count,
			nullptr, /* control_cond */
			0., /* control_strength */
			0., /* style strength */
			false, /* normalize_input */
			nullptr /* input_id_images_path */ );

	ret = new SBitmap * [batch_count];
	for (int e = 0; e < batch_count; ++e) {
		ret[e] = sdimg_to_bmp(output, e);
	}

	free_sdimg(output, batch_count);
	free_sdimg(in_img, 1);

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

sd_type_t sdtype_from_wtype(ggml_type wtype) {
	switch (wtype) {
	case GGML_TYPE_F32:
		return SD_TYPE_F32;
	case GGML_TYPE_F16:
		return SD_TYPE_F16;
	case GGML_TYPE_Q8_0:
		return SD_TYPE_Q8_0;
	case GGML_TYPE_Q5_1:
		return SD_TYPE_Q5_1;
	case GGML_TYPE_Q5_0:
		return SD_TYPE_Q5_0;
	case GGML_TYPE_Q4_1:
		return SD_TYPE_Q4_1;
	case GGML_TYPE_Q4_0:
		return SD_TYPE_Q4_0;
	}

	return SD_TYPE_F16;
}

bool SDServer::load_from_file(const std::string& model_path, const std::string& vae_path, ggml_type wtype) {
	ScopeMutex sm(__ldm_mtx);
	const char * model_path_cstr, * vae_path_cstr = nullptr;
	if (sd_model != nullptr) {
		free_sd_ctx(sd_model);
	}

	sd_type_t mtype = sdtype_from_wtype(wtype_from_path(model_path, wtype));
	model_p = model_path;
	vae_p = vae_path;

	model_path_cstr = model_p.c_str();
	if (!vae_p.empty())
		vae_path_cstr = vae_p.c_str();

	sd_model = new_sd_ctx(model_path_cstr, vae_path_cstr, nullptr, nullptr, nullptr, nullptr, nullptr, false, false, false,
				(int) nthreads, mtype, STD_DEFAULT_RNG,
				(schedule_t) scheduler, false, false, false);

	return (sd_model != nullptr);
}

bool SDServer::load_from_file(const std::string& path, ggml_type wtype) {
	std::string empty;
	return load_from_file(path, empty, wtype);
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
		load_from_file(model_p, vae_p);
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


static void fill_sdimg_data(sd_image_t* sd_img, SBitmap* bmp) {
	// we use malloc() here because the SD library generated sdimgs do, too.
	uint8_t* w;
	sd_img->data = (uint8_t *) malloc(sd_img->width * sd_img->height * sd_img->channel);
	w = sd_img->data;
	for (int y = 0; y < bmp->height(); ++y) {
		for (int x = 0; x < bmp->width(); ++x) {
			RGBColor c = bmp->get_pixel(x, y);
			*w = (uint8_t) RGB_RED(c);
			++w;
			*w = (uint8_t) RGB_GREEN(c);
			++w;
			*w = (uint8_t) RGB_BLUE(c);
			++w;
		}
	}
}

sd_image_t* bmp_to_sdimg(SBitmap* bmp) {
	NOT_NULL_OR_RETURN(bmp, nullptr);
	sd_image_t* ret = (sd_image_t*) calloc(1, sizeof(sd_image_t));

	ret->width = bmp->width();
	ret->height = bmp->height();
	// this'll work for any input SBitmap format
	ret->channel = 3;
	fill_sdimg_data(ret, bmp);

	return ret;
}

sd_image_t* bmp_array_to_sdimg(SBitmap** bmp, int n_imgs) {
	NOT_NULL_OR_RETURN(bmp, nullptr);
	sd_image_t* ret = (sd_image_t*) calloc(n_imgs, sizeof(sd_image_t));
	for (int e = 0; e < n_imgs; ++e) {
		ret[e].width = bmp[e]->width();
		ret[e].height = bmp[e]->height();
		ret[e].channel = 3;
		fill_sdimg_data(&ret[e], bmp[e]);
	}
	return ret;
}

SBitmap* sdimg_to_bmp(sd_image_t* sd_img, int img_idx) {
	NOT_NULL_OR_RETURN(sd_img, nullptr);
	SBitmap* ret = new SBitmap(sd_img[img_idx].width, sd_img[img_idx].height);
	uint8_t* w = sd_img[img_idx].data;
	bool has_alpha = (sd_img[img_idx].channel > 3);

	ship_assert(sd_img[img_idx].channel == 3 || sd_img[img_idx].channel == 4);

	for (int y = 0; y < sd_img[img_idx].height; ++y) {
		for (int x = 0; x < sd_img[img_idx].width; ++x) {
			int r, g, b;
			r = (int) (*w);
			++w;
			g = (int) (*w);
			++w;
			b = (int) (*w);
			++w;
			if (has_alpha)
				++w;
			ret->put_pixel(x, y, RGB_NO_CHECK(r, g, b));
		}
	}

	return ret;
}

void free_sdimg(sd_image_t* sd_img_free, int n_imgs) {
	NOT_NULL_OR_RETURN_VOID(sd_img_free);
	/* this data is malloc'ed in sd_tensor_to_image() */
	for (int e = 0; e < n_imgs; ++e) {
		if (!is_null(sd_img_free[e].data))
			free(sd_img_free[e].data);
	}
	free(sd_img_free);
}

/*** end ldm.cpp ***/
