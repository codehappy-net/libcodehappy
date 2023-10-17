/***

	lang.cpp

	Translate text from one language into another, or ask what language a given text is in,
	using Llama large language models.

	C. M. Street, 2023

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

void translate(const std::string& model_path, const std::string& text, const std::string& in_lang, const std::string& out_lang, double temp) {
	Llama llama(model_path);

	std::string isn = "Translate the following text ", response = "Here is a complete and accurate translation of that text ";

	if (!in_lang.empty()) {
		std::string from = "from ";
		from += in_lang;
		from += " ";

		isn += from;
		response += from;
	}

	if (out_lang.empty()) {
		isn += "into English:";
		response += "into English:";
	} else {
		std::string into = "into ";
		into += out_lang;
		into += ":";

		isn += into;
		response += into;
	}

	isn += "\n\n";
	isn += text;
	isn += "\n";

	llama.set_temp(temp);
	llama.isn_prompt(isn, response);
	llama.generate_tokens(-1, true);
}

void detect_lang(const std::string& model_path, const std::string& text, double temp) {
	Llama llama(model_path);

	std::string isn = "What language is the following text written in?\n\n";
	isn += text;
	isn += "\n";

	llama.set_temp(temp);
	llama.isn_prompt(isn, "That text is written primarily in this natural language:");
	llama.generate_tokens(-1, true);
}

int app_main() {
	ArgParse ap;
	std::string text, from_lang, to_lang;
	std::string model = "/home/exx/ml/llama-gguf/models/mythomax-l2-13b.Q8_0.gguf";
	bool detect = false;
	double temp = 0.1;

	ap.add_argument("src", type_string, "(optional) the source language for the input text (default is to auto-detect language)");
	ap.add_argument("lang", type_string, "(optional) the language into which to translate the input text (default is English)");
	ap.add_argument("model", type_string, "language model to use (in GGML/GGUF format, default is Mythomax 13B, Llama 2 70B recommended)");
	ap.add_argument("detect", type_none, "instead of translation, detect the language that the input text is in", &detect);
	ap.add_argument("temp", type_double, "temperature for model inference (default is 0.1; 0.0 indicates greedy sampling)", &temp);
	ap.ensure_args(argc, argv);

	if (ap.nonflag_args() == 0) {
		std::cerr << "*** Error: no input text given -- we need at least one argument\n";
		ap.show_help();
		return 1;
	}
	ap.all_nonflag_args(text);

	ap.value_str("model", model);
	ap.value_str("src", from_lang);
	ap.value_str("lang", to_lang);
	
	if (detect)
		detect_lang(model, text, temp);
	else
		translate(model, text, from_lang, to_lang, temp);

	std::cout << "\n";

	return 0;
}

/*** end lang.cpp ***/
