/***

	llama.cpp
	
	A friendly wrapper around the ggml/llama.cpp library for loading models, tokenizing
	text, generating text or embeddings, or making chatbots.

	Chris Street, 2023

***/
#ifdef  CODEHAPPY_CUDA

Llama::Llama(const char* model_path, int vram_gb, bool og_llama, bool is_70b) {
	do_init(model_path, vram_gb, og_llama, is_70b);
}

Llama::Llama(const std::string& model_path, int vram_gb, bool og_llama, bool is_70b) {
	do_init(model_path.c_str(), vram_gb, og_llama, is_70b);
}

Llama::Llama(const ArgParse& ap, const LlamaDefaults& defaults) {
	std::string model_path;
	int iv;
	float fv;
	int vram_gb;

	ap.value_str("llama-model", model_path);
	if (model_path.empty())
		model_path = defaults.model_path;
	if (model_path.empty())
		codehappy_cerr << "error: no model pathname provided\n";

	vram_gb = defaults.vram_gb;
	if (ap.flag_present("llama-vram")) {
		iv = ap.value_int("llama-vram");
		if (iv >= 0)
			vram_gb = iv;
		else
			codehappy_cerr << "invalid VRAM GB: " << iv << "\n";
	}

	do_init(model_path.c_str(), vram_gb, defaults.og_llama, false);

	// set each parameter according to the defaults or the ArgParse values.
	if (defaults.top_k > 0)
		params.top_k = defaults.top_k;
	if (ap.flag_present("llama-top-k")) {
		 iv = ap.value_int("llama-top-k");
		 if (iv > 0)
		 	params.top_k = iv;
		 else
		 	codehappy_cerr << "invalid top-k parameter value: " << iv << "\n";
	}

	if (defaults.top_p >= 0.)
		params.top_p = defaults.top_p;
	if (ap.flag_present("llama-top-p")) {
		fv = (float) ap.value_double("llama-top-p");
		if (fv >= 0.)
			params.top_p = fv;
		else
			codehappy_cerr << "invalid top-p parameter value: " << fv << "\n";
	}

	if (defaults.temp >= 0.)
		params.temp = defaults.temp;
	if (ap.flag_present("llama-temp")) {
		fv = (float) ap.value_double("llama-temp");
		if (fv >= 0.)
			params.temp = fv;
		else
			codehappy_cerr << "invalid temperature value: " << fv << "\n";
	}

	if (defaults.rp >= 0.)
		params.repeat_penalty = defaults.rp;
	if (ap.flag_present("llama-repeat")) {
		fv = (float) ap.value_double("llama-repeat");
		if (fv >= 0.)
			params.repeat_penalty = fv;
		else
			codehappy_cerr << "invalid repetition penalty value: " << fv << "\n";
	}

	if (defaults.fp >= 0.)
		params.frequency_penalty = defaults.fp;
	if (ap.flag_present("llama-freq")) {
		fv = (float) ap.value_double("llama-freq");
		if (fv >= 0.)
			params.frequency_penalty = fv;
		else
			codehappy_cerr << "invalid frequency penalty value: " << fv << "\n";
	}

	if (defaults.mirostat >= 0)
		params.mirostat = defaults.mirostat;
	if (ap.flag_present("llama-mirostat")) {
		iv = ap.value_int("llama-mirostat");
		if (iv >= 0)
			params.mirostat = iv;
		else
			codehappy_cerr << "invalid mirostat type: " << iv << "\n";
	}

	if (defaults.miro_tau >= 0.)
		params.mirostat_tau = defaults.miro_tau;
	if (ap.flag_present("llama-tau")) {
		fv = (float) ap.value_double("llama-tau");
		if (fv >= 0.)
			params.mirostat_tau = fv;
		else
			codehappy_cerr << "invalid mirostat tau value: " << fv << "\n";
	}

	if (defaults.miro_eta >= 0.)
		params.mirostat_eta = defaults.miro_eta;
	if (ap.flag_present("llama-eta")) {
		fv = (float) ap.value_double("llama-eta");
		if (fv >= 0.)
			params.mirostat_eta = fv;
		else
			codehappy_cerr << "invalid mirostat eta value: " << fv << "\n";
	}

	if (defaults.main_gpu >= 0)
		params.main_gpu = defaults.main_gpu;
	if (ap.flag_present("llama-gpu")) {
		iv = ap.value_int("llama-gpu");
		if (iv >= 0)
			params.main_gpu = iv;
		else
			codehappy_cerr << "invalid main gpu index: " << iv << "\n";
	}

	if (defaults.layers_gpu >= 0)
		params.n_gpu_layers = defaults.layers_gpu;
	if (ap.flag_present("llama-layers")) {
		iv = ap.value_int("llama-layers");
		if (iv >= 0)
			params.n_gpu_layers = iv;
		else
			codehappy_cerr << "invalid layer count: " << iv << "\n";
	}

	if (ap.flag_present("cpuonly")) {
		params.n_gpu_layers = 0;
	}
}

void Llama::do_init(const char* model_path, int vram_gb, bool og_llama, bool is_70b) {
	log_disable();
	model = nullptr;
	ctx = nullptr;
	ctx_cfg = nullptr;
	grammar = nullptr;
	guidance_offset = 0;
	original_prompt_len = 0;
	keep_tok = 0;
	isn_type = ISN_ALPACA;
	params.model = model_path;
	params.n_ctx = (og_llama ? 2048 : 4096);
	params.n_batch = 1024;
	params.embedding = false;

	/*** Determining the default number of model layers to put on GPU. These values were selected for a 24GB VRAM card,
	     and will need to be scaled for a different card. These values can be changed by calling layers_to_gpu(),
	     run_cpu_only(), or load_fully_on_gpu() before loading the model. ***/

	/* Llama-70B is partially loaded, 40 layers, to GPU by default. */
	if (__stristr(model_path, "70b") != nullptr)
		is_70b = true;
	params.n_gpu_layers = (is_70b ? 40 : 43);

	/* If using 8 bit quantization, adjust the default number of layers accordingly. */
	if (is_70b && __stristr(model_path, "q8") != nullptr)
		params.n_gpu_layers = 22;

	/* 13 layers of 4bit-K-M Falcon-180B fits into VRAM (including context). */
	if (__stristr(model_path, "180b") != nullptr)
		params.n_gpu_layers = 13;

	if (vram_gb >= 0 && vram_gb != 24) {
		params.n_gpu_layers *= vram_gb;
		params.n_gpu_layers /= 24;
	}

	// attempt to guess the instruction rubric (if any) used by the model from the name.
	isn_type = ISN_ALPACA;
	if (!__stristr(model_path, "mistral"))
		isn_type = ISN_MISTRAL;
	if (!__stristr(model_path, "pygmalion"))
		isn_type = ISN_PYGMALION;
	if (!__stristr(model_path, "codellama"))
		isn_type = ISN_CODELLAMA;

	params.n_threads = std::max((int) std::thread::hardware_concurrency() / 2, 1);

	// Generation parameters that I prefer.
	params.temp = 0.9f;
	params.mirostat = 2;
	remove_stop_str = false;
}

Llama::~Llama() {
	free();
}

static bool __ggml_be_init = false;
bool ggml_backend_is_init() {
	return __ggml_be_init;
}

void free_llama_backend() {
	if (ggml_backend_is_init()) {
		llama_backend_free();
		__ggml_be_init = false;
	}
}

static std::string isn_rubric_opening(InstructionType isn_type) {
	switch (isn_type) {
	default:
	case ISN_ALPACA:
		break;
	case ISN_MISTRAL:
		return "<s>[INST]";
	case ISN_PYGMALION:
		return "<|system|>";
	case ISN_CODELLAMA:
		return "[INST] Write code to solve the following coding problem that obeys the constraints and passes the example test cases. Please wrap your code answer using ```:\n";
	}
	return "### Instruction: ";
}

static std::string isn_rubric_closing(InstructionType isn_type, bool trail_space) {
	switch (isn_type) {
	default:
	case ISN_ALPACA:
		// add a trailing space if we're beginning the response ourselves.
		if (trail_space)
			return "\n\n### Response: ";
		break;
	case ISN_MISTRAL:
		return "[/INST]";
	case ISN_PYGMALION:
		return "<|model|>";
	case ISN_CODELLAMA:
		return "[/INST]";
	}
	return "\n\n### Response:";
}

void Llama::ensure_model_loaded() {
	if (nullptr == model || nullptr == ctx) {
		if (!ggml_backend_is_init()) {
			llama_backend_init(params.numa);
			__ggml_be_init = true;
		}
		std::tie(model, ctx) = llama_init_from_gpt_params(params);
		//assert(params.n_ctx <= llama_n_ctx_train(ctx));
		if (params.n_ctx != llama_n_ctx_train(ctx)) {
			codehappy_cerr << "*** Warning: model was trained on context size " << llama_n_ctx_train(ctx) << "; context size parameter is " << params.n_ctx << "\n";
		}
		// model warm-up
		{
		const std::vector<llama_token> tmp = { llama_token_bos(ctx), };
		llama_eval(ctx, tmp.data(), tmp.size(), 0, params.n_threads);
		llama_reset_timings(ctx);
		}
		last_n_tokens.resize(params.n_ctx, 0);
	}
	if (nullptr == ctx_cfg && params.cfg_scale != 1.f) {
	        struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
	        ctx_cfg = llama_new_context_with_model(model, lparams);
	}
}

void Llama::reset_contexts() {
	struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
	if (ctx != nullptr)
		llama_free(ctx);
	if (ctx_cfg != nullptr)
		llama_free(ctx_cfg);
	ctx = llama_new_context_with_model(model, lparams);
	if (params.cfg_scale != 1.f) {
		ctx_cfg = llama_new_context_with_model(model, lparams);
	}
}

void Llama::set_grammar(const std::string& grammar_str) {
	clear_grammar();
	grammar_s = grammar_str;
}

void Llama::set_grammar_from_file(const std::string& pathname) {
	std::string grammar_str = string_from_text_file(pathname);
	set_grammar(grammar_str);
}

void Llama::ensure_grammar() {
	if (grammar_s.empty())
		return;
	if (grammar != nullptr) {
		llama_grammar_free(grammar);
		grammar = nullptr;
	}

	grammar_parser::parse_state parsed_grammar;
	
	parsed_grammar = grammar_parser::parse(grammar_s.c_str());
	if (parsed_grammar.rules.empty()) {
		return;
	}

	grammar_parser::print_grammar(stderr, parsed_grammar);
        {
            auto it = params.logit_bias.find(llama_token_eos(ctx));
            if (it != params.logit_bias.end() && it->second == -INFINITY) {
                codehappy_cerr << "warning: EOS token is disabled, which will cause most grammars to fail\n";
            }
        }

	std::vector<const llama_grammar_element *> grammar_rules(parsed_grammar.c_rules());
        grammar = llama_grammar_init(grammar_rules.data(), grammar_rules.size(), parsed_grammar.symbol_ids.at("root"));
}


void Llama::clear_grammar() {
	if (grammar != nullptr) {
		llama_grammar_free(grammar);
		grammar = nullptr;
	}
	grammar_s.clear();
}

int Llama::tokenize(const std::string& str, std::vector<llama_token>& out, bool add_bos, u32 max_tokens) {
	int ret;
	ensure_model_loaded();
	if (out.size() == 0) {
		out = ::llama_tokenize(ctx, str, add_bos);
		if (max_tokens > 0 && out.size() > max_tokens) {
			out.resize(max_tokens);
		}
		ret = out.size();
	} else {
		std::vector<llama_token> ltoks = ::llama_tokenize(ctx, str, add_bos);
		if (max_tokens > 0 && ltoks.size() > max_tokens) {
			ltoks.resize(max_tokens);
		}
		ret = ltoks.size();
		out.insert(out.end(), ltoks.begin(), ltoks.end());
	}
	return ret;
}

int Llama::tokenize(const char* str, std::vector<llama_token>& out, bool add_bos, u32 max_tokens) {
	std::string s = str;
	return tokenize(s, out, add_bos, max_tokens);
}

u32 Llama::token_count(const std::string& str) {
	std::vector<llama_token> tok;
	tokenize(str, tok, false);
	return (u32) tok.size();
}

u32 Llama::token_count(const char* str) {
	std::string s = str;
	return token_count(s);
}

std::string Llama::truncate_nicely_by_tokens(const std::string& str, u32 max_tokens) {
	// TODO: this could be a lot more efficient.
	if (0 == max_tokens || str.empty())
		return "";
	u32 ct = token_count(str);
	int i, best_i;
	std::string substr;
	int lc;

	if (ct <= max_tokens) {
		substr = str;
		return substr;
	}
	if (str.length() <= 1) {
		substr = str;
		return substr;
	}
	i = (int) floor((double(str.length()) * double(max_tokens) / double(ct)) + 0.5);
	i = std::min(i, (int) str.length() - 1);
	i = std::max(i, 1);
	lc = 0;
	forever {
		int j;
		substr = str.substr(0, i + 1);
		if (token_count(substr) <= max_tokens)
			break;
		j = std::max((i * 7) / 8, 1);
		if (i <= j)
			return "";
		i = j;
		++lc;
		if (lc > 100)
			return "";
	}
	best_i = i;
	lc = 0;
	forever {
		while (i < str.length() && isspace(str[i])) {
			++i;
		}
		while (i < str.length() && !isspace(str[i])) {
			++i;
		}
		if (i >= str.length())
			break;
		substr = str.substr(0, i + 1);
		if (token_count(substr) <= max_tokens)
			best_i = i;
		else
			break;
		++lc;
		if (lc > max_tokens * 32)
			break;
	}
	best_i = std::min(best_i, (int) str.length() - 1);
	substr = str.substr(0, best_i + 1);
	return substr;	
}

std::string Llama::truncate_nicely_by_tokens(const char* str, u32 max_tokens) {
	std::string s = str;
	return truncate_nicely_by_tokens(s, max_tokens);
}

void Llama::session_prompt(const std::string& str) {
	std::string str_cp = str;
	embd_inp.clear();
	session_tok.clear();
    	// (no longer needed) str_cp.insert(0, 1, ' ');
	tokenize(str_cp, session_tok, true, params.n_ctx - 4);
	tokenize_cfg_prompt();
	reset_contexts();
	embd_inp = session_tok;
}

void Llama::session_prompt(const char* str) {
	std::string s = str;
	session_prompt(s);
}

void Llama::isn_prompt(const std::string& str) {
	std::string isn = isn_rubric_opening(isn_type);
	std::string str_trunc = truncate_nicely_by_tokens(str, params.n_ctx - 100);
	isn += str_trunc;
	isn += isn_rubric_closing(isn_type, false);
	session_prompt(isn);
	keep_tok = session_tok.size();
}

void Llama::isn_prompt(const char* str) {
	std::string s = str;
	isn_prompt(s);
}

void Llama::isn_prompt(const std::string& str, const std::string& response_begin) {
	u32 ct = token_count(response_begin);
	std::string isn = isn_rubric_opening(isn_type);
	std::string str_trunc = truncate_nicely_by_tokens(str, params.n_ctx - 100 - ct);
	isn += str_trunc;
	isn += isn_rubric_closing(isn_type, true);
	isn += response_begin;
	session_prompt(isn);
	keep_tok = session_tok.size();
}

void Llama::isn_prompt(const char* str, const char* response_begin) {
	std::string s1 = str, s2 = response_begin;
	isn_prompt(s1, s2);
}

void Llama::prefix_prompt(const std::string& str) {
	pfx_prompt = str;
	session_prompt(str);
}

void Llama::prefix_prompt(const char* str) {
	std::string s = str;
	prefix_prompt(s);
}

void Llama::add_text(const std::string& str) {
	tokenize(str, session_tok, false);
	tokenize(str, embd_inp, false);
}

void Llama::add_text(const char* str) {
	std::string s = str;
	add_text(s);
}

void Llama::tokenize_cfg_prompt() {
	if (ctx_cfg != nullptr) {
		std::string str_cp = params.cfg_negative_prompt;
        	// (no longer needed) str_cp.insert(0, 1, ' ');
        	guidance_inp = ::llama_tokenize(ctx_cfg, str_cp, true);
        	original_prompt_len = embd_inp.size();
        	guidance_offset = (int)guidance_inp.size() - original_prompt_len;
	}
}

std::string Llama::text_from_tokens(const std::vector<llama_token>& toks) const {
	std::string out;
	for (const auto tok : toks) {
		out += llama_token_to_str(ctx, tok);
	}
	return out;
}

void Llama::generate_tokens(std::vector<llama_token>& toks_out, bool echo, LlamaCallback clback) {
	const int nctx = params.n_ctx - 4;
	int nremain = get_tokens_predict(), npast = 0, npast_guidance = 0;
	std::vector<llama_token> embd_guidance;

	ensure_model_loaded();
	ensure_grammar();
	if (embd_inp.size() > nctx) {
		embd_inp.erase(embd_inp.begin(), embd_inp.begin() + embd_inp.size() - nctx);
	}

	std::vector<llama_token_data> candidates;
	auto n_vocab = llama_n_vocab(ctx);
	candidates.reserve(n_vocab);

	for (int i = 0; i < embd_inp.size(); ++i) {
		last_n_tokens.erase(last_n_tokens.begin());
		last_n_tokens.push_back(embd_inp[i]);
	}

	while (nremain != 0) {
		if (embd_inp.size() > 0) {
			// if the context has filled, let's move things up (keeping the original instruction, if present.) 
			if (npast + (int) embd_inp.size() + std::max<int>(0, guidance_offset) > nctx) {
				const int n_left = npast - keep_tok;
				npast = std::max(1, keep_tok);
				npast_guidance = std::max(1, keep_tok + guidance_offset);
				embd_inp.insert(embd_inp.begin(), last_n_tokens.begin() + nctx - n_left / 2 - embd_inp.size(), last_n_tokens.end() - embd_inp.size());
			}

			// evaluate classifier-free guidance prompt, if present.
			if (ctx_cfg != nullptr) {
				int input_size = 0;
				llama_token* input_buf = NULL;
				if (npast_guidance < (int) guidance_inp.size()) {
					embd_guidance = guidance_inp;
					if (embd_inp.begin() + original_prompt_len < embd_inp.end()) {
						embd_guidance.insert(
							embd_guidance.end(),
							embd_inp.begin() + original_prompt_len,
							embd_inp.end()
						);
					}
					input_buf = embd_guidance.data();
					input_size = embd_guidance.size();
				} else {
					input_buf = embd_inp.data();
					input_size = embd_inp.size();
				}
				for (int i = 0; i < input_size; i += params.n_batch) {
					int n_eval = std::min(input_size - i, params.n_batch);
					if (llama_eval(ctx_cfg, input_buf + i, n_eval, npast_guidance, params.n_threads)) {
						fprintf(stderr, "%s : failed to eval\n", __func__);
						exit(1);
					}
					npast_guidance += n_eval;
				}
			}

			// run the tokens that haven't been evaluated through the transformer.
			for (int i = 0; i < (int) embd_inp.size(); i += params.n_batch) {
				int n_eval = (int) embd_inp.size() - i;
				if (n_eval > params.n_batch) {
					n_eval = params.n_batch;
				}
				if (llama_eval(ctx, &embd_inp[i], n_eval, npast, params.n_threads)) {
					fprintf(stderr, "%s : failed to eval\n", __func__);
					exit(1);
				}
				npast += n_eval;
			}
		}

        	embd_inp.clear();
		embd_guidance.clear();

		llama_token id = llama_sample_token(ctx, ctx_cfg, grammar, params, last_n_tokens, candidates);

		last_n_tokens.erase(last_n_tokens.begin());
		last_n_tokens.push_back(id);
		if (clback != nullptr) {
			clback(llama_token_to_str(ctx, id).c_str());
		}

		--nremain;
		toks_out.push_back(id);
		embd_inp.push_back(id);
		session_tok.push_back(id);
		if (id == llama_token_eos(ctx))
			break;
		if (echo) {
			fprintf(stdout, "%s", llama_token_to_str(ctx, id).c_str());
			fflush(stdout);
		}
		if (remove_stop_string(toks_out)) {
			remove_stop_string(embd_inp);
			remove_stop_string(session_tok);
			nremain = 0;
			break;
		}
	}
}

bool Llama::remove_stop_string(std::vector<llama_token>& toks) {
	if (toks.empty() || stop_string.empty())
		return false;
	std::string detok = text_from_tokens(toks);
	const char* w;
	w = strstr(detok.c_str(), stop_string.c_str());

	if (!is_null(w)) {
		if (remove_stop_str) {
			bool needs_bos = (toks[0] == llama_token_bos(ctx));
			w = find_last_str(detok.c_str(), stop_string.c_str());
			assert(!is_null(w));
			std::string substr = detok.substr(0, w - detok.c_str());
			toks.clear();
			toks = ::llama_tokenize(ctx, substr, needs_bos);
		}
		return true;
	} else {
		w = strstr(detok.c_str(), "\n:");
		if (w != nullptr) {
			if (remove_stop_str) {
				bool needs_bos = (toks[0] == llama_token_bos(ctx));
				w = find_last_str(detok.c_str(), "\n:");
				assert(!is_null(w));
				std::string substr = detok.substr(0, w - detok.c_str());
				toks.clear();
				toks = ::llama_tokenize(ctx, substr, needs_bos);
			}
			return true;
		}
	}
	return false;
}

void Llama::generate_tokens(std::vector<llama_token>& toks_out, int max_tokens, bool echo, LlamaCallback clback) {
	set_tokens_predict(max_tokens);
	generate_tokens(toks_out, echo, clback);
}

std::string Llama::generate_tokens(int max_tokens, bool echo, LlamaCallback clback) {
	std::vector<llama_token> toks_out;
	generate_tokens(toks_out, max_tokens, echo, clback);
	return text_from_tokens(toks_out);
}

std::string Llama::session_text() {
	return text_from_tokens(session_tok);
}

void Llama::free() {
	if (ctx != nullptr)
		llama_free(ctx);
	if (ctx_cfg != nullptr)
		llama_free(ctx_cfg);
	if (model != nullptr)
		llama_free_model(model);
	ctx = nullptr;
	ctx_cfg = nullptr;
	model = nullptr;
	remove_stop_str = false;
	bot_name.clear();
	user_name.clear();
	char_card.clear();
	stop_string.clear();
	chats.clear();
	pfx_prompt.clear();
	clear_grammar();
}

void Llama::chat_session(const std::string& char_card, const std::string& bn, const std::string& un, const std::string& bot_greeting) {
	u32 greet_size = 0;
	bot_name = bn;
	user_name = un;
	if (!bot_greeting.empty())
		greet_size = token_count(bot_greeting);
	chat_isn = isn_rubric_opening(isn_type);
	chat_isn += " A chat between ";
	chat_isn += bot_name;
	chat_isn += " and ";
	chat_isn += user_name;
	chat_isn += ". They respond back and forth, and each response begins with their name and a colon, for example:\n\n";
	chat_isn += bot_name;
	chat_isn += ": ";
	if (bot_greeting.empty()) {
		chat_isn += "Good morning!\n";
	} else {
		chat_isn += bot_greeting;
		chat_isn += "\n";
	}
	chat_isn += user_name;
	chat_isn += ": Good morning, how are you doing?\n\n";
	std::string char_card_trunc = truncate_nicely_by_tokens(char_card, params.n_ctx - token_count(chat_isn) - greet_size - 100);
	chat_isn += char_card_trunc;
	chat_isn += isn_rubric_closing(isn_type, false);
	chat_isn += "\n";
	keep_tok = token_count(chat_isn);
	chats.clear();
	if (!bot_greeting.empty()) {
		ChatEntry ce(this, bot_name, bot_greeting);
		chats.push_back(ce);
	}
	stop_string = "\n";
	stop_string += user_name;
	stop_string += ":";
	remove_stop_str = true;
}

void Llama::chat_user_response(const std::string& response) {
	ChatEntry ce(this, user_name, response);
	chats.push_back(ce);
}

void Llama::multichat_user_response(const std::string& user, const std::string& response) {
	ChatEntry ce(this, user, response);
	chats.push_back(ce);
}

std::string Llama::chat_response() {
	std::string fullisn = chat_isn;
	int istart, tok_total = (int) token_count(chat_isn);

	istart = chats.size() - 1;
	while (istart >= 0) {
		tok_total += (int) chats[istart].tokens;
		if (tok_total > params.n_ctx - 200)
			break;
		--istart;
	}
	++istart;

	for (int e = istart; e < chats.size(); ++e) {
		fullisn += chats[e].persona;
		fullisn += ": ";
		fullisn += chats[e].response;
		fullisn += "\n";
	}
	fullisn += bot_name;
	fullisn += ":";

	session_prompt(fullisn);

	std::string response = generate_tokens(-1);
	// if the generation begins with multiple spaces, remove a leading space
	if (response.length() > 2 && response[0] == ' ' && response[1] == ' ')
		response.erase(0, 1);
	ChatEntry ce(this, bot_name, response);
	chats.push_back(ce);
	return response;
}

std::string Llama::chat_history() {
	std::string ret;
	for (const auto& ce : chats) {
		ret += ce.persona;
		ret += ": ";
		ret += ce.response;
		ret += "\n";
	}
	return ret;
}

void Llama::chat_rewind() {
	if (chats.empty())
		return;
	chats.pop_back();
}

void Llama::force_model_load() {
	ensure_model_loaded();
}

void Llama::force_model_reload() {
	free();
	force_model_load();
}

u64 Llama::num_params() {
	ensure_model_loaded();
	return llama_model_n_params(model);
}

int Llama::vocab_size() {
	ensure_model_loaded();
	return llama_model_n_vocab(model);
}

int Llama::context_size() {
	ensure_model_loaded();
	return llama_model_n_ctx(model);
}

int Llama::context_size_trained() {
	ensure_model_loaded();
	return llama_model_n_ctx_train(model);
}

int Llama::num_layers() {
	ensure_model_loaded();
	return llama_model_n_layer(model);
}

int Llama::embedding_dimension() {
	ensure_model_loaded();
	return llama_n_embd(ctx);
}

int Llama::max_devices() const {
	return llama_max_devices();
}

void Llama::set_main_gpu(int gpu_idx) {
	params.main_gpu = gpu_idx;
}

int Llama::get_main_gpu() const {
	return params.main_gpu;
}

void Llama::set_tensor_split(const std::vector<float> percentages) {
	if (percentages.size() > max_devices()) {
		codehappy_cerr << "*** Error: tensor split contains more proportions than available devices (num. of devices available: " << max_devices() << ")\n";
		return;
	}
	float sum = 0.0f;
	for (int e = 0; e < percentages.size(); ++e) {
		sum += percentages[e];
		if (percentages[e] < 0.) {
			codehappy_cerr << "*** Error: tensor split proportion less than 0?\n";
			return;
		}
	}
	if (sum == 0.) {
		codehappy_cerr << "*** Error: tensor split proportions sum to 0?\n";
		return;
	}
	for (int e = 0; e < percentages.size(); ++e) {
		params.tensor_split[e] = percentages[e];
	}
	for (; e < max_devices(); ++e) {
		params.tensor_split[e] = 0.0f;
	}
}

void Llama::run_cpu_only() {
	params.n_gpu_layers = 0;
}

void Llama::layers_to_gpu(int nlayers) {
	params.n_gpu_layers = nlayers;
}

int Llama::get_layers_to_gpu() const {
	return params.n_gpu_layers;
}

void Llama::load_fully_on_gpu() {
	// should be safe up to Llama-90T or so
	layers_to_gpu(99999);
}

LlamaEmbedding* Llama::embedding_for_prompt(const std::string& str) {
	LlamaEmbedding* ret = new LlamaEmbedding;
	embedding_for_prompt(str, ret);
	return ret;
}

void Llama::embedding_for_prompt(const std::string& str, LlamaEmbedding* le) {
	ship_assert(le != nullptr);
	if (!params.embedding) {
		codehappy_cerr << "be sure to enable embeddings before calculating them!\n";
		exit(1);
	}
	ensure_model_loaded();
	reset_contexts();
	le->free();

	std::vector<llama_token> toks;
	toks = ::llama_tokenize(ctx, str, true);
	if (toks.size() > context_size_trained()) {
		// prompt is too long: truncate it according to the model's context window size.
		toks.erase(toks.begin() + context_size_trained(), toks.end());
	}

	int npast = 0;
	while (!toks.empty()) {
		int ntok = std::min(params.n_batch, (int) toks.size());
		if (llama_eval(ctx, toks.data(), ntok, npast, params.n_threads)) {
			codehappy_cerr << "Error in evaluation\n";
			exit(1);
		}
		npast += ntok;
		toks.erase(toks.begin(), toks.begin() + ntok);
	}

	const float* embeds = llama_get_embeddings(ctx);
	if (is_null(embeds)) {
		codehappy_cerr << "Null embedding array?\n";
		exit(1);
	}
	le->copy_from_array(llama_n_embd(ctx), embeds);
}

LlamaEmbeddingFile* Llama::embeddings_for_file(const std::string& str, int n_tok) {
	LlamaEmbeddingFile* ret = new LlamaEmbeddingFile;
	embeddings_for_file(str, ret, n_tok);
	return ret;
}

void Llama::embeddings_for_file(const std::string& str, LlamaEmbeddingFile* lef, int n_tok) {
	ship_assert(lef != nullptr);
	if (n_tok <= 0)
		n_tok = context_size_trained() / 2;

	lef->free();
	lef->pathname = str;
	std::string content = string_from_text_file(str);

	std::vector<llama_token> toks;
	tokenize(content, toks);
	int e = 0;
	int offs = 0;
	while (e < toks.size()) {
		std::vector<llama_token> toks_iter;
		LlamaEmbedding* le;

		for (int f = e; f < e + n_tok && f < toks.size(); ++f) {
			toks_iter.push_back(toks[f]);
		}

		std::string s = text_from_tokens(toks_iter);

		lef->embeds.push_back(embedding_for_prompt(s));
		lef->offsets.push_back(offs);

		offs += s.length();
		e += n_tok;
	}
}

LlamaEmbeddingFolder* Llama::embeddings_for_folder(const std::string& path, int n_tok) {
	LlamaEmbeddingFolder* ret = new LlamaEmbeddingFolder;
	embeddings_for_folder(path, ret, n_tok);
	return ret;
}

void Llama::embeddings_for_folder(const std::string& path, LlamaEmbeddingFolder* lef, int n_tok) {
	DIR* di = opendir(path.c_str());
	dirent* entry;
	/* Text file extensions we're supporting. */
	const char* extensions[] = {
		".txt", ".html", ".htm", ".c", ".cpp", ".cxx", ".py"
	};

	while (entry = readdir(di)) {
		const char* w;
		bool ok = false;
		for (const auto ext : extensions) {
			w = strstr(entry->d_name, ext);
			if (!is_null(w)) {
				ok = true;
				break;
			}
		}
		if (!ok)
			continue;
			
		std::string filename;
		make_pathname(path, entry->d_name, filename);
		std::cout << filename << std::endl;
		lef->files.push_back(embeddings_for_file(filename, n_tok));
	}
	closedir(di);
}

ChatEntry::ChatEntry(Llama* l, const std::string& p, const std::string& r) {
	std::string entry;
	persona = p;
	response = r;
	entry = persona;
	entry += ": ";
	entry += response;
	entry += "\n";
	tokens = l->token_count(entry);
}

LlamaEmbedding::LlamaEmbedding() {
	n_embed = 0;
	embed_data = nullptr;
}

LlamaEmbedding::~LlamaEmbedding() {
	free();
}

void LlamaEmbedding::free() {
	if (embed_data != nullptr)
		delete embed_data;
	n_embed = 0;
	embed_data = nullptr;
}

double LlamaEmbedding::cosine_similarity(const LlamaEmbedding* le) const {
	double cos_val;

	NOT_NULL_OR_RETURN(embed_data, 0.0);
	ship_assert(!is_null(le));

	cos_val = dot_product(le) / (magnitude() * le->magnitude());
	return cos_val;
}

double LlamaEmbedding::magnitude() const {
	double ret = 0.;
	NOT_NULL_OR_RETURN(embed_data, 0.0);

	for (int e = 0; e < n_embed; ++e) {
		ret += (embed_data[e] * embed_data[e]);
	}

	return sqrt(ret);
}

double LlamaEmbedding::dot_product(const LlamaEmbedding* le) const {
	double ret = 0.;

	NOT_NULL_OR_RETURN(embed_data, 0.0);
	ship_assert(!is_null(le));
	ship_assert(n_embed == le->n_embed);

	for (int e = 0; e < n_embed; ++e) {
		ret += (embed_data[e] * le->embed_data[e]);
	}

	return ret;
}

void LlamaEmbedding::copy_from_array(int n_el, const float* array) {
	ship_assert(n_el >= 0);
	ship_assert(array != nullptr);
	free();
	embed_data = new float [n_el];
	for (int e = 0; e < n_el; ++e) {
		embed_data[e] = array[e];
	}
	n_embed = n_el;
}

void LlamaEmbedding::out_to_ramfile(RamFile* rf) {
	rf->put32(n_embed);
	for (int e = 0; e < n_embed; ++e)
		rf->putfloat(embed_data[e]);
}

void LlamaEmbedding::in_from_ramfile(RamFile* rf) {
	free();
	n_embed = rf->get32();
	embed_data = new float [n_embed];
	for (int e = 0; e < n_embed; ++e)
		embed_data[e] = rf->getfloat();
}

LlamaEmbeddingFile::LlamaEmbeddingFile() {
}

LlamaEmbeddingFile::~LlamaEmbeddingFile() {
	free();
}

void LlamaEmbeddingFile::free() {
	pathname.clear();
	offsets.clear();
	for (auto le : embeds)
		delete le;
	embeds.clear();
}

void LlamaEmbeddingFile::out_to_ramfile(RamFile* rf) {
	rf->putstring(pathname);
	ship_assert(embeds.size() == offsets.size());
	rf->put32((i32) embeds.size());
	for (int e = 0; e < embeds.size(); ++e) {
		embeds[e]->out_to_ramfile(rf);
		rf->put32(offsets[e]);
	}
}

void LlamaEmbeddingFile::in_from_ramfile(RamFile* rf) {
	i32 sz;
	free();
	pathname = rf->getstring();
	sz = rf->get32();
	for (i32 e = 0; e < sz; ++e) {
		LlamaEmbedding* le = new LlamaEmbedding;
		le->in_from_ramfile(rf);
		embeds.push_back(le);
		offsets.push_back(rf->get32());
	}
}

int LlamaEmbeddingFile::best_match(const LlamaEmbedding* le, double* score) {
	double best_sc = -2.;
	int iret = -1;
	
	for (int e = 0; e < embeds.size(); ++e) {
		double sc = embeds[e]->cosine_similarity(le);
		if (sc > best_sc) {
			best_sc = sc;
			iret = e;
		}
	}

	if (score != nullptr)
		*score = best_sc;
	return iret;
}

int LlamaEmbeddingFile::count_embeddings() const {
	return embeds.size();
}

LlamaEmbeddingFolder::LlamaEmbeddingFolder() {
}

LlamaEmbeddingFolder::~LlamaEmbeddingFolder() {
	free();
}

void LlamaEmbeddingFolder::out_to_ramfile(RamFile* rf) {
	rf->put32((i32) files.size());
	for (auto f : files)
		f->out_to_ramfile(rf);
}

void LlamaEmbeddingFolder::in_from_ramfile(RamFile* rf) {
	free();
	i32 nf = rf->get32();
	for (i32 e = 0; e < nf; ++e) {
		LlamaEmbeddingFile* lef = new LlamaEmbeddingFile;
		lef->in_from_ramfile(rf);
		files.push_back(lef);
	}
}

void LlamaEmbeddingFolder::free() {
	for (auto f : files)
		delete f;
	files.clear();
}

int LlamaEmbeddingFolder::count_files() const {
	return files.size();
}

int LlamaEmbeddingFolder::count_embeddings() const {
	int ret = 0;
	for (const auto* f : files)
		ret += f->count_embeddings();
	return ret;
}

int LlamaEmbeddingFolder::best_match(int file_idx, const LlamaEmbedding* le, double* score) {
	if (file_idx < 0 || file_idx >= files.size())
		return -1;
	return files[file_idx]->best_match(le, score);
}

LlamaDefaults::LlamaDefaults() {
	top_k = -1;
	top_p = -1.0f;
	temp = -1.0f;
	rp = -1.0f;
	fp = -1.0f;
	// note that, when using ArgParse + default arguments to initialize a model, that
	// mirostat sampling is off by default, since it's a command line option
	mirostat = 0;
	miro_tau = -1.f;
	miro_eta = -1.f;
	main_gpu = -1;
	cpuonly = false;
	layers_gpu = -1;
	vram_gb = 24;
	og_llama = false;
};

LlamaDefaults llama_defaults;

void llama_args(ArgParse& ap) {
	ap.add_argument("llama-model", type_string, "path to the .GGUF-format large language model");
	ap.add_argument("llama-top-k", type_int, "top k (most likely) parameter for sampling");
	ap.add_argument("llama-top-p", type_double, "top p (cumulative probability) parameter for sampling");
	ap.add_argument("llama-temp", type_double, "temperature for large language model sampling");
	ap.add_argument("llama-repeat", type_double, "repetition penalty for large language model sampling");
	ap.add_argument("llama-freq", type_double, "frequency penalty for large language model sampling");
	ap.add_argument("llama-mirostat", type_int, "type of mirostat sampling to use (0 = none)");
	ap.add_argument("llama-tau", type_double, "tau parameter for mirostat sampling");
	ap.add_argument("llama-eta", type_double, "eta parameter for mirostat sampling");
	ap.add_argument("llama-gpu", type_int, "index of the main GPU to use for large language model inference");
	ap.add_argument("cpuonly", type_none, "run large language model inference CPU only");
	ap.add_argument("llama-layers", type_int, "number of Llama layers to load onto GPU for inference");
	ap.add_argument("llama-vram", type_int, "use this many GB of VRAM to determine default number of layers loaded to gpu");
}

#endif  // CODEHAPPY_CUDA

/* helper functions */
const char* find_last_str(const char* haystack, const char* needle) {
	const char* w = haystack + strlen(haystack);
	const size_t findlen = strlen(needle);
	while (w >= haystack) {
		if (!strncmp(w, needle, findlen))
			return w;
		--w;
	}
	return nullptr;
}

std::string multiline_input() {
	std::string ret, response;
	const char* w;
	bool more;
	do {
		more = false;
		std::getline(std::cin, response);
		if (response.empty())
			break;
		w = response.c_str() + response.length() - 1;
		while (w >= response.c_str()) {
			if (!isspace(*w))
				break;
			--w;
		}
		if (w >= response.c_str() && *w == '\\') {
			response = response.substr(0, w - response.c_str());
			response += "\n";
			more = true;
		}
		ret += response;
	} while (more);
	return ret;
}

std::string string_from_text_file(const std::string& path, bool restore_newlines) {
	std::string ret;
	std::string line;
	std::ifstream i;
	
	i.open(path);
	forever {
		std::getline(i, line);
		if (i.eof())
			break;
		ret += line;
		if (restore_newlines)
			ret += "\n";
	}
	i.close();
	i.clear();

	return ret;
}

/* end llama.cpp */
