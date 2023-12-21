/***

	llava.cpp

	Llava inference example: describe images.

	2023, Chris Street.

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string img_path;
	
	llama_defaults.temp = 0.1;
	llama_args(ap);
	ap.ensure_args(argc, argv);
	ap.all_nonflag_args(img_path);

	if (img_path.empty()) {
		codehappy_cerr << "*** you must provide the path to an image file to describe!\n";
		ap.show_help();
		return 1;
	}

	llama_defaults.temp = 0.1;
	llama_args(ap);
	ap.ensure_args(argc, argv);

	Llama model(ap);
	model.multimodal_image_prompt("Describe the image accurately and concisely; your description should be able to fit into a tweet. Be detailed but do not waste words. Write as if you are prompting an image model.", img_path);

	model.generate_tokens(-1, true);
	std::cout << std::endl;

	return 0;
}

/*** end llava.cpp ***/
