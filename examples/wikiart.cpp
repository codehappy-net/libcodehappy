/***

	wikiart.cpp
	
	Create and maintain a WikiArt dataset. Fine paintings are labeled by artist, title, date, genre, school or style, etc.
	in addition to a machine generated BLIP2 caption.

	C. M. Street, April 2023.

***/
#define CODEHAPPY_NATIVE
#include <codehappy.h>


struct Artwork {
	std::string artist;
	std::string title;	// year also, if included
	std::string caption;
	int genre_class;
	int style_class;
};

// keyed on image path
std::unordered_map<std::string, Artwork> art_collection;

std::string genres_styles[] = {
	"abstract painting", "cityscape", "genre painting", "illustration", "landscape", "nude painting", "portrait", "religious painting",
	"sketch and study", "still life", "", "abstract expressionism", "action painting", "analytical cubism", "art nouveau", "baroque",
	"color field painting", "contemporary realism", "cubism", "early Renaissance", "Expressionism", "fauvism", "High Renaissance",
	"impressionism", "mannerism, late Renaissance", "minimalism", "primitivism", "new realism", "northern Renaissance", "pointillism",
	"pop art", "post-impressionism", "realism", "rococo", "romanticism", "symbolism", "synthetic cubism", "ukiyo-e",
};

std::string name_for_class(int class_idx) {
	if (class_idx < 129 || class_idx > 166) {
		return "";
	}
	return genres_styles[class_idx - 129];
}

void clear_dashes(char* w) {
	while (*w) {
		if (*w == '-')
			*w = ' ';
		++w;
	}
}

void load_caption_file(const std::string& fname, std::unordered_map<std::string, char *>& captions) {
	std::ifstream i;
	char line[2048];

	i.open(fname);
	forever {
		char* w, *stra;
		i.getline(line, 2048);
		if (i.eof())
			break;
		w = strchr(line, ',');
		if (is_null(w))
			continue;
		*w = '\000';
		stra = new char [strlen(w + 1) + 1];
		strcpy(stra, w + 1);
		captions[line] = stra;
	}
	i.close();
	i.clear();
}

void read_class_info() {
	std::ifstream i;
	char line[2048];
	i.open("/data/train/wikiart/wikiart/wclasses.csv");
	forever {
		std::string path;
		char* w, * w2;
		i.getline(line, 2048, '\n');
		if (i.eof())
			break;
		w = strchr(line, ',');
		if (is_null(w))
			continue;
		*w = '\000';
		path = "/data/train/wikiart/wikiart/";
		path += line;
		if (art_collection.find(path) == art_collection.end()) {
			std::cout << "Warning: did not find caption for path '" << path << "'.\n";
			continue;
		}
		++w;
		w = strchr(w, ',');
		if (is_null(w))
			continue;
		++w;
		art_collection[path].genre_class = atoi(w);
		w = strchr(w, ',');
		if (is_null(w))
			continue;
		++w;
		art_collection[path].style_class = atoi(w);
		w = strchr(line, '/');
		if (is_null(w))
			continue;
		++w;
		w2 = strchr(w, '_');
		if (is_null(w))
			continue;
		*w2 = '\000';
		clear_dashes(w);
		art_collection[path].artist = w;
		clear_dashes(w2 + 1);
		w = strstr(w2 + 1, ".jpg");
		if (is_null(w))
			w = strstr(w2 + 1, ".png");
		if (is_null(w))
			w = strstr(w2 + 1, ".jpeg");
		if (is_null(w))
			w = strstr(w2 + 1, ".gif");
		if (!is_null(w))
			*w = '\000';
		art_collection[path].title = w2 + 1;
	}
}

void join_captions() {
	std::unordered_map<std::string, char *> blip2;

	load_caption_file("/home/exx/ml/LAVIS/wikiart-captions.txt", blip2);
	for (const auto& ey : blip2) {
		Artwork art;
		art.caption = ey.second;
		art.genre_class = -1;
		art.style_class = -1;
		art_collection[ey.first] = art;
	}

	read_class_info();

	std::ofstream o;
	o.open("wikiart-combined-captions.txt");
	for (const auto& ey : art_collection) {
		std::string v;
		o << ey.first << ",";
		o << ey.second.caption << ", ";
		o << ey.second.artist;
		v = name_for_class(ey.second.genre_class);
		if (!v.empty())
			o << ", " << v;
		v = name_for_class(ey.second.style_class);
		if (!v.empty())
			o << ", " << v;
		if (!ey.second.title.empty())
			o << ", " << ey.second.title;
		o << "\n";
	}
	o.close();
	o.clear();
}

int app_main() {
	join_captions();
	return 0;
}

/* end wikiart.cpp */
