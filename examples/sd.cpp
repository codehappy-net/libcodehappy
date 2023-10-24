/***

	sd.cpp

	Simple demo of Stable Diffusion inference.

	Built for CPU (sd-cpu) and for CUDA (sd-cuda), although at time of writing the 2-d convolutional operations in GGML
	are not all available in the CUDA version of the library; these run on CPU so sd-cuda is only slightly faster.

	2023 C. M. Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string model_path, prompt, neg_prompt, out_path = "output.png";
	int w = 512, h = 512, threads = -1;
	double cfg = 7.0;

	ap.add_argument("w", type_int, "Width in pixels (default is 512)", &w);
	ap.add_argument("h", type_int, "Height in pixels (default is 512)", &h);
	ap.add_argument("prompt", type_string, "Prompt (default is empty)");
	ap.add_argument("neg_prompt", type_string, "Negative prompt (default is empty)");
	ap.add_argument("cfg", type_double, "Classifier-free guidance scale (default is 7.0)", &cfg);
	ap.add_argument("model", type_string, "Path to Stable Diffusion model in .gguf format");
	ap.add_argument("out", type_string, "Output path for the generated image (default is 'output.png')");
	ap.add_argument("threads", type_int, "Number of computational threads", &threads);
	ap.ensure_args(argc, argv);

	ap.value_str("prompt", prompt);
	ap.value_str("neg_prompt", neg_prompt);
	ap.value_str("model", model_path);
	ap.value_str("out", out_path);

	if (threads > 0)
		sd_server.set_nthreads((u32) threads);

	if (model_path.empty()) {
		std::cout << "Attempting to load a default SD model.\n";
		if (!sd_server.load_default_model()) {
			std::cerr << "Unable to find a default model to load! Place a .gguf-format Stable Diffusion\nmodel in the current path or specify a model path using /model.\n";
			return 1;
		}
	} else {
		if (!sd_server.load_from_file(model_path)) {
			std::cerr << "Error loading model file " << model_path << "!\n";
			return 1;
		}
	}
	std::cout << "Generating using model " << sd_server.get_model_path() << "\n";
	std::cout << "Prompt: " << prompt << "\n";
	std::cout << "Neg. prompt: " << neg_prompt << "\n";
	std::cout << "Dimensions: " << w << " x " << h << "\n";
	std::cout << "Classifier-free guidance scale: " << cfg << "\n";
	std::cout << "Number of CPU threads: " << sd_server.get_nthreads() << "\n";

	SBitmap* out = sd_server.txt2img(prompt, neg_prompt, w, h, cfg);

	std::cout << "The random seed used for this generation was " << sd_server.get_last_seed() << "\n";
	std::cout << "Writing image to '" << out_path << "'...\n";
	out->save_bmp(out_path);

	return 0;
}

/* end sd.cpp */
