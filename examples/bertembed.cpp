/***

	bertembed.cpp
	
	Make a bunch of BERT model embeddings from a folder full of text documents, and do
	best cosine similarity search on it.

	2023, Chris Street

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

void print_stats(const BertEmbeddingManager& bert, LMEmbeddingFolder* lef) {
	std::cout << lef->count_files() << " text files processed.\n";
	std::cout << lef->count_embeddings() << " total text embeddings created.\n";
	std::cout << lef->count_text_bytes() << " bytes of original text saved.\n";
	std::cout << "Each embedding is " << bert.embedding_dimension() << " floats in length.\n";
	std::cout << "Total uncompressed size: " << (lef->count_embeddings() * bert.embedding_dimension() * sizeof(float)) + lef->count_text_bytes() << " bytes.\n";
}

void do_embedding_search(BertEmbeddingManager& bert, const std::string& search, const std::string& in_file, int max_matches) {
	LMEmbeddingFolder lef;
	std::vector<LMEmbedding*> les;
	double score;

	RamFile rf(in_file, RAMFILE_READONLY);
	lef.in_from_ramfile(&rf);
	rf.close();
	print_stats(bert, &lef);

	les = bert.embedding_for_text(search);
	std::cout << "Number of chunks in search string: " << les.size() << std::endl;

	std::cout << "*** Best match by file:\n";
	int cf = lef.count_files();

	for (int e = 0; e < cf; ++e) {
		for (const LMEmbedding* le : les) {
			int i = lef.best_match(e, le, &score);
			if (i >= 0) {
				std::cout << lef.files[e]->pathname << " at offset " << lef.files[e]->offsets[i] << " with score " << score << ".\n";
				if (lef.files[e]->embeds[i]->text != nullptr)
					std::cout << "\t" << lef.files[e]->embeds[i]->original_text() << "\n";
			}
		}
	}

	if (les.empty())
		return;

	LMBestMatch bm(max_matches);
	lef.best_matches(bm, les[0]);
	if (1 == max_matches)
		std::cout << "\n*** Best overall match:\n";
	else
		std::cout << "\n*** Best overall matches:\n";
	bm.sort_matches();
	for (int i = 0; i < bm.n_matches; ++i) {
		LMEmbedding* le = bm.matches[i];
		std::cout << bm.filename[i] << " at offset " << bm.offset[i] << " with score " << bm.cos_sim[i] << ".\n";
		std::cout << "\t" << le->original_text() << "\n";
	}

	for (LMEmbedding* le : les)
		delete le;
}

void compile_folder_embeddings(BertEmbeddingManager& bert, const std::string& folder, const std::string& out_file) {
	std::cout << "Compiling embeddings for text documents in folder '" << folder << "'...\n";
	LMEmbeddingFolder* lef = bert.embeddings_for_folder(folder);
	print_stats(bert, lef);

	std::cout << "Saving to file '" << out_file << "'...\n";
	RamFile rf(out_file, RAMFILE_DEFAULT_COMPRESS);
	lef->out_to_ramfile(&rf);
	rf.close();
	delete lef;
}

int app_main() {
	ArgParse ap;
	std::string text, folder, out_file = "bert.embeddings", in_file = "bert.embeddings", search, model = "bge-large-en-ggml-model-f16.bin";
	int max_matches = 8, n_sentences = 4;

	ap.add_argument("model", type_string, "BERT architecture embedding model (default is bge-large-en)");
	ap.add_argument("folder", type_string, "folder of text files to compile an embedding database from");
	ap.add_argument("out", type_string, "name of the output file (default is 'bert.embeddings')");
	ap.add_argument("in", type_string, "input embedding file; use with 'search' (default is 'bert.embeddings')");
	ap.add_argument("search", type_string, "search string");
	ap.add_argument("max_matches", type_int, "the number of best matches returned from the embedding search (default is 8)", &max_matches);
	ap.add_argument("num_sentences", type_int, "the number of sentences in each embedding (default is 4)", &n_sentences);
	ap.ensure_args(argc, argv);

	ap.value_str("folder", folder);
	ap.value_str("out", out_file);
	ap.value_str("in", in_file);
	ap.value_str("search", search);
	ap.value_str("model", model);
	
	if (search.empty() && folder.empty()) {
		codehappy_cerr << "*** Error: if you're compiling an embeddings file, you must provide a folder containing text documents.\n";
		ap.show_help();
		return 1;
	}

	BertEmbeddingManager bert(model);
	bert.set_nsentences(n_sentences);

	if (search.empty())
		compile_folder_embeddings(bert, folder, out_file);
	else
		do_embedding_search(bert, search, in_file, max_matches);

	return 0;
}

/* end bertembed.cpp */
