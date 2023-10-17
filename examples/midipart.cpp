/***

	midipart.cpp

	Render a MIDI into parts (one for each program/patch). This can be used
	to render with mixed soundfonts. 

	Copyright (c) 2022 Chris Street.

***/

#define CODEHAPPY_NATIVE
#include <libcodehappy.h>

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: midipart [MIDI file]\n";
		return 1;
	}
	const char* midi_path = app_argv(1);
	WavRender wr(44100, 2);
	tsf* soundfonts[7];
	const char* short_names[] = { "tou", "tim", "cri", "dsf", "wee", "cho", "ome" };

	std::cout << "Loading soundfonts...\n";
	soundfonts[0] = wr.load_soundfont_for_render("\\soundfonts\\Touhou.sf2");
	soundfonts[1] = wr.load_soundfont_for_render("\\soundfonts\\Timbres Of Heaven GM_GS_XG_SFX V 3.4 Final.sf2");
	soundfonts[2] = wr.load_soundfont_for_render("\\soundfonts\\CrisisGeneralMidi3.01.sf2");
	soundfonts[3] = wr.load_soundfont_for_render("\\soundfonts\\DSoundFont_Plus_V4.sf2");
	soundfonts[4] = wr.load_soundfont_for_render("\\soundfonts\\WeedsGM3.sf2");
	soundfonts[5] = wr.load_soundfont_for_render("\\soundfonts\\choriumreva.sf2");
	soundfonts[6] = wr.load_soundfont_for_render("\\soundfonts\\OmegaGMGS2.sf2");

	/* Render the midi parts. */
	std::unordered_set<int> programs;
	midi_programs_used(midi_path, programs);
	std::cout << programs.size() << " MIDI programs used.\n";
	std::cout << "Rendering MIDI...\n";
	for (auto p : programs) {
		char fname[2048];
		std::cout << "Program " << p << "...\n";
		for (int e = 0; e < 7; ++e) {
			sprintf(fname, "%s.%03d.%s.wav", filename_from_path(midi_path), p, short_names[e]);
			WavBuild* wb = wr.build_midi(nullptr, midi_path, soundfonts[e], p);
			WavFile* wf = wb->render();
			wf->out_to_file(fname);
			delete wf;
			delete wb;
		}
	}
	std::cout << "Render complete.\n";

	return 0;
}
