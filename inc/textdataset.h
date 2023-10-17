/***

	textdataset.h

	Loads text files (individually, or by the directoryful, etc.) Can output
	them into .json files for model training, etc.
	
	May 2023, C. M. Street

***/
#ifndef _TEXTDATASET_H
#define _TEXTDATASET_H

/* A series of text lines makes a Paragraph. */
class Paragraph {
public:
	Paragraph();
	void add_line(std::string& line, const std::string& src_label);

	// output various kinds of splits to json
	void output_all_splits(std::ostream &o) const;
	void output_range_splits(int i1, int i2, int i3, std::ostream& o) const;

	void clear();

	SplitString content;
	std::string src;
	int length;
	const int max_two_splits = 200;
	const int min_three_splits = 60;
	const int min_four_splits = 80;
	const int max_three_splits = 300;
	const int max_four_splits = 400;
};

/* A collection of Paragraphs. */
class TextDataset {
public:
	TextDataset();
	void add_from_file(const char* filename);
	void add_from_folder(const char* path);
	void output_training_json(const char* pathname_out) const;
	void show_stats() const;

private:
	std::vector<Paragraph> paras;
	int total_length;
	const int min_len_paragraph = 20;
};

/* helper: output json-escaped text to the passed ostream. */
extern void output_json_escaped_text(const std::string& str, std::ostream& o);

#endif  // _TEXTDATASET_H
/* end textdataset.h */
