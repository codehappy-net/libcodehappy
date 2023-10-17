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
	std::string model_path = "/home/exx/ml/llama-gguf/models/mythomax-l2-13b.Q8_0.gguf";
	std::string input, output;

	ap.add_argument("model", type_string, "Model (path to ggml-compatible .bin file, Mythomax 13B default)");
	ap.ensure_args(argc, argv);
	if (ap.flag_present("model")) {
		model_path = ap.value_str("model");
	}
	
	Llama llama(model_path);

	forever {
		std::cout << "Enter instruction>> ";
		std::getline(std::cin, input);
		llama.isn_prompt(input);
		output = llama.generate_tokens(-1, true);
		std::cout << std::endl;
	}
	
	return 0;
}

