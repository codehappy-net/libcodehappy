/***

	llamagen.cpp
	
	A simple example of Llama generation using libcodehappy + CUDA-accelerated GGML.

	Chris Street, 2023.

***/
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string model_path = "/home/exx/ml/llama-gguf/models/mythomax-l2-13b.Q8_0.gguf";
	std::string input;

	ap.add_argument("input", type_string, "Prompt string");
	ap.add_argument("model", type_string, "Model (path to ggml-compatible .bin file, Mythomax 13B default)");
	ap.ensure_args(argc, argv);
	if (ap.flag_present("input")) {
		input = ap.value_str("input");
	}
	if (ap.flag_present("model")) {
		model_path = ap.value_str("model");
	}
	
	Llama llama(model_path);
	llama.session_prompt(input);

	std::vector<llama_token> toks_out;
	std::cout << input;
	llama.generate_tokens(toks_out, true);
	std::cout << std::endl;

	return 0;
}

