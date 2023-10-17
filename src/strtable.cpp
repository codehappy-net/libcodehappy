/***

	strtable.cpp

	If you have to work with data with repeated strings, save memory
	by using a StringTable to map indices to common strings.

	Copyright (c) 2022 Chris Street.

***/

u32 StringTable::index(const std::string& str) {
	u32 offs;
	const char* buf = str.c_str();
	if (present(str))
		return str_to_idx[str];
	offs = sp.length();
	sp.memcat((const u8 *)buf, strlen(buf) + 1);
	if (!case_sensitive) {
		__strlwr((char *)sp.buffer() + offs);
	}
	idx_to_offset[nidx] = offs;
	str_to_idx[str] = nidx;
	return nidx++;
}

u32 StringTable::index(const char* str) {
	std::string s = str;
	return index(s);
}

const char* StringTable::string(u32 idx) {
	u32 offs = idx_to_offset[idx];
	return (const char*)sp.buffer() + offs;
}

void StringTable::string(std::string& str_out, u32 idx) {
	u32 offs = idx_to_offset[idx];
	str_out = ((char*)sp.buffer() + offs);
}

bool StringTable::present(const char* str) const {
	std::string s = str;
	return present(s);
}

bool StringTable::present(const std::string& str) const {
	if (!case_sensitive) {
		std::string s = str;
		for (auto& c : s)
			c = tolower(c);
		return (str_to_idx.find(s) != str_to_idx.end());
	}
	return (str_to_idx.find(str) != str_to_idx.end());
}

void StringTable::persist(RamFile* rf) {
	/* We write case_sensitive, nidx, sp, idx_to_offset; str_to_idx is rebuilt on load. */
	rf->putc(case_sensitive ? 1 : 0);
	rf->putu32(nidx);
	rf->putsp(sp);
	rf->putu32(idx_to_offset.size());
	for (const auto& e : idx_to_offset) {
		rf->putu32(e.first);
		rf->putu32(e.second);
	}
}

bool StringTable::load(RamFile* rf) {
	int i;
	u32 u;

	i = rf->getc();
	if (i < 0 || i > 1)
		return false;
	case_sensitive = (i != 0);
	nidx = rf->getu32();
	sp.free();
	if (rf->getsp(sp) != 0)
		return false;
	u = rf->getu32();
	idx_to_offset.clear();
	for (u32 e = 0; e < u; ++e) {
		u32 first, second;
		first = rf->getu32();
		second = rf->getu32();
		idx_to_offset[first] = second;
	}

	/* Rebuild str_to_idx. */
	char *w = ((char*)sp.buffer());
	str_to_idx.clear();
	for (u32 e = 0; e < u; ++e) {
		std::string str = (w + idx_to_offset[e]);
		str_to_idx[str] = e;
	}

	return true;
}

void StringTable::clear() {
	nidx = 0;
	idx_to_offset.clear();
	str_to_idx.clear();
	sp.clear();
}

/* end strtable.cpp */
