/***

	midi.cpp

	MIDI player using TSF and TML libraries. Can be pointed to
	different sound fonts and render any General MIDI file.

	Define "JUKEBOX" and it will randomly play MIDI files
	in the specified directory.

***/
#define	SF2_FOLDER	"\\soundfonts\\"
#define MIDI_FOLDER	"\\magic_of_midi\\"
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>

#ifndef TSF_RENDER_EFFECTSAMPLEBLOCK
#define TSF_RENDER_EFFECTSAMPLEBLOCK 64
#endif

struct soundfonts {
	const char* name;
	const char* longdesc;
	const char* path;
};

static soundfonts sfs[] = {
	{ " DSF",	"DSoundFont+ (large, high quality)",		SF2_FOLDER "DSoundFont_Plus_V4.sf2" },
	{ " Touhou",	"Touhou General MIDI (generally awesome)",	SF2_FOLDER "Touhou.sf2" },
	{ " Omega",	"Default (OmegaGMGS2)",				SF2_FOLDER "OmegaGMGS2.sf2" },
	{ " Megadrive",	"Megadrive (Sega Genesis)",			SF2_FOLDER "The_Ultimate Megadrive_Soundfont.sf2" },
	{ " SGM",	"Shan's GM Soundfont",				SF2_FOLDER "SGM-V2.01.sf2" },
	{ " Undertale",	"Undertale",					SF2_FOLDER "undertale.sf2" },
	{" Megalovania","Definitive Megalovania v.1.19",		SF2_FOLDER "megalovania.sf2" },
	{ " 8bits",	"8bits (NES style)",				SF2_FOLDER "8bitsf.sf2" },
	{ " Essential",	"Essential Keys sforzondo v9.6",		SF2_FOLDER "Essential Keys-sforzando-v9.6.sf2" },
	{ " Chateau",	"Chateau Grand Plus instruments",		SF2_FOLDER "ChateauGrand-Plus-Instruments-bs16i-v3.0.sf2" },
	{ " Airfont",	"Airfont 380 final",				SF2_FOLDER "airfont_380_final.sf2" },
	{ " SC-55",	"Roland SC-55",					SF2_FOLDER "Roland_SC-55_by_StrikingUAC.sf2" },
	{ " SC-88",	"Roland SC-88",					SF2_FOLDER "Roland_SC-88.sf2" },
	{ " Sonivox",	"Sonivox GS 250 MB",				SF2_FOLDER "SONiVOX_GS250.sf2" },
	{ " Timbres",	"Timbres of Heaven v3.4",			SF2_FOLDER "Timbres Of Heaven GM_GS_XG_SFX V 3.4 Final.sf2" },
	{ " Merlin",	"Merlin Symphony v1.21",			SF2_FOLDER "merlin_symphony(v1.21).sf2" },
	{ " Arachno",	"Arachno v. 1.0",				SF2_FOLDER "Arachno_SoundFont_Version_1.0.sf2" },
	{ " Fairy",	"The Fairy Tale Bank",				SF2_FOLDER "The_Fairy_Tale_Bank.sf2" },
	{ " Somsak",	"SOMSAK 2016 v2.8",				SF2_FOLDER "Soundfont_SOMSAK_2016-V2.8.SF2" },
	{ " Nokia",	"Nokia S30",					SF2_FOLDER "Nokia_S30.sf2" },
	{ " N64",	"Nintendo64",					SF2_FOLDER "Nintendo_64_ver_2.0.sf2" },
	{ " MT-32",	"Roland MT-32",					SF2_FOLDER "MT32.sf2" },
	{ " Microsoft",	"Microsoft GM",					SF2_FOLDER "Microsoft_gm.sf2" },
	{ " MagicSF",	"MagicSF v2.0",					SF2_FOLDER "MagicSFver2.sf2" },
	{ " MuseScore",	"MuseScore General v.0.1.3",			SF2_FOLDER "MuseScore_General(v0.1.3).sf2" },
	{ " Musica",	"Musica Theoria GM v2",				SF2_FOLDER "Musica_Theoria_v2_(GM).sf2" },
	{ " FatBoy",	"FatBoy v0.786",				SF2_FOLDER "FatBoy-v0.786.sf2" },	
	{ " SNES",	"Super Nintendo Unofficial Update",		SF2_FOLDER "Super_Nintendo_Unofficial_update.sf2" },
	{ " Weeds",	"Weeds GM3",					SF2_FOLDER "WeedsGM3.sf2" },
	{ " Wii",	"Ultimate Wii Soundfont V1.1",			SF2_FOLDER "The_Ultimate_Wii_Soundfont_V1-1.sf2" },
	{ " OPL",	"OPL-3 FM (SoundBlaster 16) 128MB",		SF2_FOLDER "OPL-3_FM_128M(Sound_Blaster_16).sf2" },
	{ " PC51",	"PC51d",					SF2_FOLDER "PC51d.sf2" },
};

// Holds the global instance pointer
static tsf* g_TinySoundFont;

// Holds global MIDI playback state
static double g_Msec = 0.;               //current playback time
static tml_message* g_MidiMessage;  //next message to be played
static volatile bool _renderzip = false;

// Callback function called by the audio thread
static void AudioCallback(void* data, Uint8 *stream, int len)
{
	//Number of samples to process
	int SampleBlock, SampleCount = (len / (2 * sizeof(short))); //2 output channels
	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short))))
	{
		//We progress the MIDI playback and then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		if (_renderzip) {
			for (int e = 0; e < SampleBlock; ++e)
				stream[e] = 0;
			return;
		}

		//Loop through all MIDI messages which need to be played up until the current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next)
		{
			switch (g_MidiMessage->type)
			{
				case TML_PROGRAM_CHANGE: //channel program (preset) change (special handling for 10th MIDI channel with drums)
					tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, (g_MidiMessage->channel == 9));
					break;
				case TML_NOTE_ON: //play a note
					tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
					break;
				case TML_NOTE_OFF: //stop a note
					tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
					break;
				case TML_PITCH_BEND: //pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE: //MIDI controller messages
					tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}

		// Render the block of audio samples in short format
		tsf_render_short(g_TinySoundFont, (short*)stream, SampleBlock, 0);
	}
}

const char* process_arg(const char* name) {
	static char buf[1024];
	if (name[0] == '\"')
		strcpy(buf, name + 1);
	else
		strcpy(buf, name);
	char* w = (char *)buf + strlen(buf) - 1;
	while (w > buf && isspace(*w)) {
		*w = '\000';
		--w;
	}
	for (const auto& sf : sfs) {
		w = (char *)strstr(buf, sf.name);
		if (w) {
			--w;
			while (w > buf && isspace(*w))
				--w;
			++w;
			*w = '\000';
		}
	}
	if (buf[strlen(buf) - 1] == '\"')
		buf[strlen(buf) - 1] = '\000';
	return buf;
}

#ifdef JUKEBOX
void ready_soundfont(const char* path) {
	g_TinySoundFont = tsf_load_filename(path);
	if (!g_TinySoundFont) {
		fprintf(stderr, "Could not load SoundFont %s\n", path);
		exit(1);
	}
	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);
	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, 44100, 0.0f);
}

void jukebox_mode(const char* sfpath) {
	ready_soundfont(sfpath);
	std::vector<std::string> files;
	std::cout << "Scanning directory " << MIDI_FOLDER << " for MIDI files...\n";
	DIR* dir = opendir(MIDI_FOLDER);
	dirent* entry;
	while (entry = readdir(dir)) {
		if (strstr(entry->d_name, ".mid") || strstr(entry->d_name, ".MID")) {
			std::string pfx = MIDI_FOLDER;
			files.push_back(pfx + entry->d_name);
		}
	}
	closedir(dir);
	std::cout << files.size() << " MIDI files found!\n";

	forever {
		u32 idx = RandU32Range(0, files.size() - 1);
		tml_message* TinyMidiLoader = NULL, *orig;

		std::cout << "Loading #" << idx << ", '" << files[idx] << "' (" << filelen(files[idx].c_str()) << " bytes)...\n";
		TinyMidiLoader = tml_load_filename(files[idx].c_str());
		if (is_null(TinyMidiLoader)) {
			std::cout << "Error loading, skipping.\n";
			continue;
		}
		orig = TinyMidiLoader;
		g_MidiMessage = TinyMidiLoader;
		_renderzip = false;
		SDL_PauseAudio(0);
		forever {
#if 0
			u8* key_state = SDL_GetKeyState(nullptr);
			if (key_state[SDLK_ESCAPE]) {
				// Fast-forward to the end of the MIDI file.
				std::cout << "Skipping...\n";
				TinyMidiLoader = g_MidiMessage;
				while (!is_null(TinyMidiLoader) && !is_null(TinyMidiLoader->next))
					TinyMidiLoader = TinyMidiLoader->next;
				g_MidiMessage = TinyMidiLoader;
				tsf_reset(g_TinySoundFont);
				tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);
			}
#endif
			SDL_Delay(100);
			if (is_null(g_MidiMessage)) {
				// Wait a few seconds for any buffered samples to finish, then free MIDI data.
				SDL_Delay(700);
				tsf_reset(g_TinySoundFont);
				SDL_Delay(700);
				_renderzip = true;
				SDL_Delay(700);
				SDL_PauseAudio(1);
				g_Msec = 0.0;
				tml_free(orig);
				break;
			}
		}
	}

}
#endif  // JUKEBOX

int app_main() {
	tml_message* TinyMidiLoader = NULL;

#ifdef JUKEBOX
	printf("Usage: jukebox [optional soundfont name]\n");
	printf("Recognized soundfont names: \n");
	for (const auto& sf : sfs) {
		printf("\t%s%s\t%s\n", sf.name, (strlen(sf.name) > 7 ? "" : "\t"), sf.longdesc);
	}
#else
	printf("Usage: midi [MIDI file] {Optional soundfont name} {render}\n");
	printf("Recognized soundfont names: \n");
	for (const auto& sf : sfs) {
		printf("\t%s%s\t%s\n", sf.name, (strlen(sf.name) > 7 ? "" : "\t"), sf.longdesc);
	}
	printf("Args: %d\n", app_argc());
	if (app_argv(1) != nullptr)
		printf(" argv[1] = '%s'\n", app_argv(1));
	if (app_argv(2) != nullptr)
		printf(" argv[2] = '%s'\n", app_argv(2));
	if (app_argv(3) != nullptr)
		printf(" argv[3] = '%s'\n", app_argv(3));
#endif

	// Define the desired audio output format we request
	SDL_AudioSpec OutputAudioSpec;
	OutputAudioSpec.freq = 44100;
	OutputAudioSpec.format = AUDIO_S16;
	OutputAudioSpec.channels = 2;
	OutputAudioSpec.samples = 4096;
	OutputAudioSpec.callback = AudioCallback;
	SDL_AudioSpec ActualAudioSpec;

	// Initialize the audio system
	if (SDL_AudioInit(0) < 0)
	{
		fprintf(stderr, "Could not initialize audio hardware or driver\n");
		return 1;
	}

	g_MidiMessage = nullptr;
#ifndef JUKEBOX
	if (app_argc() > 1) {
		printf("Loading MIDI %s...\n", app_argv(1));
		TinyMidiLoader = tml_load_filename(app_argv(1));
	} else {
		TinyMidiLoader = tml_load_filename("examples/giveyouup.mid");
	}
	if (!TinyMidiLoader)
	{
		fprintf(stderr, "Could not load MIDI file\n");
		return 1;
	}

	//Set up the global MidiMessage pointer to the first MIDI message
	g_MidiMessage = TinyMidiLoader;
#endif  // !JUKEBOX

	// Load the appropriate SoundFont from file
	bool loaded = false;
#ifdef JUKEBOX
	const char* sfpath = "\\soundfonts\\OmegaGMGS2.sf2";
	if (app_argc() > 1) {
		for (const auto& sf : sfs) {
			if (!__stricmp(sf.name + 1, app_argv(1))) {
				printf("Loading %s soundfont...\n", sf.longdesc);
				sfpath = sf.path;
				loaded = true;
				break;
			}
		}
		// Allow simply specifying a pathname, as well.
		if (!loaded && FileExists(app_argv(1))) {
			sfpath = app_argv(1);
			loaded = true;
		}
	}
	if (!loaded) {
		printf("Loading default soundfont...\n");
	}

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, &ActualAudioSpec) < 0)
	{
		fprintf(stderr, "Error msg: %s\n", SDL_GetError());
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
		return 1;
	}

	jukebox_mode(sfpath);
	return 0;
#else
	bool render = false;
	if (app_argc() > 2) {
		for (const auto& sf : sfs) {
			if (!__stricmp(sf.name + 1, app_argv(2))) {
				printf("Loading %s soundfont...\n", sf.longdesc);
				g_TinySoundFont = tsf_load_filename(sf.path);
				loaded = true;
				break;
			}
		}
		// Allow simply specifying a pathname, as well.
		if (!loaded && FileExists(app_argv(2))) {
			printf("Loading soundfont path '%s'...\n", app_argv(2));
			g_TinySoundFont = tsf_load_filename(app_argv(2));
			loaded = true;
		}
		if (app_argc() > 3) {
			render = true;
		}
	}

	if (!g_TinySoundFont) {
		fprintf(stderr, "Could not load SoundFont\n");
		return 1;
	}

	//Initialize preset on special 10th MIDI channel to use percussion sound bank (128) if available
	tsf_channel_set_bank_preset(g_TinySoundFont, 9, 128, 0);

	// Set the SoundFont rendering output mode
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, OutputAudioSpec.freq, 0.0f);

	if (render) {
		std::cout << "Rendering MIDI file...\n";
		WavRender wr;
		WavBuild* wb = wr.build_midi(nullptr, g_MidiMessage, g_TinySoundFont);
		WavFile* wf = wb->render();
		wf->out_to_file("midi.wav");
		std::cout << "Output to 'midi.wav'.\n";
		return 0;
	}

	// Request the desired audio output format
	if (SDL_OpenAudio(&OutputAudioSpec, &ActualAudioSpec) < 0)
	{
		fprintf(stderr, "Error msg: %s\n", SDL_GetError());
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
		return 1;
	}

	printf("Rendering MIDI file to audio out...\n");

	// Start the actual audio playback here
	// The audio thread will begin to call our AudioCallback function
	SDL_PauseAudio(0);

	//Wait until the entire MIDI file has been played back (until the end of the linked message list is reached)
	while (g_MidiMessage != NULL) SDL_Delay(100);

	// We could call tsf_close(g_TinySoundFont) and tml_free(TinyMidiLoader)
	// here to free the memory and resources but we just let the OS clean up
	// because the process ends here.
#endif // !JUKEBOX
	return 0;
}
