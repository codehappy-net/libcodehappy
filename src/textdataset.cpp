/***

	textdataset.cpp

	Loads text files (individually, or by the directoryful, etc.) Can output
	them into .json files for model training, etc.

	TODO: Get it to use the LLaMA tokenizer, so I have accurate token counts.

	May 2023, C. M. Street

***/

Paragraph::Paragraph() {
	length = 0;
}

void Paragraph::add_line(std::string& line, const std::string& src_label) {
	SplitString ss;
	ss.split_on_whitespace(line);
	content += ss;
	src = src_label;
	length += ss.nstrs();
}

void Paragraph::output_all_splits(std::ostream &o) const {
	if (length < Paragraph::max_two_splits) {
		output_range_splits(0, length / 2, length - 1, o);
	}
	if (length >= Paragraph::min_three_splits && length <= Paragraph::max_three_splits) {
		output_range_splits(0, length / 3, (2 * length) / 3, o);
		output_range_splits(length / 3, (2 * length) / 3, length - 1, o);
	}
	if (length >= Paragraph::min_four_splits && length <= Paragraph::max_four_splits) {
		output_range_splits(0, length / 4, length / 2, o);
		output_range_splits(length / 4, length / 2, (3 * length) / 4, o);
		output_range_splits(length / 2, (3 * length) / 4, length - 1, o);
	}
	if (length > Paragraph::max_four_splits) {
		int e = 0;
		forever {
			if (e + 200 >= length) {
				int v = (length - e) / 2;
				if (v >= 10) {
					output_range_splits(e, e + v, length - 1, o);
				}
				break;
			}
			output_range_splits(e, e + 100, e + 200, o);
			e += 200;
		}
	}
}

#define GPT4ALL

void Paragraph::output_range_splits(int i1, int i2, int i3, std::ostream& o) const {
	// we were using the gpt4all .jsonl format, which looks something like:
	// {"prompt": "this is the prompt", "response": "this is the response", "source": "this is the source"}
	// or are we using the Alpaca format, which is "instruction", "input", "output".
	const std::vector<std::string>& strs = content.strs_const();
	i1 = std::min(std::max(i1, 0), (int) (strs.size()));
	i2 = std::min(std::max(i2, 0), (int) (strs.size()));
	i3 = std::min(std::max(i3, 0), (int) (strs.size()));
#ifdef GPT4ALL
	o << "{\"prompt\": \"";
#else	
	o << "{\"instruction\": \"Complete the provided text.\", \"input\": \"";
#endif	
	for (int i = i1; i < i2; ++i) {
		output_json_escaped_text(strs[i], o);
		o << " ";
	}
#ifdef GPT4ALL
	o << "\", \"response\": \"";
#else	
	o << "\", \"output\": \"";
#endif
	for (int i = i2; i <= i3; ++i) {
		output_json_escaped_text(strs[i], o);
		if (i == i3) {
			if (i3 + 1 >= strs.size()) {
				o << "\\n";
			}
		} else {
			o << " ";
		}
	}
#ifdef GPT4ALL
	o << "\", \"source\": \"";
	output_json_escaped_text(src, o);
#endif	
	o << "\"}\n";
}

#undef GPT4ALL

void Paragraph::clear() {
	content.clear();
	src.clear();
	length = 0;
}

TextDataset::TextDataset() {
	total_length = 0;
}

void TextDataset::add_from_file(const char* filename) {
	Paragraph para_in_progress;
	std::ifstream i;
	std::string line;
	
	i.open(filename);
	forever {
		std::getline(i, line);
		if (i.eof())
			break;
		if (line.empty())
			continue;
		if (isspace(line[0]) && para_in_progress.length >= TextDataset::min_len_paragraph) {
			// leading whitespace/indentation is counted as indicative of a new paragraph.
			paras.push_back(para_in_progress);
			total_length += para_in_progress.length;
			para_in_progress.clear();
		}
		para_in_progress.add_line(line, filename);
	}
	i.close();
	i.clear();
	
	if (para_in_progress.length >= TextDataset::min_len_paragraph) {
		paras.push_back(para_in_progress);
		total_length += para_in_progress.length;
	}
}

void TextDataset::add_from_folder(const char* path) {
	DIR* di = opendir(path);
	dirent* entry;

	while (entry = readdir(di)) {
		if (entry->d_name[0] == '.')
			continue;
		if (strstr(entry->d_name, ".txt") != nullptr) {
			std::string pathname;
			make_pathname(path, entry->d_name, pathname);
			add_from_file(pathname.c_str());
		}
	}
	closedir(di);
}

void TextDataset::output_training_json(const char* pathname_out) const {
	std::ofstream o;
	o.open(pathname_out);
	for (const auto& para : paras) {
		para.output_all_splits(o);
	}
	o.close();
	o.clear();
}

void TextDataset::show_stats() const {
	std::cout << "Contents of dataset: " << paras.size() << " paragraphs, " << total_length << " word-equivalents.\n";
}

void output_json_escaped_text(const std::string& str, std::ostream& o) {
	for (unsigned char c : str) {
		u32 i = (u32)c;
		switch (i) {
		default:
			if (i < 128)
				o << char(c);
			break;
		case 150:
		case 151:
			o << "-";
			break;
		case 146:
		case 147:
			o << "'";
			break;
		case '\t':
			o << "\\t";
			break;
		case '\b':
			o << "\\b";
			break;
		case '\f':
			o << "\\f";
			break;
		case '\n':
			o << "\\n";
			break;
		case '\r':
			o << "\\r";
			break;
		case '\\':
			o << "\\\\";
			break;
		case '\"':
		case 145:
		case 148:
			o << "\\\"";
			break;
		}
	}
}

