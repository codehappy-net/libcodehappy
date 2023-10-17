/***

	llamatok.cpp
	
	Demo of using the GGML/llama.cpp library with libcodehappy.
	Tokenizes passed-in text according to the Llama tokenizer.

	C. M. Street, 2023

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	gpt_params params;
	std::string model_path = "/home/exx/ml/llama-gguf/models/mythomax-l2-13b.Q8_0.gguf";
	std::string input, out_file;

	ap.add_argument("input", type_string, "String to tokenize");
	ap.add_argument("model", type_string, "Model (path to ggml-compatible .bin file, Mythomax 13B default)");
	ap.add_argument("output", type_string, "If you specify an output filename, the tokenization is written here.");
	ap.ensure_args(argc, argv);
	if (ap.flag_present("input")) {
		input = ap.value_str("input");
	} else {
		codehappy_cerr << "Error: Please specify a text to tokenize.\n";
		return 1;
	}
	if (ap.flag_present("model")) {
		model_path = ap.value_str("model");
	}
	if (ap.flag_present("output")) {
		out_file = ap.value_str("output");
	}
	
	params.n_ctx = 4096;
	params.model = model_path;
	params.n_batch = 1024;
	llama_model * model;
	llama_context * ctx;
	std::tie(model, ctx) = llama_init_from_gpt_params(params);
	if (nullptr == model || nullptr == ctx) {
		codehappy_cerr << "Unable to load the model '" << model_path << "'.\n";
		return 2;
	}

	// tokenize text up to 8 million tokens at a time
	llama_token* ltokens = new llama_token[8 * 1024 * 1024];
	int ntok = llama_tokenize(ctx, input.c_str(), ltokens, 8 * 1024 * 1024, false);
	if (ntok <= 0) {
		codehappy_cerr << "Unable to tokenize string; llama_tokenize() returned " << ntok << ".\n";
		return 3;
	}
	std::cout << "Llama tokenization:\n[" << ltokens[0];
	for (int e = 1; e < ntok; ++e) {
		std::cout << ", " << ltokens[e];
	}
	std::cout << "]\n";
	std::cout << ntok << " total tokens in the Llama token representation.\n";

	if (!out_file.empty()) {
		RamFile rf(out_file, RAMFILE_DEFAULT);
		for (int e = 0; e < ntok; ++e) {
			rf.putu16((u16) ltokens[e]);
		}
		rf.close();
	}

	delete ltokens;

	return 0;
}
/* end llamatok.cpp */
