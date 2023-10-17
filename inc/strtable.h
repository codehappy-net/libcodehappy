/***

	strtable.h

	If you have to work with data with repeated strings, save memory
	by using a StringTable to map indices to common strings.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __STRTABLE_H__
#define __STRTABLE_H__

#include <unordered_map>

const u32 STRINDEX_INVALID = 0xfffffffful;

class StringTable {
public:
	StringTable() { case_sensitive = true; nidx = 0; }
	StringTable(bool cs) { case_sensitive = cs; nidx = 0; }
	~StringTable() {}

	/* The index of a specified string. This adds the string to the table if not already present. */
	u32 index(const std::string& str);
	u32 index(const char* str);

	/* The number of strings in the table. */
	u32 length(void) const { return idx_to_offset.size(); }

	/* Get the string for a specified index. */
	const char* string(u32 idx);
	void string(std::string& str_out, u32 idx);

	/* Is the string present in the table? */
	bool present(const char* str) const;
	bool present(const std::string& str) const;

	/* The total number of bytes of all strings in the string table. */
	u32 bytes(void) const { return sp.length(); }

	/* Is this StringTable case sensitive? */
	bool is_case_sensitive(void) const { return case_sensitive; }

	/* Persist the string table to a ramfile. */
	void persist(RamFile* rf);

	/* Load the string table from a ramfile. Returns true on success. */
	bool load(RamFile* rf);

	/* Count of strings in the string table. */
	u32 cstr(void) const { return nidx; }

	/* Clear the string table. */
	void clear();

private:
	bool case_sensitive;
	u32 nidx;
	Scratchpad sp;
	std::unordered_map<u32, u32> idx_to_offset;
	std::unordered_map<std::string, u32> str_to_idx;
};

#endif  // __STRTABLE_H__
/* end strtable.h */