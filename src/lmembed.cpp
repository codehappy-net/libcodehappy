/***

	lmembed.cpp
	
	Structs and classes for handling language model embeddings. Good for
	semantic search, retrieval-augmented generation, etc.

	2023, Chris Street

***/

LMEmbedding::LMEmbedding() {
	n_embed = 0;
	embed_data = nullptr;
	text = nullptr;
}

LMEmbedding::~LMEmbedding() {
	free();
}

void LMEmbedding::free() {
	if (embed_data != nullptr)
		delete embed_data;
	if (text != nullptr)
		delete text;
	n_embed = 0;
	embed_data = nullptr;
	text = nullptr;
}

double LMEmbedding::cosine_similarity(const LMEmbedding* le) const {
	double cos_val;

	NOT_NULL_OR_RETURN(embed_data, -2.0);
	NOT_NULL_OR_RETURN(le, -2.0);

	cos_val = dot_product(le) / (magnitude() * le->magnitude());
	return cos_val;
}

double LMEmbedding::magnitude() const {
	double ret = 0.;
	NOT_NULL_OR_RETURN(embed_data, 0.0);

	for (int e = 0; e < n_embed; ++e) {
		ret += (embed_data[e] * embed_data[e]);
	}

	return sqrt(ret);
}

double LMEmbedding::dot_product(const LMEmbedding* le) const {
	double ret = 0.;

	NOT_NULL_OR_RETURN(embed_data, 0.0);
	ship_assert(!is_null(le));
	ship_assert(n_embed == le->n_embed);

	for (int e = 0; e < n_embed; ++e) {
		ret += (embed_data[e] * le->embed_data[e]);
	}

	return ret;
}

void LMEmbedding::copy_from_array(int n_el, const float* array) {
	ship_assert(n_el >= 0);
	ship_assert(array != nullptr);
	free();
	embed_data = new float [n_el];
	for (int e = 0; e < n_el; ++e) {
		embed_data[e] = array[e];
	}
	n_embed = n_el;
}

void LMEmbedding::out_to_ramfile(RamFile* rf) {
	rf->put32(n_embed);
	for (int e = 0; e < n_embed; ++e)
		rf->putfloat(embed_data[e]);
	rf->putstring(text);
}

void LMEmbedding::in_from_ramfile(RamFile* rf) {
	free();
	n_embed = rf->get32();
	embed_data = new float [n_embed];
	for (int e = 0; e < n_embed; ++e)
		embed_data[e] = rf->getfloat();
	std::string text_s = rf->getstring();
	if (!text_s.empty()) {
		text = cpp_strdup(text_s);
	}
}

void LMEmbedding::out_to_stream_fmt(std::ostream& o) {
	o << n_embed << std::endl;
	for (int e = 0; e < n_embed; ++e) {
		o << embed_data[e] << std::endl;
	}
	std::string tout = text;
	p_replace(tout, "\n", " ");
	o << tout << std::endl;
}

std::string LMEmbedding::original_text() const {
	std::string ret;
	if (text != nullptr)
		ret = text;
	return ret;
}

LMBestMatch::LMBestMatch(int max_matches) {
	n_matches = 0;
	n_matches_max = std::min(max_matches, MAX_EMBED_MATCHES);
	n_matches_max = std::max(n_matches_max, 1);
	for (int i = 0; i < MAX_EMBED_MATCHES; ++i) {
		matches[i] = nullptr;
		cos_sim[i] = -2.0;
		filename[i] = nullptr;
		offset[i] = 0UL;
	}
	min_cos_sim = 2.0;
	i_min = 0;
	i_own_this_memory = false;
}

LMBestMatch::~LMBestMatch() {
	if (i_own_this_memory) {
		for (int e = 0; e < MAX_EMBED_MATCHES; ++e) {
			if (matches[e] != nullptr) {
				matches[e]->free();
				delete matches[e];
				matches[e] = nullptr;
			}
			if (filename[e] != nullptr) {
				delete filename[e];
				filename[e] = nullptr;
			}
		}
	}
}

bool LMBestMatch::check_match(LMEmbedding* lme, double score, const char* fname, u32 offs) {
	if (n_matches == n_matches_max && score <= min_cos_sim) {
		return false;
	}

	if (n_matches < n_matches_max) {
		matches[n_matches] = lme;
		cos_sim[n_matches] = score;
		filename[n_matches] = fname;
		offset[n_matches] = offs;
		if (score < min_cos_sim) {
			min_cos_sim = score;
			i_min = n_matches;
		}
		n_matches++;
		return true;
	}

	// the match array is full, but this has a better cosine similarity than the worst, so replace that.
	if (i_own_this_memory) {
		matches[i_min]->free();
		delete matches[i_min];
		delete filename[i_min];
	}
	matches[i_min] = lme;
	cos_sim[i_min] = score;
	filename[i_min] = fname;
	offset[i_min] = offs;
	i_min = 0;
	min_cos_sim = cos_sim[0];
	for (int e = 1; e < n_matches; ++e) {
		if (cos_sim[e] < min_cos_sim) {
			min_cos_sim = cos_sim[e];
			i_min = e;
		}
	}
	return true;
}

void LMBestMatch::sort_matches() {
	for (int i = 0; i < MAX_EMBED_MATCHES; ++i) {
		for (int j = i + 1; j < MAX_EMBED_MATCHES; ++j) {
			if (matches[j] == nullptr) {
				continue;
			}
			if (cos_sim[i] < cos_sim[j]) {
				SWAP(matches[i], matches[j], LMEmbedding*);
				SWAP(cos_sim[i], cos_sim[j], double);
				SWAP(filename[i], filename[j], const char*);
				SWAP(offset[i], offset[j], u32);
				--j;
			}
		}
	}
}

LMEmbeddingFile::LMEmbeddingFile() {
}

LMEmbeddingFile::~LMEmbeddingFile() {
	free();
}

void LMEmbeddingFile::free() {
	pathname.clear();
	offsets.clear();
	for (auto le : embeds)
		delete le;
	embeds.clear();
}

void LMEmbeddingFile::out_to_ramfile(RamFile* rf) {
	rf->putstring(pathname);
	ship_assert(embeds.size() == offsets.size());
	rf->put32((i32) embeds.size());
	for (int e = 0; e < embeds.size(); ++e) {
		embeds[e]->out_to_ramfile(rf);
		rf->put32(offsets[e]);
	}
}

void LMEmbeddingFile::in_from_ramfile(RamFile* rf) {
	i32 sz;
	free();
	pathname = rf->getstring();
	sz = rf->get32();
	for (i32 e = 0; e < sz; ++e) {
		LMEmbedding* le = new LMEmbedding;
		le->in_from_ramfile(rf);
		embeds.push_back(le);
		offsets.push_back(rf->get32());
	}
}

void LMEmbeddingFile::out_to_stream_fmt(std::ostream& o) {
	for (int e = 0; e < embeds.size(); ++e) {
		o << pathname << std::endl;
		o << offsets[e] << std::endl;
		embeds[e]->out_to_stream_fmt(o);
	}
}

int LMEmbeddingFile::best_match(const LMEmbedding* le, double* score) {
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

void LMEmbeddingFile::best_matches(LMBestMatch& best_matches, const LMEmbedding* le) {
	for (int e = 0; e < embeds.size(); ++e) {
		double sc = embeds[e]->cosine_similarity(le);
		best_matches.check_match(embeds[e], sc, pathname.c_str(), offsets[e]);
	}
}

int LMEmbeddingFile::count_embeddings() const {
	return embeds.size();
}

int LMEmbeddingFile::count_text_bytes() const {
	int ret = 0;
	for (const auto e : embeds) {
		ret += 4;
		if (!is_null(e->text))
			ret += strlen(e->text);
	}
	return ret;
}

LMEmbeddingFolder::LMEmbeddingFolder() {
}

LMEmbeddingFolder::~LMEmbeddingFolder() {
	free();
}

void LMEmbeddingFolder::out_to_ramfile(RamFile* rf) {
	rf->put32((i32) known_files.size());
	for (const auto& str : known_files)
		rf->putstring(str);
	rf->put32((i32) files.size());
	for (auto f : files)
		f->out_to_ramfile(rf);
}

void LMEmbeddingFolder::in_from_ramfile(RamFile* rf) {
	free();
	i32 nkf = rf->get32();
	for (i32 e = 0; e < nkf; ++e) {
		std::string kf = rf->getstring();
		known_files.insert(kf);
	}
	i32 nf = rf->get32();
	for (i32 e = 0; e < nf; ++e) {
		LMEmbeddingFile* lef = new LMEmbeddingFile;
		lef->in_from_ramfile(rf);
		files.push_back(lef);
	}
}

void LMEmbeddingFolder::in_from_file(const char* path) {
	RamFile rf(path, RAMFILE_READONLY);
	in_from_ramfile(&rf);
	rf.close();
}

void LMEmbeddingFolder::out_to_file(const char* path) {
	RamFile rf(path, RAMFILE_DEFAULT_COMPRESS);
	rf.truncate();
	out_to_ramfile(&rf);
	rf.close();
}

void LMEmbeddingFolder::out_to_stream_fmt(const std::string& path) {
	std::ofstream o;
	
	o.open(path);
	for (LMEmbeddingFile* lef : files) {
		lef->out_to_stream_fmt(o);
	}
	o.close();
	o.clear();
}

void LMEmbeddingFolder::free() {
	for (auto f : files)
		delete f;
	files.clear();
}

int LMEmbeddingFolder::count_files() const {
	return files.size();
}

int LMEmbeddingFolder::count_embeddings() const {
	int ret = 0;
	for (const auto* f : files)
		ret += f->count_embeddings();
	return ret;
}

int LMEmbeddingFolder::count_text_bytes() const {
	int ret = 0;
	for (const auto* f : files)
		ret += f->count_text_bytes();
	return ret;
}

int LMEmbeddingFolder::best_match(int file_idx, const LMEmbedding* le, double* score) {
	if (file_idx < 0 || file_idx >= files.size())
		return -1;
	return files[file_idx]->best_match(le, score);
}

void LMEmbeddingFolder::best_matches(LMBestMatch& best_matches, const LMEmbedding* le) {
	for (auto file : files) {
		file->best_matches(best_matches, le);
	}
}

char* cpp_strdup(const std::string& str) {
	char* ret = nullptr;
	ret = new char [strlen(str.c_str()) + 1];
	NOT_NULL_OR_RETURN(ret, ret);
	strcpy(ret, str.c_str());
	return ret;
}

LMEmbeddingStream::LMEmbeddingStream(const std::string& filepath) {
	stream.open(filepath);
}

LMEmbeddingStream::~LMEmbeddingStream() {
	stream.close();
}

void LMEmbeddingStream::rewind() {
	stream.clear();
	stream.seekg(0);
}

bool LMEmbeddingStream::read_embedding(LMEmbedding& lme_out, std::string* path_out, u32* offs_out) {
	std::string line;
	float* em = nullptr;
	char* text = nullptr;
	int n_em;

	std::getline(stream, line);
	NOT_EOF_OR_RETURN(stream, false);

	if (path_out != nullptr) {
		*path_out = line;
	}

	std::getline(stream, line);
	NOT_EOF_OR_RETURN(stream, false);

	if (offs_out != nullptr) {
		*offs_out = (u32) atoi(line.c_str());
	}

	std::getline(stream, line);
	NOT_EOF_OR_RETURN(stream, false);
	n_em = atoi(line.c_str());
	if (n_em <= 0)
		return false;
	em = new float [n_em];
	NOT_NULL_OR_RETURN(em, false);

	for (int e = 0; e < n_em; ++e) {
		std::getline(stream, line);
		NOT_EOF_OR_RETURN(stream, false);
		em[e] = atof(line.c_str());
	}

	std::getline(stream, line);
	NOT_EOF_OR_RETURN(stream, false);
	text = cpp_strdup(line);
	NOT_NULL_OR_RETURN(text, false);

	lme_out.n_embed = n_em;
	lme_out.embed_data = em;
	lme_out.text = text;

	return true;
}

void LMEmbeddingStream::best_matches(LMBestMatch& best_matches, const LMEmbedding* le) {
	LMEmbedding* lme = new LMEmbedding;
	bool ok = false;

	// let the LMBestMatch object know it can free matches that are bumped.
	best_matches.i_own_this_memory = true;

	rewind();
	forever {
		std::string path;
		u32 offs;

		ok = read_embedding(*lme, &path, &offs);
		if (!ok)
			break;

		if (best_matches.check_match(lme, lme->cosine_similarity(le), cpp_strdup(path), offs)) {
			// we have saved that match, so create a new LMEmbedding object.
			lme = new LMEmbedding;
			NOT_NULL_OR_RETURN_VOID(lme);
		}
	}

	delete lme;
}

/*** end lmembed.cpp ***/
