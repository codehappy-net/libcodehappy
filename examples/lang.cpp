/***

	lang.cpp

	Translate text from one language into another, or ask what language a given text is in,
	using Llama large language models.

	C. M. Street, 2023

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

void translate(const ArgParse& model_args, const std::string& text, const std::string& in_lang, const std::string& out_lang) {
	Llama llama(model_args);

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

	llama.isn_prompt(isn, response);
	llama.generate_tokens(-1, true);
}

void detect_lang(const ArgParse& model_args, const std::string& text) {
	Llama llama(model_args);

	std::string isn = "What language is the following text written in?\n\n";
	isn += text;
	isn += "\n";

	llama.isn_prompt(isn, "That text is written primarily in this natural language:");
	llama.generate_tokens(-1, true);
}

int app_main() {
	ArgParse ap;
	std::string text, from_lang, to_lang;
	bool detect = false;

	ap.add_argument("src", type_string, "(optional) the source language for the input text (default is to auto-detect language)");
	ap.add_argument("lang", type_string, "(optional) the language into which to translate the input text (default is English)");
	ap.add_argument("detect", type_none, "instead of translation, detect the language that the input text is in", &detect);
	llama_defaults.temp = 0.1;
	llama_args(ap);
	ap.ensure_args(argc, argv);

	if (ap.nonflag_args() == 0) {
		std::cerr << "*** Error: no input text given -- we need at least one argument\n";
		ap.show_help();
		return 1;
	}
	ap.all_nonflag_args(text);

	ap.value_str("src", from_lang);
	ap.value_str("lang", to_lang);
	
	if (detect)
		detect_lang(ap, text);
	else
		translate(ap, text, from_lang, to_lang);

	std::cout << "\n";

	return 0;
}

/*** end lang.cpp ***/
