/***

	llamagen.cpp
	
	A simple example of Llama generation using libcodehappy + CUDA-accelerated GGML.

	Chris Street, 2023.

***/
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string input;

	ap.add_argument("input", type_string, "Prompt string");
	ap.add_argument("input_file", type_string, "Prompt as a text file");
	llama_args(ap);
	ap.ensure_args(argc, argv);
	if (ap.flag_present("input")) {
		input = ap.value_str("input");
	}
	if (ap.flag_present("input_file")) {
		input = ap.value_str("input_file");
		if (!FileExists(input)) {
			codehappy_cerr << "*** Error: unable to find prompt file " << input << "\n";
			return 1;
		}
		input = string_from_text_file(input);
	}
	
	Llama llama(ap);
	llama.session_prompt(input);

	std::vector<llama_token> toks_out;
	std::cout << input;
	llama.generate_tokens(toks_out, true);
	std::cout << std::endl;

	return 0;
}

