/***

	astropix.cpp
	
	Create and maintain an Astronomy Picture of the Day (APOD) dataset.

	C. M. Street, April 2023.

***/
#define CODEHAPPY_NATIVE
#include <codehappy.h>

struct APOD {
	int date;	// YYYYMMDD
	std::string caption;
	std::string uri;
};

int month_from_str(const char* w) {
	const char * mos[] = { "", "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };
	int ret;
	for (ret = 1; ret <= 12; ++ret) {
		if (!strncmp(w, mos[ret], strlen(mos[ret])))
			return ret;
	}
	return 0;
}

void build_collection(std::vector<APOD>& coll) {
	RamFile* rf;
	char* w, * w2, * w3;
	rf = FetchURI("https://apod.nasa.gov/apod/archivepixFull.html");
	w = (char *)rf->buffer();
	forever {
		APOD apod;
		w2 = strstr(w, "<a href=\"ap");
		if (is_null(w2))
			break;
		w = w2;
		w2 += 9;
		w3 = strchr(w2, '\"');
		*w3 = '\000';
		apod.uri = w2;
		w2 = w3 + 2;
		w3 = strchr(w2, '<');
		*w3 = '\000';
		apod.caption = w2;
		forever {
			--w;
			if (*w == '\n') {
				++w;
				break;
			}
			apod.date = atoi(w);
			if (apod.date > 1995 && apod.date < 2040) {
				int i;
				w += 5;
				apod.date *= 10000;
				i = month_from_str(w);
				if (i > 0) {
					apod.date += (i * 100);
					while (!isspace(*w)) ++w;
					apod.date += atoi(w);
					coll.push_back(apod);
				}
				break;
			}
		}
		w = w3 + 1;
	}
	delete rf;
}


void download_collection(std::vector<APOD>& coll, const std::string& outfolder, int maxdate) {
	std::string uri;
	std::ofstream o;
	RamFile* rf;
	o.open("astro.csv");
	for (const auto& apod : coll) {
		char* w, * w2;
		std::string pathname, extension;
		if (apod.date > maxdate)
			continue;
		uri = "https://apod.nasa.gov/apod/" + apod.uri;
		std::cout << apod.date << "\t" << apod.caption << "\n";
		rf = FetchURI(uri.c_str());
		if (is_null(rf))
			goto LCont;
		w = (char *)rf->buffer();
		w = strstr(w, "<a href=");
		if (is_null(w))
			goto LCont;
		++w;
		w = strstr(w, "<a href=");
		if (is_null(w))
			goto LCont;
		w += 9;
		w2 = strchr(w, '\"');
		*w2 = '\000';

		uri = "https://apod.nasa.gov/apod/";
		uri += w;
		if (strstr(w, ".jpg") != nullptr)
			extension = ".jpg";
		else if (strstr(w, ".jpeg") != nullptr)
			extension = ".jpg";
		else if (strstr(w, ".png") != nullptr)
			extension = ".png";
		else if (strstr(w, ".gif") != nullptr)
			extension = ".gif";
		else
			goto LCont;

		delete rf;
		rf = FetchURI(uri.c_str());
		if (is_null(rf))
			goto LCont;

		uri = std::to_string(apod.date) + extension;
		make_pathname(outfolder, uri, pathname);
		rf->write_to_file(pathname);
		o << pathname << "," << apod.caption << "\n";
		o.flush();
LCont:		delete rf;
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

void join_captions(const std::string& caption_fname) {
	std::unordered_map<std::string, std::string> astro;
	std::unordered_map<std::string, std::string> blip2;
	std::ofstream o;
	
	load_caption_file("astro_full.csv", astro);
	load_caption_file(caption_fname, blip2);

	o.open("outcaptions.txt");
	for (const auto& ey : astro) {
		std::string path = ey.first, caption = ey.second;
		if (blip2.find(path) != blip2.end()) {
			o << path << "," << caption << ", " << blip2[path] << std::endl;
		} else {
			o << path << "," << caption << std::endl;
		}
	}
	o.close();
	o.clear();
}

int app_main() {
	ArgParse ap;
	std::vector<APOD> collection;
	std::string outfolder = "/data/train/dataset/astro/";
	std::string joincaptions;
	int maxdate = 99999999;

	ap.add_argument("outfolder", type_string, "output folder [default is '/data/train/dataset/astro']");
	ap.add_argument("max-date", type_int, "Maximum date (in YYYYMMDD integer form) of APOD to download.", &maxdate);
	ap.add_argument("join-captions", type_string, "BLIP2 COCO captions to join with the APOD captions.");
	ap.ensure_args(argc, argv);
	if (ap.flag_present("outfolder")) {
		outfolder = ap.value_str("outfolder");
	}
	if (ap.flag_present("join-captions")) {
		joincaptions = ap.value_str("join-captions");
	}
	
	if (!joincaptions.empty()) {
		join_captions(joincaptions);
		return 0;
	}
	
	// Default behavior: build the APOD collection and download images.
	if (FileExists("astro.csv")) {
		std::cerr << "astro.csv exists -- we don't want to overwrite it.\n";
		return 1;
	}
	build_collection(collection);
	download_collection(collection, outfolder, maxdate);

	// TODO, Can also: combine APOD's captions with the LAVIS BLIP2 COCO captions to create a training dataset.

	return 0;
}
