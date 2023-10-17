/***

	split.cpp
	
	Split strings.

	Copyright (c) 2022 Chris Street.

***/

SplitString::SplitString(const char* str, char split_char) {
	std::string s;
	sc = split_char;
	s = str;
	update_split(s);
}

SplitString::SplitString(const std::string& str, char split_char) {
	sc = split_char;
	update_split(str);
}

SplitString::SplitString(Scratchpad* sp, char split_char) {
	std::string s;
	sc = split_char;
	s = (char *)sp->buffer();
	update_split(s);
}

SplitString::~SplitString() {
}

SplitString& SplitString::operator=(const char* str) {
	std::string s;
	s = str;
	update_split(s);
	return *this;
}

SplitString& SplitString::operator=(const std::string& str) {
	update_split(str);
	return *this;
}

SplitString& SplitString::operator+=(const char* str) {
	std::string s;
	s = str;
	splits.push_back(s);
	return *this;
}

SplitString& SplitString::operator+=(const std::string& str) {
	splits.push_back(str);
	return *this;
}

SplitString& SplitString::operator+=(const SplitString& ss) {
	for (const auto& s : ss.splits) {
		splits.push_back(s);
	}
	return *this;
}

void SplitString::add_splits(const char* str) {
	std::string s = str;
	update_split(s, false);
}

void SplitString::add_splits(const std::string& str) {
	update_split(str, false);
}

void SplitString::update_split(const std::string& str, bool clear_splits) {
	// TODO: respect quoted separators?
	std::string s_c = str;
	char* w = const_cast<char*>(s_c.c_str()), * w2;
	if (clear_splits)
		splits.clear();
	forever {
		w2 = strchr(w, sc);
		if (is_null(w2)) {
			if (*w)
				splits.push_back(w);
			break;
		}
		*w2 = '\000';
		if (*w)
			splits.push_back(w);
		w = w2 + 1;
	}
}

bool SplitString::empty() const {
	if (0 == splits.size())
		return true;
	return splits[0].length() > 0;
}

void SplitString::clear() {
	splits.clear();
}

void SplitString::split_on_whitespace(const char* str) {
	std::string word;
	const char* w, * w2;
	w = str;
	forever {
		if ((*w) == '\000')
			break;
		if (isspace(*w)) {
			if (!word.empty()) {
				splits.push_back(word);
				word.clear();
			}
		} else {
			word += (*w);
		}
		++w;
	}
	if (!word.empty())
		splits.push_back(word);
}

void SplitString::split_on_whitespace(const std::string& str) {
	split_on_whitespace(str.c_str());
}

