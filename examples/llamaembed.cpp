/***

	llamaembed.cpp
	
	Make a bunch of Llama model embeddings from a folder full of text documents, and do
	best cosine similarity search on it.

	2023, Chris Street

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_CUDA
#include <libcodehappy.h>

void print_stats(Llama& llama, LMEmbeddingFolder* lef) {
	std::cout << lef->count_files() << " text files processed.\n";
	std::cout << lef->count_embeddings() << " total text embeddings created.\n";
	std::cout << "Each embedding is " << llama.embedding_dimension() << " floats in length.\n";
	std::cout << "Total uncompressed size: " << (lef->count_embeddings() * llama.embedding_dimension() * sizeof(float)) << " bytes.\n";
}

void do_embedding_search(Llama& llama, const std::string& search, const std::string& in_file) {
	LMEmbeddingFolder lef;
	LMEmbedding* le;
	double score;

	RamFile rf(in_file, RAMFILE_READONLY);
	lef.in_from_ramfile(&rf);
	rf.close();
	print_stats(llama, &lef);

	le = llama.embedding_for_prompt(search);

	std::cout << "Best match by file:\n";
	int cf = lef.count_files();
	int best_fidx = 0, best_eidx = 0;
	double best_score = -2.0;

	for (int e = 0; e < cf; ++e) {
		int i = lef.best_match(e, le, &score);
		if (i >= 0) {
			std::cout << lef.files[e]->pathname << " at offset " << lef.files[e]->offsets[i] << " with score " << score << ".\n";
			if (score > best_score) {
				best_fidx = e;
				best_eidx = i;
				best_score = score;
			}
		}
	}
	std::cout << "\nBest overall match:\n";
	std::cout << lef.files[best_fidx]->pathname << " at offset " << lef.files[best_fidx]->offsets[best_eidx] << " with score " << best_score << ".\n";
	delete le;
}

void compile_folder_embeddings(Llama& llama, const std::string& folder, const std::string& out_file, int ntok) {
	std::cout << "Compiling embeddings for text documents in folder '" << folder << "', using ntok == " << ntok << "...\n";
	LMEmbeddingFolder* lef = llama.embeddings_for_folder(folder, ntok);
	print_stats(llama, lef);

	std::cout << "Saving to file '" << out_file << "'...\n";
	RamFile rf(out_file, RAMFILE_DEFAULT_COMPRESS);
	lef->out_to_ramfile(&rf);
	rf.close();
	delete lef;
}

int app_main() {
	ArgParse ap;
	std::string text, folder, out_file = "llama.embeddings", in_file = "llama.embeddings", search;
	int ntok = 0;

	llama_args(ap);
	ap.add_argument("folder", type_string, "folder of text files to compile an embedding database from");
	ap.add_argument("out", type_string, "name of the output file (default is 'llama.embeddings')");
	ap.add_argument("in", type_string, "input embedding file; use with 'search' (default is 'llama.embeddings')");
	ap.add_argument("search", type_string, "search string");
	ap.add_argument("ntok", type_int, "the number of tokens per embedding (default is 0, which indicates maximum allowed by context)", &ntok);
	ap.ensure_args(argc, argv);

	ap.value_str("folder", folder);
	ap.value_str("out", out_file);
	ap.value_str("in", in_file);
	ap.value_str("search", search);
	
	if (search.empty() && folder.empty()) {
		codehappy_cerr << "*** Error: if you're compiling an embeddings file, you must provide a folder containing text documents.\n";
		ap.show_help();
		return 1;
	}

	Llama llama(ap);
	llama.enable_embeddings();

	if (search.empty())
		compile_folder_embeddings(llama, folder, out_file, ntok);
	else
		do_embedding_search(llama, search, in_file);

	return 0;
}

/* end llamaembed.cpp */
