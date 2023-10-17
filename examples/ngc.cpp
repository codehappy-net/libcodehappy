/***

	ngc.cpp
	
	Download and caption images from the NGC featured coin galleries.
	
	Chris Street, 2023

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

#define GALLERY_FILE		"/home/exx/Documents/ngc_galleries"
#define OUTPUT_FOLDER		"/data/train/coins/"
#define MACHINE_CAPTIONS	"/home/exx/ml/LAVIS/coin-blip2-captions.txt"

struct CoinData {
	std::string uri_obverse;
	std::string uri_reverse;
	std::string caption;
	std::string subtitle;
	std::string grade;
};

/* key entries on the obverse URI */
std::unordered_map<std::string, CoinData> URItable;

void build_uri_table() {
	RamFile gallery;
	char * w, * w2, * we;

	gallery.open(GALLERY_FILE, RAMFILE_READ);
	w = (char *)gallery.buffer();
	forever {
		CoinData cd;
		w = strstr(w, "ccg-animate-enter ng-scope card");
		if (is_null(w))
			break;

		we = strstr(w, "</card-template>");
		if (is_null(we)) {
LErr:			++w;
			continue;
		}
		*we = 0;

		// Obverse and reverse URIs
		w2 = strstr(w, "ObverseImageUrl");
		NOT_NULL_OR_GOTO(w2, LErr);
		w2 = strstr(w2, "ng-href=\"");
		w2 += 9;
		w = strchr(w2, '\"');
		*w = 0;
		cd.uri_obverse = w2;
		
		w2 = strstr(w + 1, "ReverseImageUrl");
		NOT_NULL_OR_GOTO(w2, LErr);
		w2 = strstr(w2, "ng-href=\"");
		w2 += 9;
		w = strchr(w2, '\"');
		*w = 0;
		cd.uri_reverse = w2;

		// Caption, subtitle, and grade.
		w = strstr(w + 1, "card-content gallery-info");
		NOT_NULL_OR_GOTO(w, LErr);
		w = strstr(w, "ng-binding");
		NOT_NULL_OR_GOTO(w, LErr);
		w = strchr(w, '>') + 1;
		NOT_NULL_OR_GOTO(w, LErr);
		while (isspace(*w))
			++w;
		w2 = strstr(w, "</div");
		NOT_NULL_OR_GOTO(w2, LErr);
		if (isspace(*(w2 - 1))) {
			--w2;
			while (w2 > w && isspace(*w2))
				--w2;
			++w2;
		}
		*w2 = 0;
		cd.caption = w;
		w = w2 + 1;

		w2 = strstr(w, "div ng-if=\"card.Subtitle");
		if (!is_null(w2)) {
			w2 = strchr(w2, '>') + 1;
			w = strstr(w2, "</div");
			if (isspace(*(w - 1))) {
				--w;
				while (w > w2 && isspace(*w))
					--w;
				++w;
			}
			*w = 0;
			cd.subtitle = w2;
			++w;
		}
		
		w2 = strstr(w, "<div ng-bind-html=\"card.Grade");
		if (!is_null(w2)) {
			w2 = strchr(w2, '>') + 1;
			w = strstr(w2, "</div");
			*w = 0;
			w = strstr(w2, "<i class=\"star-grade");
			if (!is_null(w)) {
				*w = '*';
				*(w + 1) = 0;
			}
			w = strstr(w2, "<i class=\"plus-grade");
			if (!is_null(w)) {
				*w = '+';
				*(w + 1) = 0;
			}
			cd.grade = w2;
		}

		URItable[cd.uri_obverse] = cd;
		w = we + 1;
	}
}

bool str_contains_nonspace(const std::string& s) {
	const char* w = s.c_str();
	if (!(*w))
		return false;
	while (*w) {
		if (!isspace(*w))
			return true;
		++w;
	}
	return false;
}

void download_image(const std::string& URI, const std::string& obvrev, const CoinData& cd, std::ostream& o) {
	RamFile* rf;
	std::string pathname;
	char* tmpname;

	rf = FetchURI(URI);
	if (is_null(rf))
		return;

	tmpname = temp_file_name(OUTPUT_FOLDER, ".jpg");
	rf->write_to_file(tmpname);
	o << tmpname << "," << obvrev << ", " << cd.caption;
	if (str_contains_nonspace(cd.subtitle))
		o << ", " << cd.subtitle;
	if (str_contains_nonspace(cd.grade))
		o << ", " << cd.grade;
	o << "\n";
	o.flush();
	delete tmpname;
	delete rf;
}

void download_table() {
	std::ofstream o;
	int i = 1;

	o.open("coin-captions.txt");
	for (const auto& ey : URItable) {
		const auto& cd = ey.second;

		std::cout << "(" << i << "/" << URItable.size() << ") " << cd.caption << "...\n";
		download_image(cd.uri_obverse, "coin obverse", cd, o);
		download_image(cd.uri_reverse, "coin reverse", cd, o);
		++i;
	}
	o.close();
	o.clear();
}

void load_caption_file(const std::string& fname, std::unordered_map<std::string, std::string>& captions) {
	std::ifstream i;
	char line[2048];

	i.open(fname);
	forever {
		char* w;
		i.getline(line, 2048);
		if (i.eof())
			break;
		w = strchr(line, ',');
		if (is_null(w))
			continue;
		*w = '\000';
		captions[line] = (w + 1);
	}
	i.close();
	i.clear();
}

void join_machine_captions(void) {
	std::unordered_map<std::string, std::string> machine_captions, ngc_captions;
	std::ofstream o;

	load_caption_file(MACHINE_CAPTIONS, machine_captions);
	load_caption_file("coin-captions.txt", ngc_captions);
	
	o.open("coin-combined-captions.txt");
	for (const auto& ey : ngc_captions) {
		std::string path = ey.first, caption = ey.second;
		if (machine_captions.find(path) != machine_captions.end()) {
			o << path << "," << machine_captions[path] << ", " << caption << std::endl;
		} else {
			o << path << "," << caption << std::endl;
		}
	}
	o.close();
	o.clear();
}

int app_main() {
	build_uri_table();

#ifdef PRINT_TABLE	
	for (const auto& ey : URItable) {
		const auto& cd = ey.second;
		std::cout << cd.caption << "\t" << cd.subtitle << "\t" << cd.grade << std::endl;
	}
#endif	

#ifdef DOWNLOAD_GALLERY_IMAGES
	std::cout << URItable.size() << " coins found in gallery file.\n";
	std::cout << "Downloading coin images...\n";
	download_table();
#endif	

	join_machine_captions();

	return 0;
}

/*** end ngc.cpp ***/
