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
	std::string input, out_file;

	ap.add_argument("input", type_string, "String to tokenize");
	ap.add_argument("output", type_string, "If you specify an output filename, the tokenization is written here.");
	llama_args(ap);
	ap.ensure_args(argc, argv);

	if (ap.flag_present("input")) {
		input = ap.value_str("input");
	} else {
		codehappy_cerr << "Error: Please specify a text to tokenize.\n";
		return 1;
	}
	if (ap.flag_present("output")) {
		out_file = ap.value_str("output");
	}

	Llama model(ap);
	std::vector<llama_token> ltokens;
	int ntok = model.tokenize(input, ltokens);

	if (ntok <= 0) {
		codehappy_cerr << "Unable to tokenize string; llama.tokenize() returned " << ntok << ".\n";
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

	return 0;
}
/* end llamatok.cpp */
