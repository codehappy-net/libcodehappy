/***

	lora.cpp
	
	Compiles a text dataset to use for LoRA training. Give it a folder containing .txt 
	files and it will auto-split the text into paragraphs and output .json in the gpt4all 
	format (prompt, response, source).

	May 2023, C. M. Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	TextDataset dataset;
	ArgParse ap;
	std::string outname = "lora.json";

	ap.add_argument("dir", type_string, "The folder containing .txt files comprising the dataset (required)");
	ap.add_argument("out", type_string, "The path to the desired output .json (default is 'lora.json')");
	ap.ensure_args(argc, argv);

	if (!ap.flag_present("dir")) {
		codehappy_cerr << "Please specify a folder with the input .txt files using the --dir command line flag.\n";
		return 1;
	}
	if (ap.flag_present("out")) {
		outname = ap.value_str("out");
	}

	dataset.add_from_folder(ap.value_str("dir").c_str());
	dataset.show_stats();
	dataset.output_training_json(outname.c_str());

	return 0;
}

/* end lora.cpp */
