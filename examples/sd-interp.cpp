/***

	sd.cpp

	Simple demo of Stable Diffusion interpolation.

	May 2024, C. M. Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

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

int app_main() {
	ArgParse ap;
	std::string model_path, vae_path;
	int w = 512, h = 512, threads = -1, steps = 30, sampler = -1, scheduler = -1, batch_size = 1;
	bool interp_noise = false;
	double cfg;
	std::string prompt_1, neg_prompt_1;
	SDInterpolationData interp_data;
	SBitmap** ret;
	const int MAX_SAMPLER = 7, MAX_SCHEDULER = 3;

	ap.add_argument("w", type_int, "Width in pixels (default is 512)", &w);
	ap.add_argument("h", type_int, "Height in pixels (default is 512)", &h);
	ap.add_argument("model", type_string, "Path to Stable Diffusion model");
	ap.add_argument("vae", type_string, "Path to VAE (if a separate VAE is desired)");
	ap.add_argument("threads", type_int, "Number of computational threads (important for CPU inference)", &threads);
	ap.add_argument("steps", type_int, "Number of denoising steps", &steps);
	ap.add_argument("sampler", type_int, "sampler type (0-7)", &sampler);
	ap.add_argument("scheduler", type_int, "scheduler type (0-3)", &scheduler);
	ap.add_argument("batch", type_int, "batch size", &batch_size);
	ap.add_argument("noise", type_none, "interpolate on the noise tensor", &interp_noise);
	ap.ensure_args(argc, argv);

	ap.value_str("model", model_path);
	ap.value_str("vae", vae_path);

	if (threads > 0)
		sd_server.set_nthreads((u32) threads);
	sd_server.set_steps((u32) steps);
	if (sampler >= 0 && sampler <= MAX_SAMPLER) {
		sd_server.set_sampler_type((SDSamplerType) sampler);
	}
	if (scheduler >= 0 && scheduler <= MAX_SCHEDULER) {
		sd_server.set_scheduler_type((SDSchedulerType) scheduler);
	}

	if (model_path.empty()) {
		std::cout << "Attempting to load a default SD model.\n";
		if (!sd_server.load_default_model()) {
			std::cerr << "Unable to find a default model to load! Place a .gguf-format Stable Diffusion\nmodel in the current path or specify a model path using /model.\n";
			return 1;
		}
	} else {
		if (!sd_server.load_from_file(model_path, vae_path)) {
			std::cerr << "Error loading model file " << model_path << "!\n";
			return 1;
		}
	}
	std::cout << "Generating using model " << sd_server.get_model_path() << "\n";
	if (!vae_path.empty()) {
		std::cout << "Using user-provided variadic autoencoder model at " << vae_path << "\n";
	}

	/* input parameters */
	interp_data.max_steps = (int) user_u32("How many frames would you like in the interpolation? ", 1, 1000);
	std::cout << "Enter the prompt for the starting image: ";
	prompt_1 = multiline_input();
	std::cout << "Enter the prompt for the ending image (empty to use same prompt): ";
	interp_data.prompt2 = multiline_input();
	std::cout << "Enter the negative prompt for the starting image (empty for none): ";
	neg_prompt_1 = multiline_input();
	std::cout << "Enter the negative prompt for the ending image (empty to use same prompt): ";
	interp_data.neg_prompt2 = multiline_input();
	std::cout << "Enter the classifer-free guidance scale for the starting image: ";
	std::cin >> cfg;
	std::cout << "Enter the classifer-free guidance scale for the ending image (0 to use the same): ";
	std::cin >> interp_data.cfg;

	if (interp_noise) {
		interp_data.seed2 = RandI64();
		if (interp_data.seed2 < 1)
			interp_data.seed2 = -interp_data.seed2;
		std::cout << "Interpolating on noise as well.\n";
	}

	std::cout << "Number of CPU threads: " << sd_server.get_nthreads() << "\n";
	std::cout << "Denoising steps: " << sd_server.get_steps() << "\n";
	if (sampler >= 0) {
		std::cout << "Sampler: " << sampler << "\n";
	}
	if (scheduler >= 0) {
		std::cout << "Scheduler: " << scheduler << "\n";
	}

	ret = sd_server.txt2img_slerp(&interp_data, prompt_1, neg_prompt_1, -1ll, (u32) w, (u32) h, cfg, nullptr, nullptr, batch_size);

	if (is_null(ret)) {
		codehappy_cerr << "no images returned from SDServer::txt2img_slerp()?\n";
		return 1;
	}

	std::cout << "Writing images to frame%04d.png...\n";
	int fs = frame_number_start();
	for (int i = 0; i < interp_data.max_steps; ++i) {
		char fname[32];
		sprintf(fname, "frame%04d.png", i + fs);
		ret[i]->save_bmp(fname);
	}
	free_batch_bmps(ret, interp_data.max_steps);

	return 0;
}

/* end sd.cpp */
