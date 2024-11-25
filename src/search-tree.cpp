/***

	search-tree.cpp

	Fast search and replace, over a set of target words/phrases. Tell the tree the set of strings that
	are of interest, and you can use the SearchReplaceTree to look for (and replace) instances of those
	strings.

	2024, C. M. Street

***/

int srtnode_idx_from_char(int c) {
	int i = tolower(c);
	if (i >= 'a' && i <= 'z')
		return (i - 'a');
	if (isspace(i))
		return SRTNODE_SPACE;
	return SRTNODE_NONLETTER;
}

SRTNode::SRTNode() {
	v = false;
	for (int i = 0; i < SRTNODE_COUNT; ++i)
		children[i] = nullptr;
	replace = nullptr;
}

SRTNode::~SRTNode() {
	for (int i = 0; i < SRTNODE_COUNT; ++i)
		if (children[i]) {
			delete children[i];
			children[i] = nullptr;
		}
	if (replace != nullptr) {
		delete replace;
		replace = nullptr;
	}
}

SearchReplaceTree::SearchReplaceTree() {
	tree = new SRTNode;
}

SearchReplaceTree::~SearchReplaceTree() {
	delete tree;
	tree = nullptr;
}

void SearchReplaceTree::add_searchable(const std::string& term, const std::string& replace) {
	if (replace.empty())
		add_searchable(term.c_str());
	else
		add_searchable(term.c_str(), replace.c_str());
}

void SearchReplaceTree::add_searchable(const char * term, const char * replace) {
	// Make sure that replacement text doesn't contain the original (or other) tags, this can lead to an infinite loop.
	SRTNode * node = this->tree;
	const char *w = term;
	while (*w) {
		int idx = srtnode_idx_from_char(*w);
		if (node->children[idx] == nullptr) {
			node->children[idx] = new SRTNode;
			ship_assert(node->children[idx] != nullptr);
		}
		node = node->children[idx];
		++w;
	}

	node->v = true;
	if (replace != nullptr) {
		node->replace = new char [ strlen(replace) + 1 ];
		strcpy(node->replace, replace);
	}
}

void SearchReplaceTree::search_string(const std::string& search_str) {
	sstr = search_str;
	sptr = sstr.c_str();
}

void SearchReplaceTree::search_string(const char * search_str) {
	sstr = search_str;
	sptr = sstr.c_str();
}

void SearchReplaceTree::reset_search() {
	sptr = sstr.c_str();
}

SRTNode * SearchReplaceTree::search_at() {
	SRTNode * ret = tree;
	const char * w = sptr;
	match_len = 1;
	while (*w) {
		int idx = srtnode_idx_from_char(*w);
		if (ret->children[idx] == nullptr) {
			return nullptr;
		}
		ret = ret->children[idx];
		if (ret->v) {
			return ret;
		}
		++w;
		++match_len;
	}
	return nullptr;
}

const char * SearchReplaceTree::next_searchable(SRTNode ** node_ret) {
	SRTNode * ret = nullptr;
	while (*sptr) {
		ret = search_at();
		if (ret != nullptr) {
			if (node_ret != nullptr) {
				(*node_ret) = ret;
			}
			++sptr;
			return sptr - 1;
		}
		++sptr;
	}
	if (node_ret != nullptr) {
		*node_ret = nullptr;
	}
	return nullptr;
}

void SearchReplaceTree::do_all_replace(const std::string& replace) {
	if (replace.empty())
		do_all_replace(nullptr);
	else
		do_all_replace(replace.c_str());
}

void SearchReplaceTree::do_all_replace(const char * replace) {
	const char * w;
	SRTNode * node;
	forever {
		std::string rp;
		int idx;
		w = next_searchable(&node);
		NOT_NULL_OR_BREAK(w);
		idx = (w - sstr.c_str());
		if (replace == nullptr) {
			if (node->replace != nullptr)
				rp = node->replace;
		} else {
			rp = replace;
		}
		if (rp.empty()) {
			// no replacement if replace is nullptr & node has no suggested replacement.
			// if you want deletion use delete_all_searchable().
			continue;
		}
		sstr.erase(idx, (size_t) match_len);
		sstr.insert(idx, rp);
		// string may have reallocated, so reset the pointer here
		sptr = sstr.c_str() + idx;
	}
}

void SearchReplaceTree::delete_all_searchable() {
	const char * w;
	SRTNode * node;
	forever {
		size_t idx;
		w = next_searchable(&node);
		NOT_NULL_OR_BREAK(w);
		idx = (w - sstr.c_str());
		sstr.erase(idx, (size_t) match_len);
		sptr = sstr.c_str() + idx;
	}
}

/*** end search-tree.cpp ***/
