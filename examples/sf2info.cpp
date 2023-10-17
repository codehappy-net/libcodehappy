/***

	sf2info.cpp

	Loads a SoundFont (.sf2) and displays info about it.

	Copyright (c) 2022 Chris Street.

***/
#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: sf2info [path to soundfont]\n";
		return 1;
	}
	tsf* sf2 = tsf_load_filename(app_argv(1));
	if (is_null(sf2)) {
		std::cout << "Error loading soundfont " << app_argv(1) << std::endl;
		return 2;
	}
	int pc = tsf_get_presetcount(sf2);
	printf("This soundfont contains %d presets.\n", pc);
	printf("List of presets [Index / bank / MIDI preset]:------------------------------\n");
#if 0
	for (int e = 0; e < pc; ++e) {
		printf("%03d -- %s\n", e, tsf_get_presetname(sf2, e));
	}
#else
	for (int e = 0; e < pc; ++e) {
		int bank, midi_preset;
		tsf_get_bank_midi_preset(sf2, e, &bank, &midi_preset);
		printf("%03d | %02d | %03d -- %s\n", e, bank, midi_preset, tsf_get_presetname(sf2, e));
	}
#endif
	return 0;
}
