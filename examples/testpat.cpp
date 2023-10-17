/***

	testpat.cpp

	A short program for testing WAV synthesis using the built-in instrument patches.

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"

void do_scale_chord(const Patch& patch, const char* wavname) {
	// Scale, from C4 to C5.
	const double freq[] = { 261.63, 293.66, 329.63, 349.23, 392., 440., 493.88, 523.26 };
	WavBuild* wb = nullptr, * wb2, * wb3, * wb4;

	for (int e = 0; e < 8; ++e)
		wb = patch.render_msec(wb, 200, 400, 48000, freq[e], 10000);

	// Now let's concatenate a chord, C4 - E4 - G4.
	wb2 = patch.render_msec(nullptr, 800, 1000, 48000, freq[0], 28000);
	wb3 = patch.render_msec(nullptr, 800, 1000, 48000, freq[2], 28000);
	wb4 = patch.render_msec(nullptr, 800, 1000, 48000, freq[4], 28000);

	// Order by length.
	if (wb2->num_samples() < wb3->num_samples())
		SWAP(wb2, wb3, WavBuild *);
	if (wb2->num_samples() < wb4->num_samples())
		SWAP(wb2, wb4, WavBuild *);
	if (wb3->num_samples() < wb4->num_samples())
		SWAP(wb3, wb4, WavBuild *);

	// Mix and concatenate.
	wb2->mix_from_start(wb3);
	wb2->mix_from_start(wb4);
	wb->concatenate(wb2);

	// Render the final .WAV.
	WavFile* wf = wb->render();
	wf->out_to_file(wavname);

	delete wf;
	delete wb;
	delete wb2;
	delete wb3;
	delete wb4;
}

void do_song(void) {
	char melody1[] =
		"T180 V18000 O4 L4 A L2 D5 L8 C#5 B L4 A A A B E5 g# L2 a L4 a L2 G5 L8 F#5 E5 L2 A5 L8 g5 F#5 l4 E5 G5 C#5 L2 D5"
		"L4 A L2 D5 L8 C#5 B L4 A A A B E5 g# L2 a L4 a L2 G5 L8 F#5 E5 L2 A5 L8 g5 F#5 l4 E5 G5 C#5 L2 D5";
	char melody2[] = 
		"T180 V12000 O4 L4 R F# F# F# F# F# F# E E E L2 E L4 R    E  E  E  F# F# F# g g g L2 F#"
			"L4 R F# F# F# F# F# F# E E E L2 E L4 R    E  E  E  F# F# F# g g g L2 F#";
	char melody3[] =
		"T180 V12000 O4 L4 R D  D  D  D  D  D  D D D L2 C# L4 R   C# C# C# D  D  D  e e e L2 D"
			"O4 L4 R D  D  D  D  D  D  D D D L2 C# L4 R   C# C# C# D  D  D  e e e L2 D";
	char melody4[] =
		"T180 V12000 O4 L4 R R	R  R  R  R	R  R R R L2 R L4 R O3 A  A  A  A  A  A  a a a L2 R"
			"O4 L4 R R	R  R  R  R	R  R R R L2 R L4 R O3 A  A  A  A  A  A  a a a L2 R";

	WavRender wr(44100, 1);
	WavBuild* wb;
	std::vector<WavBuild*> notes;
	WavFile* wf;
	// wb = wr.render_melody_str(melody1, wave_sine, nullptr);
	wb = wr.render_melody_str(melody1, wave_sine_sq, nullptr);
	notes.push_back(wb);
	wb = wr.render_melody_str(melody2, wave_sine_sq, nullptr);
	notes.push_back(wb);
	wb = wr.render_melody_str(melody3, wave_square, nullptr);
	notes.push_back(wb);
//	wb = wr.render_melody_str(melody4, wave_sine, &patch_bottle);
	wb = wr.render_melody_str(melody4, wave_square, nullptr);
	notes.push_back(wb);
	wb = wr.mix_voices(notes);
	wf = wb->render();
	wf->out_to_file("melody.wav");
}

void do_buzz(void) {
	WavRender wr(44100, 1);
	WavBuild* wb = nullptr;
	for (u32 amp = 2000; amp < 32768; amp += 100) {
		wb = wr.build_sine(wb, 10, 262.0, amp);
	}
	WavFile* wf = wb->render();
	wf->out_to_file("buzz.wav");
	delete wb;

	int pan = 0;
	wb = nullptr;
	for (int e = 0; e < 1000; ++e) {
		int dp = RandU32();
		wb = wr.build_sine(wb, 10, 440.0, 22000, pan);
		dp = (dp % 17) - 8;
		pan += dp;
	}
	wf = wb->render();
	wf->out_to_file("pan.wav");
	delete wb;
}

void do_beat(void) {
	char rhythm[] = "T120 L4 C C C C C C C C";
	WavRender wr(44100, 1);
	WavBuild* wb = wr.render_melody_str(rhythm, wave_none, &patch_tamborine);
	WavBuild* wb2 = wr.render_melody_str(rhythm, wave_none, &patch_metal_click);
	WavBuild* wb3 = wr.render_melody_str(rhythm, wave_none, &patch_cymsplsh);
	WavBuild* wb4 = wr.render_melody_str(rhythm, wave_none, &patch_claps);
	wb->concatenate(wb2);
	wb->concatenate(wb3);
	wb->concatenate(wb4);
	WavFile* wf = wb->render();
	wf->out_to_file("beat.wav");
}

int main(void) {
	do_scale_chord(patch_bottle, "bottle.wav");
	do_scale_chord(patch_calliope2, "calliope.wav");
	do_song();
	do_buzz();
	do_beat();
	return(0);
}
