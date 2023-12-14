/***

	bert2stream.cpp

	Convert a compressed BERT .embeddings file into a streamable embeddings file.

	2023, C. M. Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	ArgParse ap;
	std::string in, out;

	ap.add_argument("in", type_string, "input embedding file");
	ap.add_argument("out", type_string, "output embedding stream file");
	ap.ensure_args(argc, argv);
	ap.value_str("in", in);
	ap.value_str("out", out);

	if (in.empty()) {
		codehappy_cerr << "*** Error: must provide an input compressed embeddings file to convert\n";
		ap.show_help();
		return 1;
	}
	if (out.empty() || in == out) {
		codehappy_cerr << "*** Error: must provide an output filename that is different from the input file\n";
		ap.show_help();
		return 1;
	}

	LMEmbeddingFolder lef;
	lef.in_from_file(in.c_str());
	lef.out_to_stream_fmt(out);

	return 0;
}

// end bert2stream.cpp
