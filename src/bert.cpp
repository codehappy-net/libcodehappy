/***

	bert.cpp
	
	BERT sentence embedding manager. Models with this architecture, such as bge-large,
	are better for semantic search than Llama models.

	Chris Street, 2023

***/

BertEmbeddingManager::BertEmbeddingManager(const std::string& model_path) {
	model = bert_load_from_file(model_path.c_str());
	assert(not_null(model));
	n_threads = std::max((int) std::thread::hardware_concurrency() / 2, 1);
	save_text = true;
	n_sentences = 1;
}

BertEmbeddingManager::~BertEmbeddingManager() {
	if (model != nullptr) {
		bert_free(model);
	}
	model = nullptr;
}

std::vector<LMEmbedding*> BertEmbeddingManager::embedding_for_text(const std::string& str) {
	std::vector<LMEmbedding*> ret;
	std::vector<u32> offs;
	embedding_for_text(str, ret, offs);
	return ret;
}

void BertEmbeddingManager::embedding_for_text(const std::string& str, std::vector<LMEmbedding*>& le) {
	std::vector<u32> offs;
	embedding_for_text(str, le, offs);
}

// helper to skip over ellipses in the text we're breaking into sentences. 
static char* end_of_ellipsis(char* wn, char* we) {
	char* w = wn;
	// account for the people that put spaces in their ellipses....
	while (!strncmp(w, ". .", 3)) {
		w += 2;
	}
	while (w < we) {
		++w;
		if (*w != '.')
			break;
	}

	return w;
}

static bool check_abbrev(char* w, char* wn, char* we) {
	const char* checks[] = { "Mr.", "Mrs.", "Ms.", "Dr.", "approx.", "etc.", "dept.", "vs.", "misc.", 
		"apt.", "viz.", "cf.", "est.", "num.", "Ave.", "Blvd.", "St.", "Rd.", "Asst.", "Inc.", "et al.",
		"ibid.", "Sr.", "Sra.", "Jr.", "Prof.", "Mme.", "Mmse.", "Rev.", };

	for (const auto check : checks) {
		int l = strlen(check);
		if (wn - l + 1 < w)
			continue;
		if (!strncmp(wn - l + 1, check, l))
			return true;
	}

	// check for acronyms
	if ((wn - 2) >= w && *(wn - 2) == '.')
		return true;
	if ((wn - 2) >= w && isspace(*(wn - 2)) && (wn + 2) < we && *(wn + 2) == '.')
		return true;


	return false;
}

/*** break a C string, in place, into sentences. ***/
void sentencify(char* ntxt, std::vector<char*>* sentences) {
	char* w, *we, *wn;
	bool seen_alpha = false;

	w = ntxt;
	we = w + strlen(w);
	while (w < we && isspace(*w))
		++w;
	wn = w;
	while (wn < we) {
		bool sentence_end = false;

		if (isalpha(*wn))
			seen_alpha = true;

		if (*wn == '.') {
			char* wn2 = end_of_ellipsis(wn, we);
			// check for titles (Mrs., Dr., etc.), acronyms, and common abbreviations
			bool is_abbrev = check_abbrev(w, wn, we);
			if (wn == wn2 - 1 && !is_abbrev) {
				// period, this is the end of a sentence
				sentence_end = true;
				if (isspace(*(wn + 1))) {
					do {
						++wn;
					} while (wn < we && (isspace(*wn) || strchr("\"'", *wn) != nullptr));
					if (wn < we)
						--wn;
				}
			} else {
				wn = wn2;
				continue;
			}
		}

		if (strchr("?!;|[]{}", *wn) != nullptr) {
			while (wn < we && (strchr("?!;|[]{}\"'", *wn) != nullptr || isspace(*wn))) {
				++wn;
			}
			if (wn < we)
				--wn;
			sentence_end = true;
		}


		if (sentence_end) {
			*wn = '\000';
			if (seen_alpha && !strncmp(w, "\" \"", 3))
				w += 3;
			if (seen_alpha && !strncmp(w, "\"  \"", 4))
				w += 4;
			if (seen_alpha && !strncmp(w, "' '", 3))
				w += 3;
			if (seen_alpha && !strncmp(w, "'  '", 4))
				w += 4;
			if (seen_alpha && !strncmp(w, ") ", 2))
				w += 2;
			if (seen_alpha && !strncmp(w, "'  ", 3))
				w += 3;
			if (seen_alpha && !strncmp(w, "' ", 2))
				w += 2;
			if (w != wn && seen_alpha) {
				if (not_null(sentences))
					sentences->push_back(w);
			}
			w = wn + 1;
			while (w < we && isspace(*w))
				++w;
			seen_alpha = false;
			wn = w;
		} else {
			wn++;
		}
	}
	// get the last sentence.
	if (seen_alpha) {
		if (not_null(sentences))
			sentences->push_back(w);
	}
}

static bool ends_in_punc(const char* str) {
	const char *w;
	w = str + strlen(str) - 1;
	while (w >= str) {
		if (isspace(*w) || *w == '\'' || *w == '\"') {
			--w;
			continue;
		}
		if (strchr(".?!;|[]{}", *w) != nullptr)
			return true;
		break;
	}
	return false;
}

void BertEmbeddingManager::embedding_for_text(const std::string& str, std::vector<LMEmbedding*>& le, std::vector<u32>& offs) {
	/* We break the text into blocks/chunks of n_sentence sentences, and then create an LMEmbedding for each chunk. */
	std::vector<char*> sentences;
	char* ntxt = normalize_string(str);

	sentencify(ntxt, &sentences);

	n_sentences = std::max(n_sentences, 1);

	for (int i = 0; i < sentences.size(); i += n_sentences) {
		std::string full;
		for (int e = i; e < (i + n_sentences) && e < sentences.size(); ++e) {
			if (e != i)
				full += " ";
			full += sentences[e];
			if (!ends_in_punc(sentences[e]))
				full += ".";
		}
		/* remove excess whitespace */
		p_replace(full, "  ", " ");
		p_replace(full, "  ", " ");

		LMEmbedding* lme = new LMEmbedding;
		lme->n_embed = embedding_dimension();
		lme->embed_data = new float [lme->n_embed];	
		if (save_text) {
			lme->text = new char [full.length() + 1];
			strcpy(lme->text, full.c_str());
		}
		bert_encode(model, n_threads, full.c_str(), lme->embed_data);
		le.push_back(lme);
		offs.push_back(sentences[i] - ntxt);
	}

	delete ntxt;
}

LMEmbeddingFile* BertEmbeddingManager::embeddings_for_file(const std::string& str) {
	LMEmbeddingFile* ret = new LMEmbeddingFile;
	embeddings_for_file(str, ret);
	return ret;
}

void BertEmbeddingManager::embeddings_for_file(const std::string& str, LMEmbeddingFile* lef) {
	std::string text = string_from_text_file(str);
 	lef->free();
	lef->pathname = str;

	embedding_for_text(text, lef->embeds, lef->offsets);
}

LMEmbeddingFolder* BertEmbeddingManager::embeddings_for_folder(const std::string& path) {
	LMEmbeddingFolder* ret = new LMEmbeddingFolder;
	embeddings_for_folder(path, ret);
	return ret;
}

void BertEmbeddingManager::embeddings_for_folder(const std::string& path, LMEmbeddingFolder* lef) {
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
		lef->files.push_back(embeddings_for_file(filename));
	}
	closedir(di);
}

char* BertEmbeddingManager::normalize_string(const std::string& in_str) const {
	// A copy of the text is made, in which newlines are converted to spaces. Excess whitespace is removed
	// when concatenating sentences into embedding chunks.
	// The bert ggml code removes accents and does some other normalizations when it processes the input text.
	char* ret = new char[in_str.length() + 1];
	strcpy(ret, in_str.c_str());
	p_replace(ret, "\n", " ");
	p_replace(ret, "\r", " ");
	p_replace(ret, "“", "\"");
	p_replace(ret, "”", "\"");
	return ret;
}

/*** end bert.cpp ***/
