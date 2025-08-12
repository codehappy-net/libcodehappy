/***

	exif.h

	Read EXIF data from an image file.

	2024, C. M. Street

***/
#ifndef __EXIF_H
#define __EXIF_H


class ExifDictionary {
public:
	ExifDictionary();
	/* this c'tor initializes the dictionary from an image file */
	ExifDictionary(const std::string& pathname);
	~ExifDictionary();

	/* returns true iff at least one EXIF key/value pair was read. */
	bool read_exif_from_image_file(const std::string& pathname);

	/* number of EXIF key/value pairs in the dictionary. */
	int exif_size() const	{ return dict.size(); }

	/* lookup specific EXIF data by key. */
	bool exif_key_present(const std::string& key) const; 
	std::string exif_key_value(const std::string& key) const; 

	/* iterate through all EXIF data keys, one at a time. iterate_dict_next() returns false iff
		there are no more keys. iterate_dict_start() will reset the iteration each time it is called. */
	void iterate_dict_start();
	bool iterate_dict_next(std::string& key_out, std::string& val_out);

private:
	std::unordered_map<std::string, std::string> dict;
	std::unordered_map<std::string, std::string>::iterator dict_i;
	int iter;
};

/*** helper functions that are generally useful ***/
/* Search haystack for needle, ignoring NUL terminators in haystack, up to haystack_end. Good for searching in binary files, etc. 
	returns ptr to the first appearance of 'needle' in 'haystack', or nullptr if none. */
extern char* zstrsearch(const char* haystack, const char* needle, const char* haystack_end);
/* same as above, except this ignores NUL terminators in the needle as well. 'needle_end' should point to the byte just after
	the last byte in the needle that you care about (just like haystack_end,) i.e. (needle_end - needle) gives needle length in bytes. */
extern char* zstrsearch(const char* haystack, const char* needle, const char* haystack_end, const char* needle_end);
/* as zstrsearch(), but return the first of two needles to appear. */
extern char* zstrsearch2(const char* haystack, const char* needle1, const char * needle2, const char* haystack_end);

#endif  // __EXIF_H
/* end exif.h */

