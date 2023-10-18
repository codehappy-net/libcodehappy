/***

	midi.cpp

	MIDI player. Can be pointed to different sound fonts and render any General MIDI file.
	
	Chris Street, 2023

***/
#define CODEHAPPY_NATIVE
#define CODEHAPPY_SDL
#include <libcodehappy.h>

void play_jukebox(const std::string& folder, const std::string& soundfont) {
	GrabBag<std::string> paths;
	DIR* di = opendir(folder.c_str());
	dirent* entry;
	bool first = true;

	while (entry = readdir(di)) {
		std::string path;
		if (strstr(entry->d_name, ".mid") == nullptr)
			continue;
		make_pathname(folder, entry->d_name, path);
		paths.Insert(path, 1);
	}
	closedir(di);

	std::cout << paths.Count() << " MIDI files found in path " << folder << "\n";
	paths.SetReplace(false);
	
	while (!paths.Empty()) {
		std::string path;
		// The soundfont could be very large; we don't want to re-load it each time.
		const char* sfname = (first ? soundfont.c_str() : nullptr);
		path = paths.Select();
		if (play_midi(path.c_str(), sfname) == MIDI_OK) {
			first = false;
			std::cout << "Playing " << path << "...\n";
			while (midi_playing())
				;
		}
	}
}

int app_main() {
	ArgParse ap;
	std::string midi, soundfont, midi_dir;
	
	ap.add_argument("midi", type_string, "the path to the MIDI to play");
	ap.add_argument("jukebox", type_string, "a directory containing MIDI files; the program will enter jukebox mode and play them sequentially in random order");
	ap.add_argument("sf", type_string, "(required) the path to the sound font to use to render the MIDI"); 
	ap.ensure_args(argc, argv);

	ap.value_str("midi", midi);
	ap.value_str("sf", soundfont);
	ap.value_str("jukebox", midi_dir);
	if (midi.empty() && midi_dir.empty()) {
		codehappy_cerr << "*** Error: you must supply either a MIDI file or a directory to play.\n";
		ap.show_help();
		return 1;
	}
	if (soundfont.empty()) {
		codehappy_cerr << "*** Error: you must supply a sound font to use.\n";
		ap.show_help();
		return 1;
	}

	codehappy_init_audiovisuals();
	if (!midi_dir.empty()) {
		play_jukebox(midi_dir, soundfont);
	} else if (play_midi(midi.c_str(), soundfont.c_str()) == MIDI_OK) {
		std::cout << "Playing MIDI...\n";
		while (midi_playing())
			;
	}

	return 0;
}

/*** end midi.cpp ***/
