/***

	llama.cpp
	
	A friendly wrapper around the ggml/llama.cpp library for loading models, tokenizing
	text, generating text or embeddings, or making chatbots.

	Chris Street, 2023

***/
Llama::Llama(const char* model_path, int vram_gb, bool og_llama, bool is_70b) {
	do_init(model_path, vram_gb, og_llama, is_70b);
}

Llama::Llama(const std::string& model_path, int vram_gb, bool og_llama, bool is_70b) {
	do_init(model_path.c_str(), vram_gb, og_llama, is_70b);
}

Llama::Llama(const ArgParse& ap, const LlamaDefaults& defaults) {
	std::string model_path, mmproj_path;
	int iv;
	float fv;
	int vram_gb;

	ap.value_str("llama-model", model_path);
	if (model_path.empty())
		model_path = defaults.model_path;
	if (model_path.empty())
		codehappy_cerr << "error: no model pathname provided\n";

	ap.value_str("llama-mmproj", mmproj_path);
	if (mmproj_path.empty())
		mmproj_path = defaults.mmproj_path;

	vram_gb = defaults.vram_gb;
	if (ap.flag_present("llama-vram")) {
		iv = ap.value_int("llama-vram");
		if (iv >= 0)
			vram_gb = iv;
		else
			codehappy_cerr << "invalid VRAM GB: " << iv << "\n";
	}

	do_init(model_path.c_str(), vram_gb, defaults.og_llama, false);

	if (!mmproj_path.empty()) {
		ctx_clip = clip_model_load(mmproj_path.c_str(), /*verbosity=*/ 0);
		ctx_llava = new llava_context;
		ctx_llava->ctx_llama = nullptr;
		ctx_llava->ctx_clip = ctx_clip;
		ctx_llava->model = nullptr;
	}

	// set each parameter according to the defaults or the ArgParse values.
	if (defaults.top_k > 0)
		params.sparams.top_k = defaults.top_k;
	if (ap.flag_present("llama-top-k")) {
		 iv = ap.value_int("llama-top-k");
		 if (iv > 0)
		 	params.sparams.top_k = iv;
		 else
		 	codehappy_cerr << "invalid top-k parameter value: " << iv << "\n";
	}

	if (defaults.top_p >= 0.)
		params.sparams.top_p = defaults.top_p;
	if (ap.flag_present("llama-top-p")) {
		fv = (float) ap.value_double("llama-top-p");
		if (fv >= 0.)
			params.sparams.top_p = fv;
		else
			codehappy_cerr << "invalid top-p parameter value: " << fv << "\n";
	}

	if (defaults.temp >= 0.)
		params.sparams.temp = defaults.temp;
	if (ap.flag_present("llama-temp")) {
		fv = (float) ap.value_double("llama-temp");
		if (fv >= 0.)
			params.sparams.temp = fv;
		else
			codehappy_cerr << "invalid temperature value: " << fv << "\n";
	}

	if (defaults.rp >= 0.)
		params.sparams.penalty_repeat = defaults.rp;
	if (ap.flag_present("llama-repeat")) {
		fv = (float) ap.value_double("llama-repeat");
		if (fv >= 0.)
			params.sparams.penalty_repeat = fv;
		else
			codehappy_cerr << "invalid repetition penalty value: " << fv << "\n";
	}

	if (defaults.fp >= 0.)
		params.sparams.penalty_freq = defaults.fp;
	if (ap.flag_present("llama-freq")) {
		fv = (float) ap.value_double("llama-freq");
		if (fv >= 0.)
			params.sparams.penalty_freq = fv;
		else
			codehappy_cerr << "invalid frequency penalty value: " << fv << "\n";
	}

	if (defaults.mirostat >= 0)
		params.sparams.mirostat = defaults.mirostat;
	if (ap.flag_present("llama-mirostat")) {
		iv = ap.value_int("llama-mirostat");
		if (iv >= 0)
			params.sparams.mirostat = iv;
		else
			codehappy_cerr << "invalid mirostat type: " << iv << "\n";
	}

	if (defaults.miro_tau >= 0.)
		params.sparams.mirostat_tau = defaults.miro_tau;
	if (ap.flag_present("llama-tau")) {
		fv = (float) ap.value_double("llama-tau");
		if (fv >= 0.)
			params.sparams.mirostat_tau = fv;
		else
			codehappy_cerr << "invalid mirostat tau value: " << fv << "\n";
	}

	if (defaults.miro_eta >= 0.)
		params.sparams.mirostat_eta = defaults.miro_eta;
	if (ap.flag_present("llama-eta")) {
		fv = (float) ap.value_double("llama-eta");
		if (fv >= 0.)
			params.sparams.mirostat_eta = fv;
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

	if (defaults.context_size > 0)
		params.n_ctx = defaults.context_size;
	if (ap.flag_present("llama-context")) {
		iv = ap.value_int("llama-context");
		if (iv > 0)
			params.n_ctx = iv;
		else
			codehappy_cerr << "invalid context window size: " << iv << "\n";
	}

	if (ap.flag_present("llama-rubric")) {
		std::string rubric_name;
		InstructionType it;
		ap.value_str("llama-rubric", rubric_name);
		if (!rubric_name.empty() && isdigit(rubric_name[0])) {
			iv = atoi(rubric_name.c_str());
			if (iv >= (int) ISN_MAX)
				codehappy_cerr << "invalid rubric index: " << iv << " (maximum permissable is " << ((int)ISN_MAX) - 1 << ")\n";
			else
				isn_type = (InstructionType) iv;		
		} else {
			it = isn_rubric_from_string(rubric_name);
			if (it == ISN_INVALID)
				codehappy_cerr << "unknown isn rubric identifier: " << rubric_name << "\n";
			else
				isn_type = it;
		}
	}

	if (ap.flag_present("llama-system")) {
		ap.value_str("llama-system", isn_system);
	}

	if (ap.flag_present("cpuonly")) {
		params.n_gpu_layers = 0;
	}
}

void Llama::do_init(const char* model_path, int vram_gb, bool og_llama, bool is_70b) {
	int bparam = 0;
	int quant = 8;
	int layers4;
	bool moe = false;
	log_disable();
	model = nullptr;
	ctx = nullptr;
	ctx_cfg = nullptr;
	ctx_clip = nullptr;
	ctx_llava = nullptr;
	img_embed = nullptr;
	guidance_offset = 0;
	original_prompt_len = 0;
	keep_tok = 0;
	isn_type = ISN_ALPACA;
	params.model = model_path;
	params.n_ctx = (og_llama ? 2048 : 4096);
	params.n_batch = 1024;
	params.embedding = false;

	/*** Determining the default number of model layers to put on GPU. These values were selected for one 24GB VRAM card,
	     and will need to be scaled for a different card. These values can be changed by calling layers_to_gpu(),
	     run_cpu_only(), or load_fully_on_gpu() before loading the model. ***/
	const char* paramstrs[] = { "180b", "175b", "120b", "80b", "72b", "70b", "67b", "65b", "40b", "34b", "33b", "30b", "20b", "15b", "13b", "11b", "7b", "3b" };
	const char* quantstr[] = { "q8", "q6", "q5", "q4", "q3", "q2" };

	if (is_70b)
		bparam = 70;
	for (auto ps : paramstrs) {
		if (__stristr(model_path, ps) != nullptr) {
			bparam = atoi(ps);
			break;
		}
	}
	for (auto qs : quantstr) {
		if (__stristr(model_path, qs) != nullptr) {
			quant = atoi(qs + 1);
			break;
		}
	}

	moe = (__stristr(model_path, "mixtral") != nullptr);
	if (__stristr(model_path, "phi-2") != nullptr) {
		bparam = 3;
	}
	if (__stristr(model_path, "llava") != nullptr) {
		bparam = 7;
	}

	switch (bparam) {
	case 180:
	case 175:
		layers4 = 13;
		break;
	case 120:
		layers4 = 44;
		break;
	case 80:
		layers4 = 28;
		break;
	case 72:
	case 70:
		layers4 = 44;
		break;
	case 67:
	case 65:
		layers4 = 52;
		break;
	case 40:
		layers4 = 60;
		break;
	case 34:
	case 33:
	case 30:
		layers4 = 68;
		break;
	case 20:
		layers4 = 100;
		break;
	case 15:
	case 13:
	case 11:
		layers4 = 140;
		break;
	case 7:
		layers4 = 200;
		if (moe) {
			layers4 = 28;
		}
		break;
	case 3:
	default:
		layers4 = 300;
		break;
	}

	params.n_gpu_layers = (layers4 * 4) / quant;

	if (vram_gb >= 0 && vram_gb != 24) {
		params.n_gpu_layers *= vram_gb;
		params.n_gpu_layers /= 24;
	}

	// attempt to guess the instruction rubric (if any) used by the model from the name.
	InstructionType it = isn_rubric_from_model_name(model_path);
	if (it != ISN_INVALID)
		isn_type = it;

	// sometimes context size is included in the file name as well
	if (__stristr(model_path, "-200k") != nullptr)
		params.n_ctx = 200000;
	if (__stristr(model_path, "-100k") != nullptr)
		params.n_ctx = 100000;
	if (__stristr(model_path, "8192") != nullptr || __stristr(model_path, "-8k") != nullptr)
		params.n_ctx = 8192;
	if (__stristr(model_path, "-16k") != nullptr)
		params.n_ctx = 16384;
	if (__stristr(model_path, "-32k") != nullptr)
		params.n_ctx = 32768;
	if (__stristr(model_path, "-64k") != nullptr)
		params.n_ctx = 65536;
	if (__stristr(model_path, "miqu") != nullptr || __stristr(model_path, "senku") != nullptr)
		params.n_ctx = 32764;	// note: not 32K exactly, but this strange number.

	params.n_threads = std::max((int) std::thread::hardware_concurrency() / 2, 1);

	// Generation parameters that I prefer.
	params.sparams.temp = 0.9f;
	params.sparams.mirostat = 2;
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

std::string Llama::isn_rubric_opening() const {
	std::string sys_prompt = isn_system_prompt();
	std::string ret;

	switch (isn_type) {
	default:
	case ISN_ALPACA:
		if (!sys_prompt.empty()) {
			sys_prompt += "\n### Instruction: ";
			return sys_prompt;
		}
		break;

	case ISN_CUSTOM:
		if (sys_prompt.empty()) {
			return isn_opening;
		}
		sys_prompt += "\n";
		sys_prompt += isn_opening;
		return sys_prompt;

	case ISN_ALPACA_SYS:
	case ISN_DEEPSEEK_CODER:
		if (sys_prompt.empty())
			return "### Instruction: ";
		sys_prompt += "\n### Instruction: ";
		return sys_prompt;

	case ISN_MISTRAL:
		return "<s> [INST]";

	case ISN_PYGMALION:
		return "<|system|>";

	case ISN_CODELLAMA:
		ret = "[INST] ";
		ret += sys_prompt;
		ret += "\n";
		return ret;

	case ISN_CHATML:
	case ISN_MONADGPT:
		if (sys_prompt.empty()) {
			return "<|im_start|>system\n";
		}
		ret = "<|im_start|>system\n";
		ret += sys_prompt;
		ret += "<|im_end|>\n<|im_start|>user\n";
		return ret;

	case ISN_VICUNA:
	case ISN_ORCA:
		if (sys_prompt.empty()) {
			return "USER: ";
		}
		ret = "SYSTEM: ";
		ret += sys_prompt;
		ret += "\nUSER: ";
		return ret;

	case ISN_TULU:
		return "<|user|>\n";

	case ISN_LLAMA2CHAT:
		ret = "[INST] <<SYS>>\n";
		ret += sys_prompt;
		ret += "\n<</SYS>>\n";
		return ret;

	case ISN_HUMAN_ASSISTANT:
		return "Human: ";

	case ISN_USER_ASSISTANT:
		return "User: ";

	case ISN_GUANACO:
		return "### Human: ";

	case ISN_ZEPHYR:
		if (!sys_prompt.empty()) {
			ret = "<|system|>\n";
			ret += sys_prompt;
			ret += "\n</s>\n<|user|>\n";
			return ret;
		}
		return "<|system|>\n</s>\n<|user|>\n";

	case ISN_PHIND:
		if (sys_prompt.empty()) {
			return "### User Message\n";
		}
		ret = "### System Prompt\n";
		ret += sys_prompt;
		ret += "\n### User Message\n";
		return ret;

	case ISN_ORCA_HASHES:
		if (sys_prompt.empty()) {
			return "### User:\n";
		}
		ret = "### System:\n";
		ret += sys_prompt;
		ret += "\n### User:\n";
		return ret;

	case ISN_XWINCODER:
		if (sys_prompt.empty()) {
			return "<user>: ";
		}
		ret = "<system>: ";
		ret += sys_prompt;
		ret += "\n<user>: ";
		return ret;

	case ISN_PHI2:
		return "Instruct: ";
	}
	return "### Instruction: ";
}

std::string Llama::isn_rubric_closing(bool trail_space) const {
	switch (isn_type) {
	default:
	case ISN_ALPACA:
	case ISN_ALPACA_SYS:
	case ISN_DEEPSEEK_CODER:
		// add a trailing space if we're beginning the response ourselves.
		if (trail_space)
			return "\n\n### Response: ";
		break;
	case ISN_CUSTOM:
		return isn_closing;
	case ISN_MISTRAL:
		return " [/INST]";
	case ISN_CODELLAMA:
	case ISN_LLAMA2CHAT:
		return "[/INST]";
	case ISN_PYGMALION:
		return "<|model|>";
	case ISN_CHATML:
	case ISN_MONADGPT:
		return "<|im_end|>\n<|im_start|>assistant\n";
	case ISN_VICUNA:
	case ISN_ORCA:
		if (trail_space)
			return "\nASSISTANT: ";
		return "\nASSISTANT:";
	case ISN_TULU:
		return "\n<|assistant|>\n";
	case ISN_HUMAN_ASSISTANT:
	case ISN_USER_ASSISTANT:
		if (trail_space)
			return "\nAssistant: ";
		return "\nAssistant:";
	case ISN_GUANACO:
	case ISN_ORCA_HASHES:
		if (trail_space)
			return "\n### Assistant: ";
		return "\n### Assistant:";
	case ISN_ZEPHYR:
		return "</s>\n<|assistant|>\n";
	case ISN_PHIND:
		return "\n### Assistant\n";
	case ISN_XWINCODER:
		if (trail_space)
			return "\n<AI>: ";
		return "\n<AI>:";
	case ISN_PHI2:
		return "\nOutput:";
	}
	return "\n\n### Response:";
}

void Llama::set_custom_isn_rubric(const std::string& custom_isn_opening, const std::string& custom_isn_closing) {
	isn_opening = custom_isn_opening;
	isn_closing = custom_isn_closing;
	isn_type = ISN_CUSTOM;
}

bool Llama::uses_system_prompt() const {
	switch (isn_type) {
	case ISN_ALPACA:
	case ISN_ALPACA_SYS:
	case ISN_CHATML:
	case ISN_CODELLAMA:
	case ISN_LLAMA2CHAT:
	case ISN_VICUNA:
	case ISN_ORCA:
	case ISN_MONADGPT:
	case ISN_DEEPSEEK_CODER:
	case ISN_ZEPHYR:
	case ISN_PHIND:
	case ISN_ORCA_HASHES:
	case ISN_XWINCODER:
		return true;

	case ISN_CUSTOM:
		// permit using system prompts with custom rubrics -- they're prepended to the instruction, along with a newline.
		// other custom system prompts can be worked into the rubric prefix or suffix as needed.
		return true;
	}

	// Pygmalion rubric has something called a 'system' prompt but doesn't distinguish between it and a user prompt,
	// so for our purposes they don't support a separate system message.
	return false;
}

std::string Llama::isn_system_prompt() const {
	if (!uses_system_prompt())
		return "";

	// User-supplied system prompt always takes precedence.
	if (!isn_system.empty())
		return isn_system;

	// Default system prompts, for the instruction rubrics that accept them.
	switch (isn_type) {
	case ISN_ALPACA_SYS:
		break;
	case ISN_ALPACA:
	case ISN_CHATML:
	case ISN_VICUNA:
	case ISN_ZEPHYR:
	case ISN_ORCA_HASHES:
	case ISN_CUSTOM:
		// These *can* use a system message, but none is supplied by default.
		return "";
	case ISN_CODELLAMA:
		// default CodeLlama instruction.
		return "Write code to solve the following coding problem that obeys the constraints and passes the example test cases. Please wrap your code answer using ```:";
	case ISN_LLAMA2CHAT:
		// a more appropriate Llama 2 chat system prompt.
		return "Follow instructions faithfully and helpfully to the best of your ability.";
	case ISN_MONADGPT:
		// this is the system message it was trained with, so this is what you want to use in most applications.
		return "You are MonadGPT, a very old chatbot from the 17th century. Please answer the questions using an archaic language";

	case ISN_DEEPSEEK_CODER:
		// DeepSeek Coder-Instruct default system instruction.
		return "You are an AI programming assistant, utilizing the DeepSeek Coder model, developed by DeepSeek Company, and you only answer questions related to computer science. For politically sensitive questions, security and privacy issues, and other non-computer science questions, you will refuse to answer.";

	case ISN_ORCA:
		return "Follow the user instructions faithfully and helpfully to the best of your ability.";

	case ISN_PHIND:
		// default Phind LlamaCoder system message
		return "You are an intelligent programming assistant.";

	case ISN_XWINCODER:
		// default for XwinCoder
		return "You are an AI coding assistant that helps people with programming. Write a response that appropriately completes the user's request.";
	}

	// This is the Alpaca default system prompt, it's also a fairly safe default since lots of people train on Alpaca examples.
	return "Below is an instruction that describes a task. Write a response that appropriately completes the request.\n";
}

static std::string __rubric_names[] = {
	"alpaca", "alpaca-system", "mistral", "pygmalion", "codellama", "chatml", "vicuna", "monadgpt",
	"tulu", "orca", "llama2chat", "human-assistant", "user-assistant", "deepseek-coder", "guanaco",
	"zephyr", "phind", "orca-hashes", "xwincoder", "phi2",
};

std::string Llama::isn_rubric_name(InstructionType it) {
	switch (it) {
	case ISN_CUSTOM:
		return "custom";
	case ISN_INVALID:
		return "invalid";
	}

	int iv = (int) it;
	if (iv < 0 || iv >= (int) ISN_MAX)
		return "invalid";
	return __rubric_names[iv];
}

InstructionType Llama::isn_rubric_from_string(const std::string& s) {
	for (int i = 0; i < (int) ISN_MAX; ++i) {
		if (!__stricmp(__rubric_names[i].c_str(), s.c_str()))
			return (InstructionType) i;
	}

	return ISN_INVALID;
}

InstructionType Llama::isn_rubric_from_model_name(const char * s) const {
	InstructionType it = ISN_INVALID;
	/* note that order is important in the following array -- original airoboros models used Vicuna
	   isn rubric, while airoboros-l2 models use Llama 2 Chat rubric, for example. Although ISN_ALPACA
	   is the default and doesn't 'need' to be checked here, I've included several Alpaca models here
	   just to document their rubrics have been verified. */
	const std::pair<std::string, InstructionType> rubrics[] = {
		{ "alpaca", ISN_ALPACA }, { "goliath", ISN_ALPACA }, { "psyfighter", ISN_ALPACA }, { "xwin", ISN_ALPACA },
		{ "mistral", ISN_MISTRAL }, { "pygmalion", ISN_PYGMALION }, { "codellama", ISN_CODELLAMA },
		{ "llama-2", ISN_LLAMA2CHAT }, { "hermes", ISN_CHATML }, { "capyb", ISN_VICUNA }, { "vicuna", ISN_VICUNA },
		{ "orca-2", ISN_CHATML }, { "monadgpt", ISN_MONADGPT }, { "tulu", ISN_TULU }, { "tess", ISN_ORCA },
		{ "lzlv", ISN_ALPACA_SYS }, { "airoboros", ISN_VICUNA }, { "airoboros-l2", ISN_LLAMA2CHAT },
		{ "guanaco", ISN_GUANACO }, { "meditron", ISN_CHATML }, { "chronomaid", ISN_ALPACA_SYS },
		{ "tigerbot", ISN_ALPACA_SYS }, { "zephyr", ISN_ZEPHYR }, { "yi-34b", ISN_HUMAN_ASSISTANT },
		{ "yi-6b", ISN_HUMAN_ASSISTANT }, { "phind", ISN_PHIND }, { "pivot", ISN_ALPACA }, { "dolphin", ISN_CHATML },
		{ "openorca", ISN_CHATML }, { "gorilla", ISN_VICUNA }, { "mythochronos", ISN_ALPACA_SYS },
		{ "saiga", ISN_CHATML }, { "noromaid", ISN_ALPACA_SYS }, { "mythomist", ISN_ALPACA },
		{ "mythomax", ISN_ALPACA_SYS }, { "synatra", ISN_CHATML }, { "causallm", ISN_CHATML },
		{ "chupacabra", ISN_ORCA_HASHES }, { "valiant", ISN_LLAMA2CHAT }, { "falcon", ISN_USER_ASSISTANT },
		{ "spicyboros", ISN_VICUNA }, { "synthia", ISN_ORCA }, { "xwincoder", ISN_XWINCODER },
		{ "wizardmath", ISN_ALPACA_SYS }, { "wizardlm", ISN_VICUNA }, { "mythalion", ISN_ALPACA_SYS },
		{ "platypus", ISN_ALPACA_SYS }, { "beluga", ISN_ORCA_HASHES }, { "euryale", ISN_ALPACA_SYS },
		{ "amethyst", ISN_ALPACA_SYS }, { "agentlm", ISN_LLAMA2CHAT }, { "lemur", ISN_CHATML },
		{ "capybara-tess", ISN_ORCA }, { "mixtral", ISN_MISTRAL }, { "phi-2", ISN_PHI2 },
		{ "miqu", ISN_MISTRAL }, { "senku", ISN_MISTRAL },
	};
	// Goliath does fine taking either ISN_ALPACA or ISN_VICUNA, does one perform better?

	for (const auto& p : rubrics) {
		if (__stristr(s, p.first.c_str()) != nullptr)
			it = p.second;
	}

	if (__stristr(s, "deepseek") != nullptr) {
		if (__stristr(s, "coder") != nullptr) {
			// DeepSeek Coder-Instruct
			it = ISN_DEEPSEEK_CODER;
		} else {
			// DeepSeek chat
			it = ISN_USER_ASSISTANT;
		}
	}

	return it;
}

void Llama::ensure_model_loaded() {
	if (nullptr == model || nullptr == ctx) {
		if (!ggml_backend_is_init()) {
			llama_backend_init(params.numa);
			__ggml_be_init = true;
		}
		std::tie(model, ctx) = llama_init_from_gpt_params(params);
		if (params.n_ctx != llama_n_ctx_train(model)) {
			codehappy_cerr << "*** Warning: model was trained on context size " << llama_n_ctx_train(model) << "; context size parameter is " << params.n_ctx << "\n";
		}
		last_n_tokens.resize(params.n_ctx, 0);
	}
	if (nullptr == ctx_cfg && params.sparams.cfg_scale != 1.f) {
	        struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
	        ctx_cfg = llama_new_context_with_model(model, lparams);
	}
	if (ctx_llava != nullptr) {
		ctx_llava->ctx_llama = ctx;
		ctx_llava->ctx_clip = ctx_clip;
		ctx_llava->model = model;
	}
}

void Llama::reset_contexts() {
	struct llama_context_params lparams = llama_context_params_from_gpt_params(params);
	if (ctx != nullptr)
		llama_free(ctx);
	if (ctx_cfg != nullptr)
		llama_free(ctx_cfg);
	ctx = llama_new_context_with_model(model, lparams);
	if (params.sparams.cfg_scale != 1.f) {
		ctx_cfg = llama_new_context_with_model(model, lparams);
	}
	if (ctx_llava != nullptr) {
	    ctx_llava->ctx_llama = ctx;
	    ctx_llava->ctx_clip = ctx_clip;
	    ctx_llava->model = model;
	}
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
	isn_mmodal.clear();
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
	std::string isn = isn_rubric_opening();
	std::string str_trunc = truncate_nicely_by_tokens(str, params.n_ctx - 100);
	isn += str_trunc;
	isn += isn_rubric_closing(false);
	session_prompt(isn);
	keep_tok = session_tok.size();
}

void Llama::isn_prompt(const char* str) {
	std::string s = str;
	isn_prompt(s);
}

void Llama::isn_prompt(const std::string& str, const std::string& response_begin) {
	u32 ct = token_count(response_begin);
	std::string isn = isn_rubric_opening();
	std::string str_trunc = truncate_nicely_by_tokens(str, params.n_ctx - 100 - ct);
	isn += str_trunc;
	isn += isn_rubric_closing(true);
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
		std::string str_cp = params.sparams.cfg_negative_prompt;
        	// (no longer needed) str_cp.insert(0, 1, ' ');
        	guidance_inp = ::llama_tokenize(ctx_cfg, str_cp, true);
        	original_prompt_len = embd_inp.size();
        	guidance_offset = (int)guidance_inp.size() - original_prompt_len;
	}
}

std::string Llama::text_from_tokens(const std::vector<llama_token>& toks) const {
	std::string out;
	for (const auto tok : toks) {
		out += llama_token_to_piece(ctx, tok);
	}
	return out;
}

void Llama::generate_tokens(std::vector<llama_token>& toks_out, bool echo, LlamaCallback clback, bool insert_bos) {
	const int nctx = params.n_ctx - 4;
	int nremain = get_tokens_predict(), npast = 0, npast_guidance = 0;
	std::vector<llama_token> embd_guidance;

	ensure_model_loaded();
	if (!isn_mmodal.empty()) {
		// handle inference with an embedded image
		generate_llava(toks_out, get_tokens_predict(), echo, clback, insert_bos);
		return;
	}

	if (insert_bos && (embd_inp.empty() || embd_inp[0] != llama_token_bos(model))) {
		embd_inp.insert(embd_inp.begin(), llama_token_bos(model));
	}
	if (embd_inp.size() > nctx) {
		embd_inp.erase(embd_inp.begin(), embd_inp.begin() + embd_inp.size() - nctx);
	}

	std::vector<llama_token_data> candidates;
	auto n_vocab = llama_n_vocab(model);
	candidates.reserve(n_vocab);

	for (int i = 0; i < embd_inp.size(); ++i) {
		last_n_tokens.erase(last_n_tokens.begin());
		last_n_tokens.push_back(embd_inp[i]);
	}

	struct llama_sampling_context* ctx_sampling = llama_sampling_init(params.sparams);

	while (nremain != 0) {
		if (embd_inp.size() > 0) {
			// if the context has filled, let's move things up (keeping the original instruction, if present.) 
			if (npast + (int) embd_inp.size() + std::max<int>(0, guidance_offset) > nctx) {
				const int n_left = npast - keep_tok - 1;
				const int n_discard = n_left / 2;
				llama_kv_cache_seq_rm(ctx, 0, keep_tok + 1, keep_tok + n_discard + 1);
				llama_kv_cache_seq_shift(ctx, 0, keep_tok + 1 + n_discard, npast, -n_discard);
				npast -= n_discard;
				if (ctx_cfg != nullptr)
					npast_guidance -= n_discard;
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
					if (llama_decode(ctx_cfg, llama_batch_get_one(input_buf + i, n_eval, npast_guidance, 0))) {
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
				if (llama_decode(ctx, llama_batch_get_one(&embd_inp[i], n_eval, npast, 0))) {
					fprintf(stderr, "%s : failed to eval\n", __func__);
					exit(1);
				}
				npast += n_eval;
			}
		}

        	embd_inp.clear();
		embd_guidance.clear();

		llama_token id = llama_sampling_sample(ctx_sampling, ctx, ctx_cfg);
		llama_sampling_accept(ctx_sampling, ctx, id, true);

		last_n_tokens.erase(last_n_tokens.begin());
		last_n_tokens.push_back(id);
		if (clback != nullptr) {
			clback(llama_token_to_piece(ctx, id).c_str());
		}

		--nremain;
		toks_out.push_back(id);
		embd_inp.push_back(id);
		session_tok.push_back(id);
		if (id == llama_token_eos(model))
			break;
		if (echo) {
			fprintf(stdout, "%s", llama_token_to_piece(ctx, id).c_str());
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
			bool needs_bos = (toks[0] == llama_token_bos(model));
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
				bool needs_bos = (toks[0] == llama_token_bos(model));
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

void Llama::generate_tokens(std::vector<llama_token>& toks_out, int max_tokens, bool echo, LlamaCallback clback, bool insert_bos) {
	set_tokens_predict(max_tokens);
	generate_tokens(toks_out, echo, clback, insert_bos);
}

std::string Llama::generate_tokens(int max_tokens, bool echo, LlamaCallback clback, bool insert_bos) {
	std::vector<llama_token> toks_out;
	generate_tokens(toks_out, max_tokens, echo, clback, insert_bos);
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
	if (ctx_clip != nullptr)
		clip_free(ctx_clip);
	if (ctx_llava != nullptr)
		delete ctx_llava;
	if (model != nullptr)
		llama_free_model(model);
	if (img_embed != nullptr)
		llava_image_embed_free(img_embed);
	ctx = nullptr;
	ctx_cfg = nullptr;
	ctx_clip = nullptr;
	ctx_llava = nullptr;
	img_embed = nullptr;
	model = nullptr;
	remove_stop_str = false;
	bot_name.clear();
	user_name.clear();
	char_card.clear();
	stop_string.clear();
	chats.clear();
	pfx_prompt.clear();
}

void Llama::chat_session(const std::string& char_card, const std::string& bn, const std::string& un, const std::string& bot_greeting) {
	u32 greet_size = 0;
	bot_name = bn;
	user_name = un;
	if (!bot_greeting.empty())
		greet_size = token_count(bot_greeting);
	chat_isn = isn_rubric_opening();
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
	chat_isn += isn_rubric_closing(false);
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
	return llama_n_vocab(model);
}

int Llama::context_size() {
	ensure_model_loaded();
	return llama_n_ctx(ctx);
}

int Llama::context_size_trained() {
	ensure_model_loaded();
	return llama_n_ctx_train(model);
}

int Llama::num_layers() {
	ensure_model_loaded();
	return llama_n_layer(model);
}

int Llama::embedding_dimension() {
	ensure_model_loaded();
	return llama_n_embd(model);
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

LMEmbedding* Llama::embedding_for_prompt(const std::string& str) {
	LMEmbedding* ret = new LMEmbedding;
	embedding_for_prompt(str, ret);
	return ret;
}

void Llama::embedding_for_prompt(const std::string& str, LMEmbedding* le) {
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
		if (llama_decode(ctx, llama_batch_get_one(toks.data(), ntok, npast, 0))) {
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
	le->copy_from_array(llama_n_embd(model), embeds);
}

LMEmbeddingFile* Llama::embeddings_for_file(const std::string& str, int n_tok) {
	LMEmbeddingFile* ret = new LMEmbeddingFile;
	embeddings_for_file(str, ret, n_tok);
	return ret;
}

void Llama::embeddings_for_file(const std::string& str, LMEmbeddingFile* lef, int n_tok) {
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
		LMEmbedding* le;

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

LMEmbeddingFolder* Llama::embeddings_for_folder(const std::string& path, int n_tok) {
	LMEmbeddingFolder* ret = new LMEmbeddingFolder;
	embeddings_for_folder(path, ret, n_tok);
	return ret;
}

void Llama::embeddings_for_folder(const std::string& path, LMEmbeddingFolder* lef, int n_tok) {
	DIR* di = opendir(path.c_str());
	dirent* entry;

	while (entry = readdir(di)) {
		const char* w;
		bool ok = is_text_file_extension(entry->d_name);
		if (!ok)
			continue;
			
		std::string filename;
		make_pathname(path, entry->d_name, filename);
		std::cout << filename << std::endl;
		lef->files.push_back(embeddings_for_file(filename, n_tok));
	}
	closedir(di);
}

bool Llama::embed_image_path(const std::string& image_pathname) {
	// we need a multimodal projector model for this operation.
	if (is_null(ctx_llava)) {
		codehappy_cerr << "*** error: image embedding requires a multimodal projection model.\n";
		return false;
	}
	if (img_embed != nullptr) {
		llava_image_embed_free(img_embed);
		img_embed = nullptr;
	}

	img_embed = llava_image_embed_make_with_filename(ctx_llava->ctx_clip, params.n_threads, image_pathname.c_str());

	return img_embed != nullptr;
}

bool Llama::embed_image(SBitmap* bmp) {
	char* path = temp_file_name(".png");
	bool ret;
	bmp->save_bmp(path);
	ret = embed_image_path(path);
	remove(path);
	delete path;
	return ret;
}

void Llama::multimodal_image_prompt(const std::string& str) {
	if (is_null(img_embed)) {
		codehappy_cerr << "*** error: we need an embedded image to do a multimodal image prompt\n";
		return;
	}
	ensure_model_loaded();
	reset_contexts();
	isn_mmodal = str;
}

void Llama::multimodal_image_prompt(const std::string& str, const std::string& image_path) {
	if (!embed_image_path(image_path))
		return;
	multimodal_image_prompt(str);
}

void Llama::multimodal_image_prompt(const std::string& str, SBitmap* bmp) {
	if (!embed_image(bmp))
		return;
	multimodal_image_prompt(str);
}

/* LLaVA helper functions */
static bool llava_eval_tokens(struct llama_context * ctx_llama, std::vector<llama_token> tokens, int n_batch, int * n_past) {
    int N = (int) tokens.size();
    for (int i = 0; i < N; i += n_batch) {
        int n_eval = (int) tokens.size() - i;
        if (n_eval > n_batch) {
            n_eval = n_batch;
        }
        if (llama_decode(ctx_llama, llama_batch_get_one(&tokens[i], n_eval, *n_past, 0))) {
            fprintf(stderr, "%s : failed to eval. token %d/%d (batch size %d, n_past %d)\n", __func__, i, N, n_batch, *n_past);
            return false;
        }
        *n_past += n_eval;
    }
    return true;
}

static bool llava_eval_id(struct llama_context * ctx_llama, int id, int * n_past) {
    std::vector<llama_token> tokens;
    tokens.push_back(id);
    return llava_eval_tokens(ctx_llama, tokens, 1, n_past);
}

// this one uses most the same rigamarole logic as generate_tokens(), should unify the sampling code
static llama_token llava_sample_id(llama_context * ctx_llama, gpt_params & params) {
    auto & sparams = params.sparams;

    // out of user input, sample next token
    const float   temp      = sparams.temp;
    const int32_t top_k     = sparams.top_k <= 0 ? llama_n_vocab(llama_get_model(ctx_llama)) : sparams.top_k;
    const float   top_p     = sparams.top_p;
    const float   tfs_z     = sparams.tfs_z;
    const float   typical_p = sparams.typical_p;
    const int     mirostat     = sparams.mirostat;
    const float   mirostat_tau = sparams.mirostat_tau;
    const float   mirostat_eta = sparams.mirostat_eta;

    llama_token id = 0;
    {
        auto logits  = llama_get_logits(ctx_llama);
        auto n_vocab = llama_n_vocab(llama_get_model(ctx_llama));

        // Apply params.logit_bias map
        for (auto it = sparams.logit_bias.begin(); it != sparams.logit_bias.end(); it++) {
            logits[it->first] += it->second;
        }

        std::vector<llama_token_data> candidates;
        candidates.reserve(n_vocab);
        for (llama_token token_id = 0; token_id < n_vocab; token_id++) {
            candidates.emplace_back(llama_token_data{token_id, logits[token_id], 0.0f});
        }

        llama_token_data_array candidates_p = { candidates.data(), candidates.size(), false };

        if (temp <= 0) {
              // Greedy sampling
            id = llama_sample_token_greedy(ctx_llama, &candidates_p);
        } else {
            if (mirostat == 1) {
                static float mirostat_mu = 2.0f * mirostat_tau;
                const  int mirostat_m    = 100;
                llama_sample_temp(ctx_llama, &candidates_p, temp);
                id = llama_sample_token_mirostat(ctx_llama, &candidates_p, mirostat_tau, mirostat_eta, mirostat_m, &mirostat_mu);
            } else if (mirostat == 2) {
                static float mirostat_mu = 2.0f * mirostat_tau;
                llama_sample_temp(ctx_llama, &candidates_p, temp);
                id = llama_sample_token_mirostat_v2(ctx_llama, &candidates_p, mirostat_tau, mirostat_eta, &mirostat_mu);
            } else {
                  // Temperature sampling
                llama_sample_top_k(ctx_llama, &candidates_p, top_k, 1);
                llama_sample_tail_free(ctx_llama, &candidates_p, tfs_z, 1);
                llama_sample_typical(ctx_llama, &candidates_p, typical_p, 1);
                llama_sample_top_p(ctx_llama, &candidates_p, top_p, 1);
                llama_sample_temp(ctx_llama, &candidates_p, temp);
                id = llama_sample_token(ctx_llama, &candidates_p);
            }
        }
    }

    return id;
}

static const char * llava_sample(struct llama_context * ctx_llama, gpt_params & params, int * n_past, llama_token* tok_out) {
    int id = llava_sample_id(ctx_llama, params);
    static std::string ret;
    if (id == llama_token_eos(llama_get_model(ctx_llama))) {
        ret = "</s>";
    } else {
        ret = llama_token_to_piece(ctx_llama, id);
    }
    llava_eval_id(ctx_llama, id, n_past);
    if (tok_out != nullptr)
    	*tok_out = id;
    return ret.c_str();
}

static bool llava_eval_string(struct llama_context * ctx_llama, const char* str, int n_batch, int * n_past, bool add_bos) {
	std::string str2 = str;
	std::vector<llama_token> embd_inp = ::llama_tokenize(ctx_llama, str2, add_bos);
	llava_eval_tokens(ctx_llama, embd_inp, n_batch, n_past);
	return true;
}

void Llama::generate_llava(std::vector<llama_token>& toks_out, int max_tokens, bool echo, LlamaCallback clback, bool insert_bos) {
	int n_past = 0;
	const int max_tgt_len = (max_tokens < 0 ? 512 : max_tokens);

	insert_bos = llama_should_add_bos_token(model);
	if (is_null(img_embed)) {
		codehappy_cerr << "*** error: multimodal image prompt without an embedded image?\n";
		return;
	}

	// llava chat format is "<system_prompt>\nUSER:<image_embeddings>\n<textual_prompt>\nASSISTANT:"
	llava_eval_string(ctx_llava->ctx_llama, "A chat between a curious human and an artificial intelligence assistant.  The assistant gives helpful, detailed, and polite answers to the human's questions.\nUSER:", params.n_batch, &n_past, insert_bos);
	llava_eval_image_embed(ctx_llava->ctx_llama, img_embed, params.n_batch, &n_past);
	llava_eval_string(ctx_llava->ctx_llama, (isn_mmodal + "\nASSISTANT:").c_str(), params.n_batch, &n_past, false);

	for (int i = 0; i < max_tgt_len; i++) {
		llama_token tok_out;
		const char * tmp = llava_sample(ctx_llava->ctx_llama, params, &n_past, &tok_out);
		if (!strcmp(tmp, "</s>"))
			break;
		toks_out.push_back(tok_out);
		if (echo) {
			printf("%s", tmp);
			fflush(stdout);
		}
	}

	isn_mmodal.clear();
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
	context_size = -1;
};

LlamaDefaults llama_defaults;

void llama_args(ArgParse& ap) {
	ap.add_argument("llama-model", type_string, "path to the .GGUF-format large language model");
	ap.add_argument("llama-mmproj", type_string, "path to the .GGUF-format multimodal projector model (if needed)");
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
	ap.add_argument("llama-context", type_int, "the number of tokens in the context window for this model");
	ap.add_argument("llama-rubric", type_string, "identifier for the rubric used by the model");
	ap.add_argument("llama-system", type_string, "specify a custom system prompt (for rubrics that use system prompts)");
}

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

bool is_text_file_extension(const char* pathname) {
	/* Text file extensions we're supporting. */
	const char* extensions[] = {
		".txt", ".html", ".htm", ".c", ".cpp", ".cxx", ".py", ".md", ".me"
	};

	for (const auto ext : extensions) {
		const char* w = strstr(pathname, ext);
		if (!is_null(w)) {
			return true;
		}
	}

	return false;
}

/* end llama.cpp */
