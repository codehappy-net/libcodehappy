/***

	lmembed.h
	
	Structs and classes for handling language model embeddings. Good for
	semantic search, retrieval-augmented generation, etc.

	2023, Chris Street

***/
#ifndef __LMEMBED_H__
#define __LMEMBED_H__

/*** embeddings and embedding managers ***/
struct LMEmbedding {
	LMEmbedding();
	~LMEmbedding();

	// Compute the cosine similarity with another embedding.
	double cosine_similarity(const LMEmbedding* le) const;

	// Compute the magnitude of the embedding.
	double magnitude() const;

	// Compute the dot product of the embedding with another.
	double dot_product(const LMEmbedding* le) const;

	// Copy the values from the provided float array.
	void copy_from_array(int n_el, const float* array);

	// Write to a ramfile.
	void out_to_ramfile(RamFile* rf);

	// Read from a ramfile.
	void in_from_ramfile(RamFile* rf);

	// Write to a text stream (for streaming embedding matches).
	void out_to_stream_fmt(std::ostream& o = std::cout);

	// Free data allocated for the embedding.
	void free();

	// Return the text as a std::string
	std::string original_text() const;

	int n_embed;		// size (dimension) of the embedding
	float* embed_data;	// embedding representation
	char* text;		// text
};

const int MAX_EMBED_MATCHES = 256;
const int DEFAULT_EMBED_MATCHES = 16;

struct LMBestMatch {
	LMBestMatch(int max_matches = DEFAULT_EMBED_MATCHES);
	~LMBestMatch();

	bool check_match(LMEmbedding* lme, double score, const char* fname = nullptr, u32 offs = 0);
	void set_min_cosine_similarity(double min_val)	{ min_cos_sim = min_val; }
	void sort_matches();

	int n_matches;
	int n_matches_max;
	LMEmbedding* matches[MAX_EMBED_MATCHES];
	double cos_sim[MAX_EMBED_MATCHES];
	const char* filename[MAX_EMBED_MATCHES];
	u32 offset[MAX_EMBED_MATCHES];
	double min_cos_sim;
	int i_min;
	bool i_own_this_memory;
};

struct LMEmbeddingFile {
	LMEmbeddingFile();
	~LMEmbeddingFile();

	std::string pathname;
	std::vector<LMEmbedding*> embeds;
	std::vector<u32> offsets;

	// Retrieve the best match (by cosine similarity) to the passed-in embedding.
	// The return value is the index of the best match in embeds/offsets. If score is non-NULL,
	// the cosine similarity is returned in that variable.
	int best_match(const LMEmbedding* le, double* score = nullptr);
	void best_matches(LMBestMatch& best_matches, const LMEmbedding* le);

	void out_to_ramfile(RamFile* rf);
	void in_from_ramfile(RamFile* rf);
	void out_to_stream_fmt(std::ostream& o = std::cout);
	void free();
	int count_embeddings() const;
	int count_text_bytes() const;
};

struct LMEmbeddingFolder {
	LMEmbeddingFolder();
	~LMEmbeddingFolder();

	std::vector<LMEmbeddingFile*> files;
	std::unordered_set<std::string> known_files;

	void out_to_ramfile(RamFile* rf);
	void in_from_ramfile(RamFile* rf);
	void in_from_file(const char* path);
	void out_to_file(const char* path);
	void out_to_stream_fmt(const std::string& path);
	void free();
	int count_files() const;
	int count_embeddings() const;
	int count_text_bytes() const;
	int best_match(int file_idx, const LMEmbedding* le, double* score = nullptr);
	void best_matches(LMBestMatch& best_matches, const LMEmbedding* le);
};

/* read/match embeddings from file -- we don't need them all in memory for a lookup */
class LMEmbeddingStream {
public:
	LMEmbeddingStream(const std::string& filepath);
	~LMEmbeddingStream();

	void rewind();
	bool read_embedding(LMEmbedding& lme_out, std::string* path_out = nullptr, u32* offs_out = nullptr);
	void best_matches(LMBestMatch& best_matches, const LMEmbedding* le);

private:
	std::ifstream stream;
};

// helper: allocates a new C string and copies a std::string into it.
extern char* cpp_strdup(const std::string& str);

#endif  // __LMEMBED_H__
