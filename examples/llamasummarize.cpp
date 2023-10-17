/***

	llamasummarize.cpp

	Given a lengthy text, generate summaries for ~2K token chunks using Llama models.

	C. M. Street, 2023

***/
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

void summarize(std::vector<llama_token>& toks, int i1, int i2, std::ostream& o, Llama& llama) {
	std::string isn;
	std::vector<llama_token> tks;

	isn = "Summarize the following text concisely and accurately in its entirety:\n\n";
	for (int i = i1; i <= i2; ++i) {
		tks.push_back(toks[i]);
	}
	isn += llama.text_from_tokens(tks);
	llama.isn_prompt(isn, "Here is a complete accurate summary of the provided text:\n");
	o << llama.generate_tokens();
	o << std::endl << std::endl;
}

int app_main() {
	ArgParse ap;
	std::string model_path = "/home/exx/ml/llama-gguf/models/mythomax-l2-13b.Q8_0.gguf";
	std::string input, output = "output.txt";
	int tok_per = 2000;
	bool overlap = false;

	ap.add_argument("input", type_string, "Input file to summarize");
	ap.add_argument("output", type_string, "Output file to contain the summarizations (default output.txt)");
	ap.add_argument("model", type_string, "Model (path to ggml-compatible .bin file, Mythomax 13B default)");
	ap.add_argument("tokens_per", type_int, "The number of tokens per chunk to summarize (default 2000)", &tok_per);
	ap.add_argument("overlap", type_none, "Use overlapping excerpts in the summarization.", &overlap);
	ap.ensure_args(argc, argv);
	if (ap.flag_present("input")) {
		input = ap.value_str("input");
	} else {
		codehappy_cerr << "Error: Please provide an input file to summarize.\n";
		return 1;
	}
	if (ap.flag_present("model")) {
		model_path = ap.value_str("model");
	}
	if (ap.flag_present("output")) {
		output = ap.value_str("output");
	}

	Llama llama(model_path);
	RamFile rf(input, RAMFILE_READONLY);
	char* buf = (char *)rf.buffer();
	std::vector<llama_token> toks;
	llama.tokenize(buf, toks);
	int nchunks = (toks.size() / tok_per) + 1;
	int tokper = toks.size() / nchunks;
	int ix = 0;
	std::ofstream o;

	std::cout << "Total of " << toks.size() << " tokens in text.\n";
	std::cout << "Generating " << nchunks << " summaries, " << tokper << " tokens per summary.\n";
	if (overlap) {
		std::cout << "Using overlapping chunks, there'll be an extra chunk at the start.\n";
	}

	o.open(output);
	if (overlap) {
		std::cout << "Chunk 1...\n";
		summarize(toks, 0, std::min(tokper - 1, (int) toks.size() - 1), o, llama);
		if (toks.size() > tokper) {
			int ch = 1;
			ix = tokper / 2;
			forever {
				if (ix >= toks.size())
					break;
				++ch;
				std::cout << "Chunk " << ch << "...\n";
				summarize(toks, ix, std::min(ix + tokper - 1, (int) toks.size() - 1), o, llama);
				ix += tokper;
			}
		}
	} else {
		for (int i = 0; i < nchunks; ++i) {
			std::cout << "Chunk " << (i + 1) << "...\n";
			if (i + 1 >= nchunks)
				summarize(toks, ix, toks.size() - 1, o, llama);
			else
				summarize(toks, ix, ix + tokper - 1, o, llama);
			ix += tokper;
		}
	}
	o.close();
	o.clear();

	return 0;
}

