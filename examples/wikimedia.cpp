/***

	wikimedia.cpp
	
	Create and maintain a Wikimedia Quality Images dataset.

	C. M. Street, May 2023.

***/
#define CODEHAPPY_NATIVE
#include <codehappy.h>

const std::string wikimedia_pages[] = {
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Exposure",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Composition",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Movement_control",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Depth_of_field",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Perspective",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Proportion",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Technical/Color",
"https://commons.wikimedia.org/wiki/Special:MyLanguage/Commons:Quality_images/Subject/Animals/Birds",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Birds/Archive_2020-2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Birds/Archive_2018-2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Birds/Archive_2016-2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Birds/Archive_up_to_2015",
"https://commons.wikimedia.org/wiki/Special:MyLanguage/Commons:Quality_images/Subject/Animals/Mammals",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Arthropods",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Molluscs",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Fish",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Amphibians",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Animals/Reptiles",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Astronomy",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Events",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Events/Archive_2008_to_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Food_and_drink",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Fungi",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Natural_phenomena",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Electronics_%26_electrical",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Household_Items",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Industrial",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Cameras,_Optics_and_Microscopes",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Statues,_Monuments_and_Plaques",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Aerial_Trams",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Steam_Powered",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Railway",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Cycles_and_Motorcycles",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Boats_and_Ships",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Automobiles",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Balloons,_Aeroplanes,_Helicopters_etc",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Transport_and_Vehicles/Other_vehicles",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Geological_objects",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Closeups_of_Structures",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other/Archive_2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other/Archive_2019_to_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other/Archive_2017_to_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other/Archive_2015_to_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Objects/Other/Archive_2013_to_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/People/Archive_2013",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Mixed",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2008_to_2012",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2013_to_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Natural_structures/archive_2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Mixed/Archive4",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Mixed/Archive3",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Mixed/Archive2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Mixed/Archive1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Agricultural_and_Industrial",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Other",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Ruins",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Towers_and_Masts",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Bridges",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Other",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Other/Archive_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Other/Archive_2018_to_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Other/Archive_2015_to_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Bridges/Archive_2019_to_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Bridges/Archive_2017_to_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Bridges/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Transport_Infrastructure/Bridges/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Towers_and_Masts/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Towers_and_Masts/Archive_2016_to_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Ruins/Archive_2018_to_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Ruins/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Ruins/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Ruins/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2022",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Residential_Buildings/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Other/Archive_2018_to_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Other/Archive_2016_to_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2022",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2021-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2021-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2020-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2020-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2019-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2019-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2018-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2018-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2017-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2017-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2016-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2016-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2015-3",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2015-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2015-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2014-2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Religious/Churches/Archive_2014-1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Public_Buildings/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Interior/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2020-2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Feudal_(Castles,_Palaces)/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2022",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2021",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2020",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Close-ups/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Cityscapes/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Agricultural_and_Industrial/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Agricultural_and_Industrial/Archive_2017_to_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Architecture/Agricultural_and_Industrial/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Exterior",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Exterior/Part_4",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Exterior/Part_3",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Exterior/Part_2",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Exterior/Part_1",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Places/Man_made_structures/Buildings,_Interior",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Foliage_etc/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Foliage_etc/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Foliage_etc/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Foliage_etc/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Trees/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Trees/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Trees/Archive_2015-2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Fruit,_berries,_seeds_etc/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Fruit,_berries,_seeds_etc/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Fruit,_berries,_seeds_etc/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Fruit,_berries,_seeds_etc/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Plant_life/Flowers/Archive_2012-2013",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sunsets",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2019",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Works_of_art/Archive_2013-2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports/Archive_2018",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports/Archive_2017",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports/Archive_2016",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports/Archive_2015",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Sports/Archive_2014",
"https://commons.wikimedia.org/wiki/Commons:Quality_images/Subject/Microscopic",
};

void remove_tags(char* w) {
	char* wo = w, * w2, *we = w + strlen(w);
	forever {
		w2 = strchr(w, '<');
		if (is_null(w2))
			break;
		w = w2;
		w2 = strchr(w, '>');
		if (is_null(w2))
			break;
		++w2;
		memcpy(w, w2, strlen(w2) + 1);
	}
	bool zero = false;
	while (wo < we) {
		zero = zero || (*wo == 0);
		if (*wo == '\n' || *wo == '\r' || zero)
			*wo = ' ';
		++wo;
	}
}

void build_collection_page(const std::string& URI, std::vector<std::string>& coll) {
	RamFile* rf;
	char* w, * w2, * w3;

	std::cout << URI << "...\n";
	rf = FetchURI(URI);
	NOT_NULL_OR_RETURN_VOID(rf);
	w = (char *)rf->buffer();

	forever {
		std::string add;
		char* w2;

		w = strstr(w, "href=\"/wiki/File:");
		if (is_null(w))
			break;
		w += 6;
		add = "https://commons.wikimedia.org";
		w2 = strchr(w, '\"');
		if (is_null(w2))
			break;
		*w2 = '\000';
		add += w;
		coll.push_back(add);
		w = w2 + 1;
	}

	delete rf;
}

void build_collection(std::vector<std::string>& coll) {
	if (FileExists("wikimedia_uris.txt")) {
		// Load the URLs from file.
		std::ifstream i;
		std::cout << "Reading Wikimedia URIs from file 'wikimedia_uris.txt'...\n";
		i.open("wikimedia_uris.txt");
		forever {
			std::string l;
			std::getline(i, l);
			if (i.eof())
				break;
			coll.push_back(l);
		}
		i.close();
		i.clear();
		return;
	}
	// note that when this is done I should use std::unordered_set<> then copy to a vector...
	for (const auto& URI : wikimedia_pages) {
		build_collection_page(URI, coll);
	}
	std::ofstream o;
	o.open("wikimedia_uris.txt");
	for (const std::string& URI : coll) {
		o << URI << std::endl;
	}
	o.close();
	o.clear();
}

void download_collection_uri(std::vector<std::string>& coll, const std::string& outfolder, std::ofstream& o, const std::string& URI, int idx)  {
	RamFile* rf;
	char* w, * w2;
	std::string caption;
	std::string orig_uri;

	std::cout << "Index " << idx << "; reading caption and data from page '" << URI << "'...\n";
	rf = FetchURI(URI);
	NOT_NULL_OR_RETURN_VOID(rf);
	w = (char *)rf->buffer();

	w = strstr(w, "fullImageLink");
	if (is_null(w)) {
		delete rf;
		return;
	}
	w = strstr(w, "href=\"");
	w += 6;
	w2 = strchr(w, '\"');
	*w2 = '\000';
	orig_uri = w;
	*w2 = '\"';
	w = (char *)rf->buffer();

	/* read caption */	
	w2 = strstr(w, "wbmi-entityview-captions-header'>Captions");
	if (w2 != nullptr) {
		w2 = strstr(w2, "English</label>");
		if (w2 != nullptr) {
			w2 = strstr(w2, "wbmi-caption-value");
			w2 = strchr(w2, '>') + 1;
			if (strncmp(w2, "Add a one-line", 14)) {
				w = strstr(w2, "</div>");
				*w = '\000';
				remove_tags(w2);
				caption = w2;
				*w = '<';
			}
		}
	}

	if (caption.empty()) {
		w2 = strstr(w, "en\" title=\"English\"><b>English");
		if (is_null(w2)) {
			w2 = strstr(w, "style=\"font-weight:bold;\"><bdi>English</bdi>");
		}
		if (is_null(w2)) {
			w2 = strstr(w, "title=\"English\"><b>English");
		}
		if (w2 != nullptr) {
			w2 = strstr(w2, "</span>");
			w2 += 7;
			if (isspace(*w2))
				++w2;
			w = strstr(w2, "</div>");
			*w = '\000';
			remove_tags(w2);
			caption = w2;
			*w = '<';
		}
	}

	/* download, rescale, and save image */
	RamFile* rf_img;
	w = (char *)rf->buffer();
	std::cout << "Downloading image '" << orig_uri << "'...\n";
	rf_img = FetchURI(orig_uri);
	delete rf;
	NOT_NULL_OR_RETURN_VOID(rf_img);

	SBitmap* bmp = SBitmap::load_bmp(rf_img);
	std::string img_uri = std::to_string(idx), pathname;
	img_uri += ".jpg";
	make_pathname(outfolder, img_uri, pathname);
	if (is_null(bmp)) {
		delete rf_img;
		return;
	}
	if (bmp->height() > 1024 && bmp->width() > 1024) {
		if (bmp->height() < bmp->width()) {
			bmp->resize_and_replace(0, 1024);
		} else {
			bmp->resize_and_replace(1024, 0);
		}
		bmp->save_bmp(pathname);
	} else {
		rf_img->write_to_file(pathname);
	}

	delete bmp;
	delete rf_img;

	/* save the image location and caption to the output .csv file */
	o << pathname << "," << caption << std::endl;
}

void download_collection(std::vector<std::string>& coll, const std::string& outfolder, int index_start) {
	std::ofstream o;
	int idx = 0;
	o.open("wikimedia.csv", std::ios::app | std::ios::out);
	for (const std::string& URI : coll) {
		if (idx < index_start) {
			++idx;
		} else {
			download_collection_uri(coll, outfolder, o, URI, idx++);
		}
	}
	o.close();
	o.clear();
}

void load_caption_file(const std::string& fname, std::unordered_map<std::string, std::string>& captions) {
	std::ifstream i;
	char* line;

	line = new char [65536];
	i.open(fname);
	forever {
		char* w;
		i.getline(line, 65536);
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
	delete line;
}

void join_captions(const std::string& caption_fname) {
	std::unordered_map<std::string, std::string> wikimedia;
	std::unordered_map<std::string, std::string> blip2;
	std::ofstream o;
	
	load_caption_file("wikimedia.csv", wikimedia);
	load_caption_file(caption_fname, blip2);

	o.open("outcaptions.txt");
	for (const auto& ey : wikimedia) {
		std::string path = ey.first, caption = ey.second;
		std::string s;
		s = caption;

		/* caption preprocessing: remove unnecessary whitespace */
		for (int i = 0; i < 8; ++i) {
			p_replace(s, "  ", " ");
		}
		/* caption preprocessing: replace characters that I have given special interpretations */
		p_replace(s, "[", "(");
		p_replace(s, "{", "(");
		p_replace(s, "|", " ");
		p_replace(s, "]", ")");
		p_replace(s, "}", ")");
		p_replace(s, ":", "-");
		
		if (blip2.find(path) != blip2.end()) {
			o << path << "," << blip2[path] << ", real world, " << s << ", Wikimedia" << std::endl;
		} else {
			o << path << "," << s << ", real world, Wikimedia" << std::endl;
		}
	}
	o.close();
	o.clear();
}


int app_main() {
	ArgParse ap;
	std::vector<std::string> collection;
	std::string outfolder = "/data/train/dataset/wikimedia/";
	std::string joincaptions;
	int index_start = 0;

	ap.add_argument("outfolder", type_string, "output folder [default is '/data/train/dataset/wikimedia']");
	ap.add_argument("join-captions", type_string, "BLIP2 COCO captions to join with the Wikimedia-loaded captions.");
	ap.add_argument("index", type_int, "the index at which to start the download (on resumption)", &index_start);
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
	
	// Default behavior: build the wikimedia collection and download images.
	build_collection(collection);
	std::cout << collection.size() << " image pages found in the quality images collection.\n";
	download_collection(collection, outfolder, index_start);

	return 0;
}
