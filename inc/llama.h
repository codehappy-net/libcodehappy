/***

	llama.h
	
	A friendly wrapper around the ggml/llama.cpp library for loading models, tokenizing
	text, generating text or embeddings, etc.

	Chris Street, August 2023

***/
#ifndef __LLAMA_CODEHAPPY
#define __LLAMA_CODEHAPPY
#ifdef  CODEHAPPY_CUDA

class Llama;

/*** embeddings and embedding managers ***/
struct LlamaEmbedding {
	LlamaEmbedding();
	~LlamaEmbedding();

	// Compute the cosine similarity with another embedding.
	double cosine_similarity(const LlamaEmbedding* le) const;

	// Compute the magnitude of the embedding.
	double magnitude() const;

	// Compute the dot product of the embedding with another.
	double dot_product(const LlamaEmbedding* le) const;

	// Copy the values from the provided float array.
	void copy_from_array(int n_el, const float* array);

	// Write to a ramfile.
	void out_to_ramfile(RamFile* rf);

	// Read from a ramfile.
	void in_from_ramfile(RamFile* rf);

	// Free data allocated for the embedding.
	void free();

	int n_embed;		// size (dimension) of the embedding
	float* embed_data;	// embedding representation
};

struct LlamaEmbeddingFile {
	LlamaEmbeddingFile();
	~LlamaEmbeddingFile();

	std::string pathname;
	std::vector<LlamaEmbedding*> embeds;
	std::vector<u32> offsets;

	// Retrieve the best match (by cosine similarity) to the passed-in embedding.
	// The return value is the index of the best match in embeds/offsets. If score is non-NULL,
	// the cosine similarity is returned in that variable.
	int best_match(const LlamaEmbedding* le, double* score = nullptr);

	void out_to_ramfile(RamFile* rf);
	void in_from_ramfile(RamFile* rf);
	void free();
	int count_embeddings() const;
};

struct LlamaEmbeddingFolder {
	LlamaEmbeddingFolder();
	~LlamaEmbeddingFolder();

	std::vector<LlamaEmbeddingFile*> files;

	void out_to_ramfile(RamFile* rf);
	void in_from_ramfile(RamFile* rf);
	void free();
	int count_files() const;
	int count_embeddings() const;
	int best_match(int file_idx, const LlamaEmbedding* le, double* score = nullptr);
};

struct ChatEntry {
	ChatEntry(Llama* l, const std::string& p, const std::string& r);

	std::string persona;
	std::string response;
	u32 tokens;
};

/* Different types of instruction rubrics that might be used by instruction-tuned models. */
enum InstructionType {
	ISN_ALPACA,	// ### Instruction: and ### Response:
	ISN_MISTRAL,	// <s>[INST] and [/INST]
	ISN_PYGMALION,	// <|system|> and <|model|>
	ISN_CODELLAMA,	// [INST]
};

typedef void (*LlamaCallback)(const char *);

struct LlamaDefaults {
	LlamaDefaults();
	std::string model_path;
	int top_k;
	float top_p;
	float temp;
	float rp;
	float fp;
	int mirostat;
	float miro_tau;
	float miro_eta;
	int main_gpu;
	bool cpuonly;
	int layers_gpu;
	int vram_gb;
	bool og_llama;
};

extern LlamaDefaults llama_defaults;

/* Add Llama generation arguments to the ArgParse object. */
extern void llama_args(ArgParse& ap);

class Llama {
public:
	// We use lazy loading; we don't actually load the model until we need to generate.
	Llama(const char* model_path, int vram_gb = 24, bool og_llama = false, bool is_70b = false);
	Llama(const std::string& model_path, int vram_gb = 24, bool og_llama = false, bool is_70b = false);
	Llama(const ArgParse& ap, const LlamaDefaults& defaults = llama_defaults);
	~Llama();

	// Tokenize the provided string. If the output vector "out" already has tokens in it, the
	// new tokens are appended to the end. Returns the number of new tokens on success, a
	// negative value on error. If max_tokens is non-0, the number of new tokens added is
	// capped at max_tokens (the rest of the string is ignored.)
	int tokenize(const std::string& str, std::vector<llama_token>& out, bool add_bos = false, u32 max_tokens = 0);
	int tokenize(const char* str, std::vector<llama_token>& out, bool add_bos = false, u32 max_tokens = 0);

	// Just give the count of tokens in the specified string.
	u32 token_count(const std::string& str);
	u32 token_count(const char* str);

	// Returns the string corresponding to the first max_tokens tokens. Tries to divide strings on word/whitespace
	// boundaries. The return value is guaranteed to be <= max_tokens tokens long.
	std::string truncate_nicely_by_tokens(const std::string& str, u32 max_tokens);
	std::string truncate_nicely_by_tokens(const char* str, u32 max_tokens);

	// Create embeddings for given text conditioning.
	LlamaEmbedding* embedding_for_prompt(const std::string& str);
	void embedding_for_prompt(const std::string& str, LlamaEmbedding* le);

	// Create embeddings for an entire text file: it's broken up into chunks of n_tok tokens (if n_tok is 0,
	// then maximum model context / 2 is used.)
	LlamaEmbeddingFile* embeddings_for_file(const std::string& str, int n_tok = 0);
	void embeddings_for_file(const std::string& str, LlamaEmbeddingFile* lef, int n_tok = 0);

	// Create embeddings for all text files (*.txt) in a folder. As above, documents are broken into
	// pieces of length n_tok tokens (if n_tok <= 0, maximum model context / 2 is used.)
	LlamaEmbeddingFolder* embeddings_for_folder(const std::string& path, int n_tok = 0);
	void embeddings_for_folder(const std::string& path, LlamaEmbeddingFolder* lef, int n_tok = 0);

	// Give an initial prompt/initial completion. This resets any session in progress.
	void session_prompt(const std::string& str);
	void session_prompt(const char* str);

	// Give an instruction prompt. This uses the "### Instruction: / ### Response:" rubric from Alpaca/Vicuna. Just
	// provide whatever you want to include under the Instruction heading. You can also begin the model's response by
	// specifying a partial completion.
	// Any session in progress is reset.
	void isn_prompt(const std::string& str);
	void isn_prompt(const char* str);
	void isn_prompt(const std::string& str, const std::string& response_begin);
	void isn_prompt(const char* str, const char* response_begin);

	// Give a prefix prompt. This is useful in contexts like chat where you might add more and more text, eventually
	// overflowing your maximum context size. When this happens, the prefix prompt is preserved, and tokens are trimmed
	// off the beginning of the response after the prefix to make the conditioning fit in context. 
	void prefix_prompt(const std::string& str);
	void prefix_prompt(const char* str);

	// Begin a chatbot session. You provide a 'character card' for the bot's persona, their display name, your display
	// name, and a greeting which will be the bot's first response (if you would prefer to begin the conversation, pass
	// an empty string as the greeting.) In chat mode, the model will only generate one line at a time, and will keep
	// track of the chat history so that if we need to scroll anything out due to context limits, we will proceed on
	// a line boundary to ensure the best quality generations. Any previous session is reset.
	// The 'character card' may also include descriptions of the user and any other characters that might enter the chat.
	void chat_session(const std::string& char_card, const std::string& bot_name, const std::string& user_name, const std::string& bot_greeting);
	// For chat sessions: give the user response. You don't need to (and should not) put the user's display name first.
	void chat_user_response(const std::string& response);
	// For chat sessions: give a response as the specified user. This can be the regular user, the main computer
	// character, or a new character. Useful for multi-chat: the additional characters can be described in the "character
	// card" used to initialize the chat session.
	void multichat_user_response(const std::string& user, const std::string& response);
	// For chat sessions: get the next response from the chatbot.
	std::string chat_response();
	// Return the full chat history.
	std::string chat_history();
	// Rewind the chat history one chat.
	void chat_rewind();

	// Add text to the session.
	void add_text(const std::string& str);
	void add_text(const char* str);

	// Give the text for the specified vector of llama_tokens.
	std::string text_from_tokens(const std::vector<llama_token>& toks) const;
	
	// Returns all the text from the current session.
	std::string session_text();

	// Generate tokens (up to max_tokens, if passed in, with < 0 meaning 'no token limit, generate to end of text'.)
	// If 'echo' is true, the tokens will be printed to stdout as they are generated.
	// If 'clback' is non-null, each token will be passed to the callback function as a C string.
	void generate_tokens(std::vector<llama_token>& toks_out, bool echo = false, LlamaCallback clback = nullptr);
	void generate_tokens(std::vector<llama_token>& toks_out, int max_tokens, bool echo = false, LlamaCallback clback = nullptr);
	// Generate tokens and return them as a string.
	std::string generate_tokens(int max_tokens = -1, bool echo = false, LlamaCallback clback = nullptr);

	// Threading for generation.
	void set_nthreads(int threads)	{ params.n_threads = threads; }
	int get_nthreads() const		{ return params.n_threads; }

	// Enable or disable embeddings.
	void enable_embeddings()		{ params.embedding = true; }
	void disable_embeddings()		{ params.embedding = false; }
	bool embeddings_enabled() const	{ return params.embedding; }

	// Set various generation parameters.
	void set_top_k(int top_k)		{ params.top_k = top_k; }
	void set_top_p(float top_p)		{ params.top_p = top_p; }
	void set_temp(float temp)		{ params.temp = temp; }
	void set_repeat_penalty(float rp)	{ params.repeat_penalty = rp; }
	void set_repeat_last_n(int n)		{ params.repeat_last_n = n; }
	void set_frequency_penalty(float fp)	{ params.frequency_penalty = fp; }
	void set_presence_penalty(float pp)	{ params.presence_penalty = pp; }
	void set_mirostat(int mirostat)	{ params.mirostat = mirostat; }
	void set_mirostat_tau(float tau)	{ params.mirostat_tau = tau; }
	void set_mirostat_eta(float eta)	{ params.mirostat_eta = eta; }
	void set_tokens_predict(int tok)	{ params.n_predict = tok; }
	void set_context(int ctx)		{ params.n_ctx = ctx; }
	void set_isn_type(InstructionType it)	{ isn_type = it; }
	void set_cfg_neg_prompt(std::string& cfg_prompt)	{ params.cfg_negative_prompt = cfg_prompt; }
	void set_cfg_scale(float cfg)				{ params.cfg_scale = cfg; }
	void set_stop_string(std::string& stop_str)		{ stop_string = stop_str; }
	void set_remove_stop_string(bool rss)			{ remove_stop_str = rss; }

	// Get various generation parameters.
	int get_top_k() const			{ return params.top_k; }
	float get_top_p() const		{ return params.top_p; }
	float get_temp() const			{ return params.temp; }
	float get_repeat_penalty() const	{ return params.repeat_penalty; }
	int get_repeat_last_n()		{ return params.repeat_last_n; }
	float get_frequency_penalty() const	{ return params.frequency_penalty; }
	float get_presence_penalty() const	{ return params.presence_penalty; }
	int get_mirostat() const		{ return params.mirostat; }
	float get_mirostat_tau() const	{ return params.mirostat_tau; }
	float get_mirostat_eta() const	{ return params.mirostat_eta; }
	int get_tokens_predict() const	{ return params.n_predict; }
	InstructionType get_isn_type() const	{ return isn_type; }
	std::string get_cfg_neg_prompt() const	{ return params.cfg_negative_prompt; }
	float get_cfg_scale() const			{ return params.cfg_scale; }
	std::string get_stop_string() const		{ return stop_string; }
	bool get_remove_stop_string() const		{ return remove_stop_str; }

	// Generate according to a BNF grammar.
	void set_grammar(const std::string& grammar_str);
	void set_grammar_from_file(const std::string& pathname);
	void clear_grammar();
	bool has_grammar() const		{ return grammar != nullptr; }
	std::string get_grammar_str() const	{ return grammar_s; }
	llama_grammar* get_grammar() const	{ return grammar; }

	// Get information about the current session.
	u32 tokens_session() const		{ return (u32) session_tok.size(); }
	u32 model_context_size() const	{ if (nullptr == ctx) return 0ul; return (u32) llama_n_ctx(ctx); }

	// Model loading status.
	bool is_model_loaded() const		{ return model != nullptr; }
	void force_model_load();
	void force_model_reload();

	// Get information about the currently loaded model. (These will force the model to be loaded, if it is not already.)
	u64 num_params();
	int vocab_size();
	int context_size();
	int context_size_trained();
	int num_layers();
	int embedding_dimension();

	// Options for configuring GPU acceleration. Make sure to set these before the model loads, or you'll need to force_model_reload().
	int max_devices() const;
	void set_main_gpu(int gpu_idx);
	int get_main_gpu() const;
	void set_tensor_split(const std::vector<float> percentages);
	void run_cpu_only();
	void layers_to_gpu(int nlayers);
	int get_layers_to_gpu() const;
	void load_fully_on_gpu();

	// Reset the Llama contexts.
	void reset_contexts();

	// Free the model and any context we've created
	void free();

private:
	void ensure_model_loaded();
	void ensure_grammar();
	void tokenize_cfg_prompt();
	bool remove_stop_string(std::vector<llama_token>& toks);
	void do_init(const char* model_path, int vram_gb, bool og_llama, bool is_70b);
	
	// TODO: implement the model grammar generation.
	gpt_params params;
	// TODO: static cache, if multiple Llama objects are created that use the same model, just load it once and share between class instances.
	llama_model * model;
	llama_context * ctx;
	llama_context * ctx_cfg;
	std::vector<llama_token> embd_inp;
	std::vector<llama_token> session_tok;
	std::vector<llama_token> guidance_inp;
	std::vector<llama_token> embd_guidance;
	std::vector<llama_token> last_n_tokens;
	int guidance_offset;
	int original_prompt_len;
	int keep_tok;
	std::string pfx_prompt;
	std::vector<ChatEntry> chats;
	std::string bot_name;
	std::string user_name;
	std::string char_card;
	u32 tokens_chatp;
	std::string stop_string;
	bool remove_stop_str;
	std::string chat_isn;
	std::string grammar_s;
	llama_grammar* grammar;
	InstructionType isn_type;
};

extern bool ggml_backend_is_init();
extern void free_llama_backend();

#else  // !CODEHAPPY_CUDA

/* stub class */
class Llama {
	Llama();
};
#endif  // CODEHAPPY_CUDA

// Helper functions exported even in non-CUDA builds.

/* find the last occurrence of 'needle' in the string 'haystack' (returns nullptr if none) */
extern const char* find_last_str(const char* haystack, const char* needle);

/* Retrieve multiline input from stdin: if a user-inputted line ends in a backslash '\', they can continue entering additional lines. */
extern std::string multiline_input();

/* Read a text file and return the contents as a string. restore_newlines ensures newline characters at the end of each line. */
extern std::string string_from_text_file(const std::string& path, bool restore_newlines = true);

#endif  // __LLAMA_CODEHAPPY
