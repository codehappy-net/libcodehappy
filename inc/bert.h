/***

	bert.h
	
	BERT sentence embedding manager. Models with this architecture, such as bge-large,
	are better for semantic search than Llama models.

	Chris Street, 2023

***/
#ifndef __BERT_EMBEDDINGS_H
#define __BERT_EMBEDDINGS_H

class BertEmbeddingManager {
public:
	// construct the embedding manager, using the specified model (raw GGML format expected.)
	// bge-large and all-MiniLM are common choices.
	// The script 'bert-convert-to-ggml.py' (in inc/external/) converts HuggingFace format BERT models to .ggml
	// format. The script 'bert_quantize.cpp' can quantize these models to 4 bpw if desired.
	BertEmbeddingManager(const std::string& model_path);
	~BertEmbeddingManager();

	// Create embeddings for given text conditioning.
	std::vector<LMEmbedding*> embedding_for_text(const std::string& str);
	void embedding_for_text(const std::string& str, std::vector<LMEmbedding*>& le);
	void embedding_for_text(const std::string& str, std::vector<LMEmbedding*>& le, std::vector<u32>& offs);

	// Create embeddings for an entire text file: it's broken up into sentences.
	LMEmbeddingFile* embeddings_for_file(const std::string& str);
	void embeddings_for_file(const std::string& str, LMEmbeddingFile* lef);

	// Create embeddings for all text files (*.txt) in a folder. As above, documents are broken into
	// sentences.
	LMEmbeddingFolder* embeddings_for_folder(const std::string& path);
	void embeddings_for_folder(const std::string& path, LMEmbeddingFolder* lef);

	// Return the embedding dimension for the model. 
	int embedding_dimension() const	{ NOT_NULL_OR_RETURN(model, 0); return bert_n_embd(model); }

	// Accessors for n_threads and save_text
	int get_nthreads() const		{ return n_threads; }
	void set_nthreads(int nthreads)	{ n_threads = nthreads; }
	bool get_save_text() const		{ return save_text; }
	void set_save_text(bool st)		{ save_text = st; }

private:
	char* normalize_string(const std::string& in_str) const;

	bert_ctx* model;
	int n_threads;
	bool save_text;
};

#endif  // __BERT_EMBEDDINGS_H
/*** end bert.h ***/
