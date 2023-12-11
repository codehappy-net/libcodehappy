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
	// chunks of sentences.
	// If 'lef_pathname' is non-null, the LMEmbeddingFolder contents will be initialized from that file (if it
	// exists), and embeddings_for_folder() will save the LMEmbeddingFolder after each file's embeddings
	// are added. This way, if a long embedding operation is stopped it can continue at the most recent file.
	LMEmbeddingFolder* embeddings_for_folder(const std::string& path, const char* lef_pathname = nullptr);
	void embeddings_for_folder(const std::string& path, LMEmbeddingFolder* lef, const char* lef_pathname = nullptr);

	// Return the embedding dimension for the model. 
	int embedding_dimension() const	{ NOT_NULL_OR_RETURN(model, 0); return bert_n_embd(model); }

	// Accessors for n_threads and save_text
	int get_nthreads() const		{ return n_threads; }
	void set_nthreads(int nthreads)	{ n_threads = nthreads; }
	bool get_save_text() const		{ return save_text; }
	void set_save_text(bool st)		{ save_text = st; }
	int get_nsentences() const		{ return n_sentences; }
	void set_nsentences(int nsentences)	{ n_sentences = nsentences; }

private:
	char* normalize_string(const std::string& in_str) const;

	bert_ctx* model;
	int n_threads;
	bool save_text;
	int n_sentences;
};

/*** Break a C string, in place, into sentences. If sentences is non-null, this vector is filled with pointers
     to the beginning of each sentence. Zero terminators are placed in the C string at the end of each sentence,
     so if sentences is null, you should store the original length or pointer to the end of the ntxt buffer. ***/
extern void sentencify(char* ntxt, std::vector<char*>* sentences = nullptr);

#endif  // __BERT_EMBEDDINGS_H
/*** end bert.h ***/
