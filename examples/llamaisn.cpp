/***

	llamaisn.cpp
	
	A simple example of Llama generation using libcodehappy + CUDA-accelerated GGML.
	This is the instruction following version.

	Chris Street, 2023.

***/
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string input, output;

	llama_args(ap);
	ap.ensure_args(argc, argv);
	Llama llama(ap);

	forever {
		std::cout << "Enter instruction>> ";
		input = multiline_input();
		llama.isn_prompt(input);
		output = llama.generate_tokens(-1, true);
		std::cout << std::endl;
	}
	
	return 0;
}

