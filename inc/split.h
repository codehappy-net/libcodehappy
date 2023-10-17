/***

	split.h
	
	Split strings.

	Copyright (c) 2022 Chris Street.

***/
#ifndef _SPLIT_H_
#define _SPLIT_H_

class SplitString {
public:
	SplitString() { sc = ','; }
	SplitString(char split_char) : sc(split_char) {}
	SplitString(const char* str, char split_char = ',');
	SplitString(const std::string& str, char split_char = ',');
	SplitString(Scratchpad* sp, char split_char = ',');
	~SplitString();

	std::vector<std::string>& strs() { return splits; }
	const std::vector<std::string>& strs_const() const { return splits; }
	int nstrs() const { return splits.size(); }
	bool empty() const;
	void clear();

	void split_on_whitespace(const char* str);
	void split_on_whitespace(const std::string& str);

	void add_splits(const char* str);
	void add_splits(const std::string& str);

	SplitString& operator=(const char* str);
	SplitString& operator=(const std::string& str);
	
	/* NB: the += operator adds the string to splits as-is, it doesn't split on character or whitespace. Use add_splits() for
		that functionality.*/
	SplitString& operator+=(const char* str);
	SplitString& operator+=(const std::string& str);
	SplitString& operator+=(const SplitString& ss);
private:
	void update_split(const std::string& str, bool clear_splits = true);
	char sc;
	std::vector<std::string> splits;
};

#endif  // _SPLIT_H_
/* end split.h */
