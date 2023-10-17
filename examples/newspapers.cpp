/***

	Download and OCR old newspapers from the Library of Congress.

	Copyright (c) 2023, Chris Street.

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

void dl_newspaper_day_page(int volume_code, const char* date, int page, const std::string& out_dir) {
	// Whew, we made it!
	RamFile* paper_page;
	std::string path;
	char URI[1024], fname[256];

	sprintf(fname, "%s-p%d.pdf", date, page);
	make_pathname(out_dir, fname, path);
	if (FileExists(path))
		return;

	sprintf(URI, "https://chroniclingamerica.loc.gov/lccn/sn%08d/%s/ed-1/seq-%d.pdf", volume_code, date, page);
	paper_page = FetchURI(URI);
	NOT_NULL_OR_RETURN_VOID(paper_page);

	paper_page->write_to_file(path);

	delete paper_page;
}

void dl_newspaper_day(int volume_code, const char* date, const std::string& out_dir) {
	RamFile* daily_paper;
	char URI[1024];
	int max_page = 0;

	sprintf(URI, "https://chroniclingamerica.loc.gov/lccn/sn%08d/%s/ed-1/", volume_code, date);
	daily_paper = FetchURI(URI);
	NOT_NULL_OR_RETURN_VOID(daily_paper);

	char* w = (char *)daily_paper->buffer();
	forever {
		w = strstr(w, "<div class=\"highlite\">");
		NOT_NULL_OR_BREAK(w);
		++max_page;
		++w;
	}

	for (int e = 0; e < max_page; ++e) {
		std::cout << "\tPage " << (e + 1) << "/" << max_page << "...\n";
		dl_newspaper_day_page(volume_code, date, e + 1, out_dir);
	}

	delete daily_paper;
}

void dl_newspapers_year(int volume_code, int y, const std::string& out_dir) {
	RamFile* calendar;
	char URI[1024];

	sprintf(URI, "https://chroniclingamerica.loc.gov/lccn/sn%08d/issues/%d/", volume_code, y);
	calendar = FetchURI(URI);
	NOT_NULL_OR_RETURN_VOID(calendar);

	char* w = (char *)calendar->buffer();
	forever {
		w = strstr(w, "/ed-1/");
		NOT_NULL_OR_BREAK(w);
		--w;
		while (*w != '/')
			--w;
		++w;
		assert(atoi(w) == y);
		*(w + 10) = '\000';
		std::cout << w << "...\n";
		dl_newspaper_day(volume_code, w, out_dir);
		w += 18;
	}

	delete calendar;
}

void dl_newspapers(int volume_code, int y1, int y2, const std::string& out_dir) {
	for (int y = y1; y <= y2; ++y) {
		dl_newspapers_year(volume_code, y, out_dir);
	}
}

int date_from_filename(char* fname) {
	int y, m, d, ret = 0;
	char* w;
	y = atoi(fname);
	w = strchr(fname, '-');
	NOT_NULL_OR_RETURN(w, ret);
	++w;
	m = atoi(w);
	w = strchr(w, '-');
	NOT_NULL_OR_RETURN(w, ret);
	++w;
	d = atoi(w);
	ret = (y * 10000) + (m * 100) + d;

	return ret;
}

void compact_for_search(char* w) {
	char* wp = w;
	int c;
	forever {
		if (!(*w)) {
			*wp = 0;
			break;
		}
		c = tolower(*w);
		if (c < 'a' || c > 'z') {
			++w;
			continue;
		}
		*wp = c;
		++wp;
		++w;
	}
}

bool char_predicate(char* ptr) {
	int c = tolower(*ptr);
	return c >= 'a' && c <= 'z';
}

void output_from(std::ostream& o, char* p1, char* p2) {
	while (p1 < p2) {
		o << char(*p1);
		++p1;
	}
}

void output_whitespace_from(std::ostream& o, char* p1, char* p2) {
	while (p1 < p2) {
		if (isspace(*p1))
			o << char(*p1);
		++p1;
	}
}

void process_eol_hyphens(char *w) {
	char* wp = w;
	forever {
		if (*w == 0) {
			*wp = 0;
			break;
		}
		if (*w == '-') {
			char* w2 = w;
			bool nl = false;
			++w2;
			forever {
				if (!(*w2))
					break;
				if ('\n' == *w2)
					nl = true;
				if (isspace(*w2)) {
					++w2;
					continue;
				}
				break;
			}
			if (nl) {
				w = w2;
				continue;
			}
		}
		// characters that shouldn't appear in the output
		if (strchr("|~`^_{}\\<>‘’*@+=¢€«", *w) != nullptr) {
			++w;
			continue;
		}
		*wp = *w;
		++w;
		++wp;
	}
}

#define PAGE_SEPARATOR	"--------------------------------------------------------------------------------"

void ocr_newspaper_page(std::ostream& o, const std::string& dir, int date, int page, int ocr_thres) {
	std::string path;
	char fname[256];
	char cmd[512];
	int N;

	sprintf(fname, "%d-%02d-%02d-p%d.pdf", yyyymmdd_year(date), yyyymmdd_month(date), yyyymmdd_day(date), page);
	make_pathname(dir, fname, path);

	/* convert the PDF to a PNG image file and then do Tesseract OCR */
	sprintf(cmd, "pdftoppm %s page -png", path.c_str());
	system(cmd);
	system("tesseract page-1.png pageocr -l eng");
	remove("page-1.png");
	/* extract the OCR text layer from the PDF file itself */
	sprintf(cmd, "pdftotext %s pageocr2.txt", path.c_str());
	system(cmd);

	/* Now pageocr.txt contains the Tesseract OCR output, and pageocr2.txt contains the original text layer from the PDF. */
	/* We create an attested combination of each. */
	RamFile rf1("pageocr.txt", RAMFILE_READONLY), rf2("pageocr2.txt", RAMFILE_READONLY);
	char* readptr = (char *)rf1.buffer(), *ptr, *rewind_ptr = nullptr;

	/* Actually, we just output the Tesseract output; it appears to be strictly better than the OCR text in the PDFs. Can clean the text separately? */
#if 1
	process_eol_hyphens(readptr);
	o << readptr;
#else
	compact_for_search((char *) rf2.buffer());
	ptr = readptr;
	N = 0;
	zeromem(cmd, 512);

	std::cout << (char*) rf2.buffer() << std::endl << std::endl;

	forever {
		if (*ptr == 0) {
			output_from(o, readptr, ptr);
			break;
		}
		if (!char_predicate(ptr)) {
			++ptr;
			continue;
		}
		cmd[N++] = tolower(*ptr);
		if (2 == N) {
			rewind_ptr = ptr;
		}
		if (strstr((char *) rf2.buffer(), cmd) == nullptr) {
			if (N - 1 >= ocr_thres) {
				std::cout << "++++ " << cmd << std::endl;
				output_from(o, readptr, ptr);
				++ptr;
				readptr = ptr;
				N = 0;
				zeromem(cmd, 512);
				continue;
			}
			std::cout << "---- " << cmd << std::endl;
			if (nullptr == rewind_ptr) {
				output_whitespace_from(o, readptr, ptr);
				++ptr;
				readptr = ptr;
				N = 0;
				zeromem(cmd, 512);
			} else {
				output_whitespace_from(o, readptr, rewind_ptr);
				readptr = rewind_ptr;
				ptr = rewind_ptr;
				N = 0;
				zeromem(cmd, 512);
				rewind_ptr = nullptr;
			}
			continue;
		}
		if (N > 510) {
			std::cout << "++++ " << cmd << std::endl;
			output_from(o, readptr, ptr);
			readptr = ptr;
			zeromem(cmd, 512);
			N = 0;
			continue;
		}
		++ptr;
	}
#endif
	o << std::endl << std::endl << PAGE_SEPARATOR << std::endl << std::endl;
}

void ocr_newspaper_date(const std::string& dir, int date, int ocr_thres, int npages, const std::string& paper_name) {
	std::ofstream o;
	std::string path;
	char fname[256];

	sprintf(fname, "%d-%02d-%02d.txt", yyyymmdd_year(date), yyyymmdd_month(date), yyyymmdd_day(date));
	make_pathname(dir, fname, path);

	/* write the paper name and the long-form date at the top of the text document */
	o.open(path);
	o << paper_name << std::endl;
	output_and_free(o, fmt_date_american_long(date));
	o << std::endl << std::endl << PAGE_SEPARATOR << std::endl << std::endl;

	/* now write in consensus versions of each page. */
	for (int e = 1; e <= npages; ++e) {
		std::cout << "\tPage " << e << "...\n";
		ocr_newspaper_page(o, dir, date, e, ocr_thres);
	}

	o.close();
	o.clear();
}

void ocr_dir(const std::string& dir, int y1, int y2, int ocr_thres, const std::string& paper_name) {
	std::unordered_map<int, bool> date_has_data;
	std::unordered_map<int, bool> date_has_txt;
	std::unordered_map<int, int> pages_for_date;
	DIR* di = opendir(dir.c_str());
	dirent* entry;
	char* w;

	while (entry = readdir(di)) {
		int date = date_from_filename(entry->d_name);
		int page;
		if (0 == date)
			continue;
		if (date < y1 * 10000 || date > (y2 * 10000 + 9999))
			continue;
		if (strstr(entry->d_name, ".txt")) {
			date_has_txt[date] = true;
			continue;
		}
		NOT_NULL_OR_CONTINUE(strstr(entry->d_name, ".pdf"));
		w = strstr(entry->d_name, "-p");
		NOT_NULL_OR_CONTINUE(w);
		w += 2;
		page = atoi(w);
		if (page > pages_for_date[date])
			pages_for_date[date] = page;
		date_has_data[date] = true;
	}
	closedir(di);

	for (const auto p : date_has_data) {
		int date = p.first;
		bool val = p.second;
		if (!val)
			continue;
		if (date_has_txt[date])
			continue;
		if (0 == pages_for_date[date])
			continue;
		
		output_and_free(std::cout, fmt_date_american_long(date));
		std::cout << "...\n";
		ocr_newspaper_date(dir, date, ocr_thres, pages_for_date[date], paper_name);
	}

}

void generate_frequency_table() {
	DIR* di = opendir("output/");
	dirent* entry;
	std::unordered_map<std::string, int> table;
	char* w;
	const int MIN_APPEARANCES = 40;

	while (entry = readdir(di)) {
		char* w, * w2;
		char word[128];
		if (!strstr(entry->d_name, ".txt"))
			continue;
		std::string path = "output/";
		path += entry->d_name;
		std::cout << path << "...\n";
		RamFile rf(path, RAMFILE_READONLY);
		w = (char *) rf.buffer();
		w2 = word;
		if (is_null(w))
			continue;
		word[127] = '\000';
		forever {
			if (!(*w))
				break;
			if (!isalpha(*w)) {
				if (w2 > word + 1) {
					*w2 = '\000';
					std::string wd = word;
					table[wd]++;
					w2 = word;
				}
				++w;
				continue;
			}
			*w2 = tolower(*w);
			++w;
			++w2;
			if (w2 == word + 127)
				--w2;
		}
		rf.close();
	}
	closedir(di);
	
	std::ofstream o;
	o.open("freqtable.txt");
	for (const auto& ey : table) {
		if (ey.second < MIN_APPEARANCES)
			continue;
		o << ey.first << "\t" << ey.second << "\n";
	}
	o.close();
	o.clear();
	std::cout << "Frequency table written to freqtable.txt\n";
}

void process_newspaper_text(const std::string& dir) {
}


int app_main() {
	ArgParse ap;
	int sn_value = 0, y1 = 0, y2 = 0;
	std::string out_dir, paper_name;
	bool ocr = false, table = false, process = false;
	int ocr_thres = 5;

	ap.add_argument("sn", type_int, "the sn-prefixed code for the desired newspaper (DC Evening Star is 83045462, e.g.)", &sn_value);
	ap.add_argument("y1", type_int, "the start year to download", &y1);
	ap.add_argument("y2", type_int, "the last year to download", &y2);
	ap.add_argument("out", type_string, "output directory");
	ap.add_argument("ocr", type_none, "instead of downloading to the output directory, OCR and combine OCR output for the specified directory and year range", &ocr);
	ap.add_argument("ocr_thres", type_int, "the minimum sequence size of alphanumeric characters that have to match for combined OCR (default 5)", &ocr_thres);
	ap.add_argument("paper", type_string, "A name to use for the paper when outputting OCR consensus documents.");
	ap.add_argument("table", type_none, "Generate a table of word frequency", &table);
	ap.add_argument("process", type_none, "Process the OCR newspaper texts into blocks of English text using the frequency tables", &process);
	ap.ensure_args(argc, argv);

	if (ap.flag_present("out")) {
		out_dir = ap.value_str("out");
	}
	if (ap.flag_present("paper")) {
		paper_name = ap.value_str("paper");
	}

	if (table) {
		std::cout << "Generating word frequency table...\n";
		generate_frequency_table();
		return 0;
	}

	if (out_dir.empty()) {
		std::cout << "Error: User must specify the 'out_dir' parameter for both download and OCR operations." << std::endl;
		return 1;
	}

	if (process) {
		std::cout << "Processing the newspaper texts in folder " << out_dir << "...\n";
		process_newspaper_text(out_dir);
		return(0);
	}

	if (sn_value == 0 && !ocr) {
		std::cout << "Error: User must specify the Library of Congress volume code using the 'sn' parameter.\n";
		return 1;
	}

	SORT2(y1, y2, int);
	if (y2 == 0) {
		std::cout << "Error: at least a start year (with 'y1') is necessary.\n";
		return 1;
	}
	if (y1 == 0) {
		y1 = y2;
	}
	std::cout << "Working with volume code sn" << sn_value << " from years " << y1 << " to " << y2 << ".\n";

	if (ocr) {
		ocr_dir(out_dir, y1, y2, ocr_thres, paper_name);
		return 0;
	}

	dl_newspapers(sn_value, y1, y2, out_dir);
	
	return 0;
}
