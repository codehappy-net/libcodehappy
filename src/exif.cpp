/***

	exif.cpp

	Read EXIF data from an image file.

	2024, C. M. Street

***/

/* Search haystack for needle, ignoring NUL terminators in haystack, up to haystack_end. Good for searching in binary files, etc. */
char* zstrsearch(const char* haystack, const char* needle, const char* haystack_end) {
	ship_assert(haystack != nullptr && needle != nullptr && haystack_end > haystack);
	size_t needle_len = strlen(needle);	// we respect NUL terminators in the needle in this version, to save a needle_end/size parameter.
	haystack_end -= needle_len;
	haystack_end++;
	
	while (haystack < haystack_end) {
		if (!memcmp(haystack, needle, needle_len))
			return (char *) haystack;
		++haystack;
	}
	return nullptr;
}

/* same as above, except this ignores NUL terminators in the needle as well. */
char* zstrsearch(const char* haystack, const char* needle, const char* haystack_end, const char* needle_end) {
	ship_assert(haystack != nullptr && needle != nullptr && haystack_end > haystack && needle_end > needle);
	size_t needle_len = needle_end - needle;
	haystack_end -= needle_len;
	haystack_end++;
	
	while (haystack < haystack_end) {
		if (!memcmp(haystack, needle, needle_len))
			return (char *) haystack;
		++haystack;
	}
	return nullptr;
}

ExifDictionary::ExifDictionary() {
}

ExifDictionary::~ExifDictionary() {
}

ExifDictionary::ExifDictionary(const std::string& pathname) {
	read_exif_from_image_file(pathname);
}

bool ExifDictionary::exif_key_present(const std::string& key) const {
	return dict.find(key) != dict.end();
}

std::string ExifDictionary::exif_key_value(const std::string& key) const {
	std::string empty;
	if (!exif_key_present(key))
		return empty;
	return (dict.find(key))->first;
}

void ExifDictionary::iterate_dict_start() {
	iter = 0;
	dict_i = dict.begin();
}

bool ExifDictionary::iterate_dict_next(std::string& key_out, std::string& val_out) {
	if (iter < 0 || iter >= dict.size())
		return false;

	key_out = dict_i->first;
	val_out = dict_i->second;

	++dict_i;
	++iter;

	return true;
}

bool ExifDictionary::read_exif_from_image_file(const std::string& pathname) {
	// TODO: this currently only works on .pngs, we should support at least .jpgs as well.
	RamFile * rf;
	char * w, * w2, * we;
	dict.clear();

	rf = new RamFile(pathname, RAMFILE_READONLY);
	NOT_NULL_OR_RETURN(rf, false);
	w = (char *) rf->buffer();
	we = w + rf->length();

	w = zstrsearch(w, "IHDR", we);
	NOT_NULL_OR_GOTO(w, LRet);
	
	w2 = zstrsearch(w, "IDATx", we);
	if (w2 != nullptr) {
		// we can stop searching at the start of the data record.
		we = w2;
	}

	forever {
		u32 nb;
		unsigned char* uw;
		std::string key, val;
		w = zstrsearch(w, "tEXt", we);
		NOT_NULL_OR_BREAK(w);
		uw = (unsigned char *)w;
		nb = u32(*(uw - 1)) + (u32(*(uw - 2)) << 8) + (u32(*(uw - 3)) << 16) + (u32(*(uw - 4)) << 24);
		w += 4;
		ship_assert(w + nb + 4 < we);
		w2 = w + nb;
		// w now points to the beginning of the key name, which is zero-terminated; w2 now
		// points to the end of the text data (there appears to be a 32-bit chksum here?)
		*w2 = '\000';
		key = w;
		w += strlen(w);
		++w;
		val = w;
		// insert the new entry and continue.
		dict[key] = val;
		w = w2 + 1;
	}

LRet:
	delete rf;
	return dict.size() > 0;
}


/*** end exif.cpp ***/
