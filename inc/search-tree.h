/***

	search-tree.h

	Fast search and replace, over a set of target words/phrases. Tell the tree the set of strings that
	are of interest, and you can use the SearchReplaceTree to look for (and replace) instances of those
	strings.

	2024, C. M. Street

***/

const int SRTNODE_SPACE = 26, SRTNODE_NONLETTER = 27, SRTNODE_COUNT = 28;

static std::string srt_empty_str;

int srtnode_idx_from_char(int c);

struct SRTNode {
	SRTNode();
	~SRTNode();
	bool v;
	SRTNode* children[SRTNODE_COUNT];
	char * replace;
};

/* Note that this implementation of SearchReplaceTree is case-insensitive, and treats all whitespace as the
   same character. */
class SearchReplaceTree {
public:
	SearchReplaceTree();
	~SearchReplaceTree();

	/* Add a searchable term (with or without a suggested replacement) */
	void add_searchable(const std::string& term, const std::string& replace = srt_empty_str);
	void add_searchable(const char * term, const char * replace = nullptr);

	/* Provide the string that we are interested in searching/replacing in. */
	void search_string(const std::string& search_str);
	void search_string(const char * search_str);

	/* Returns a pointer to the next occurrence of any of the searchable terms. (nullptr when no more) */
	const char * next_searchable(SRTNode ** node_ret = nullptr);

	/* Replaces all occurrences of search terms with their specified replacement (or the replacement given, if non-empty/null) */
	void do_all_replace(const std::string& replace = srt_empty_str);
	void do_all_replace(const char * replace);

	/* Deletes all occurrences of search terms */
	void delete_all_searchable();

	/* Resets the search pointer to the beginning of the string. */
	void reset_search();

	/* Returns the string, with any replacements or deletions. */
	std::string& str() { return sstr; }

private:
	SRTNode * tree;
	// current search string
	std::string sstr;
	// search pointer
	const char * sptr;
	// returned length of matched phrase
	int match_len;
	// attempt a match at the current search position
	SRTNode * search_at();
};

/*** end search-tree.h ***/
