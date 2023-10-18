/***

	wavrender.h

	Render WAV files, to use as music/sound effects, etc. Supports 16-bit PCM format
	with 1 or 2 channels and any valid sample rate. Support for MIDI rendering with
	soundfonts, music melodies from strings, etc.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __WAV_RENDER__
#define __WAV_RENDER__

#include <unordered_set>

#define CHANNEL_RIGHT	1
#define CHANNEL_LEFT	2
#define CHANNEL_BOTH	3
#define PAN_LEFT	(-100)
#define PAN_RIGHT	(100)
#define PAN_CENTER	(0)
#define AMPLITUDE_DEFAULT	24000
#define AMPLITUDE_MAX		32767

/* Forward declarations */
struct tsf;
struct tml_message;

enum MusicalNote {
	note_C = 0,
	note_Csharp,
	note_D,
	note_Dsharp,
	note_E,
	note_F,
	note_Fsharp,
	note_G,
	note_Gsharp,
	note_A,
	note_Asharp,
	note_B,
	note_Esharp = note_F,
	note_Bsharp = note_C,
	note_Cflat = note_B,
	note_Dflat = note_Csharp,
	note_Eflat = note_Dsharp,
	note_Fflat = note_E,
	note_Gflat = note_Fsharp,
	note_Aflat = note_Gsharp,
	note_Bflat = note_Asharp,
	note_Cdsharp = note_D,
	note_Ddsharp = note_E,
	note_Edsharp = note_Fsharp,
	note_Fdsharp = note_G,
	note_Gdsharp = note_A,
	note_Adsharp = note_B,
	note_Bdsharp = note_Csharp,
	note_Cdflat = note_Asharp,
	note_Ddflat = note_C,
	note_Edflat = note_D,
	note_Fdflat = note_Eflat,
	note_Gdflat = note_F,
	note_Adflat = note_G,
	note_Bdflat = note_A,
};

enum NoteStyle {
	style_pizzicato,
	style_stacatto,
	style_legato,
	style_legato_full,
	style_fermata,
};

enum Waveform {
	wave_sine = 1,
	wave_square,
	wave_saw,
	wave_sine_sq,
	wave_none = 0
};

enum MidiProgram {
	midi_grand_piano = 0,
	midi_bright_piano,
	midi_electric_grand,
	midi_honky_tonk,
	midi_electric_piano_1,
	midi_electric_piano_2,
	midi_harpsichord,
	midi_clavier,
	midi_celesta,
	midi_glockenspiel,
	midi_music_box,
	midi_vibraphone,
	midi_marimba,
	midi_xylophone,
	midi_tubular_bells,
	midi_dulcimer,
	midi_drawbar_organ,
	midi_percussive_organ,
	midi_rock_organ,
	midi_church_organ,
	midi_reed_organ,
	midi_accordion,
	midi_harmonica,
	midi_tango_accordion,
	midi_nylon_guitar,
	midi_steel_guitar,
	midi_jazz_guitar,
	midi_clean_guitar,
	midi_muted_guitar,
	midi_overdriven_guitar,
	midi_distortion_guitar,
	midi_guitar_harmonics,
	midi_acoustic_bass,
	midi_fingered_bass,
	midi_pick_bass,
	midi_fretless_bass,
	midi_slap_bass_1,
	midi_slap_bass_2,
	midi_synth_bass_1,
	midi_synth_bass_2,
	midi_violin,
	midi_viola,
	midi_cello,
	midi_contrabass,
	midi_tremolo_strings,
	midi_pizzicato_strings,
	midi_orchestral_harp,
	midi_timpani,
	midi_string_ensemble_1,
	midi_string_ensemble_2,
	midi_synth_strings_1,
	midi_synth_strings_2,
	midi_choir_aah,
	midi_voice_ooh,
	midi_synth_voice,
	midi_orchestral_hit,
	midi_trumpet,
	midi_trombone,
	midi_tuba,
	midi_muted_trumpet,
	midi_french_horn,
	midi_brass_section,
	midi_synth_brass_1,
	midi_synth_brass_2,
	midi_soprano_sax,
	midi_alto_sax,
	midi_tenor_sax,
	midi_baritone_sax,
	midi_oboe,
	midi_english_horn,
	midi_bassoon,
	midi_clarinet,
	midi_piccolo,
	midi_flute,
	midi_recorder,
	midi_pan_pipes,
	midi_blown_bottle,
	midi_shakuhachi,
	midi_whistle,
	midi_ocarina,
	midi_square_lead,
	midi_sawtooth_lead,
	midi_calliope_lead,
	midi_chiff_lead,
	midi_charang_lead,
	midi_voice_lead,
	midi_fifths_lead,
	midi_bass_lead,
	midi_new_age_pad,
	midi_warm_pad,
	midi_polysynth_pad,
	midi_choir_pad,
	midi_bowed_pad,
	midi_metallic_pad,
	midi_halo_pad,
	midi_sweep_pad,
	midi_rain_fx,
	midi_soundtrack_fx,
	midi_crystal_fx,
	midi_atmosphere_fx,
	midi_brightness_fx,
	midi_goblins_fx,
	midi_echoes_fx,
	midi_scifi_fx,
	midi_sitar,
	midi_banjo,
	midi_shamisen,
	midi_koto,
	midi_kalimba,
	midi_bagpipe,
	midi_fiddle,
	midi_shanai,
	midi_tinkle_bells,
	midi_agogo,
	midi_steel_drums,
	midi_woodblock,
	midi_taiko,
	midi_melodic_tom,
	midi_synth_drum,
	midi_reverse_cymbal,
	midi_guitar_fret,
	midi_breath_noise,
	midi_seashore,
	midi_bird_tweet,
	midi_telephone_ring,
	midi_helicopter,
	midi_applause,
	midi_gunshot,
};

extern double frequency_note(MusicalNote note, int octave); 
extern u32 msec_beat(u32 tempo_bpm);

struct WavFile {
	WavFile();
	~WavFile();

	u32 len;
	u8* data;

	void out_to_file(const char* pname) const;
	u8* data_chunk_start(void) const;
	WavFile* concatenate(WavFile* wf);
	u16 num_channels(void) const;
	u32 sample_rate(void) const;
	void load_from_file(const char* path);
#ifdef CODEHAPPY_SDL
	// Play and render in SDL.
	Mix_Chunk* sdl_mixchunk(void);
	int play_wav(int loops = 0);
#endif
};

struct WavBuild {
public:
	Scratchpad sp;

	u16 num_channels(void) const;
	u32 sample_rate(void) const;
	u8* data(void) const;
	u8* data_chunk(void) const;
	u16* sample_data(void) const;
	u16* sample_pos(u32 nsample) const;
	void ensure_header(void);
	u32 num_samples(void) const;
	u32 len_msec(void) const;
	u32 data_len(void) const;
	WavFile* render(void);
	void square_wave(u32 nsamples, double freq, u32 amp = 0, int pan = 0);
	void sine_wave(u32 nsamples, double freq, u32 amp = 0, int pan = 0);
	void saw_wave(u32 nsamples, double freq, u32 amp = 0, int pan = 0);
	void sine_sq_wave(u32 nsamples, double freq, u32 amp = 0, int pan = 0);
	void white_noise(u32 nsamples, u32 amp = 0, bool sep_channels = true);
	void pink_noise(u32 nsamples, u32 amp = 0, bool sep_channels = true);
	void silence(u32 nsamples);
	void samples(u32 nsamples, u16* samples);
	bool concatenate(WavBuild* wb);
	WavBuild* copy(void);

	void mix_from_end(WavBuild* wb2);
	void mix_from_end(WavBuild* wb2, u32 nsamples);
	void mix_from_start(WavBuild* wb2, u32 nsamples = 0);

	// Returns an allocated buffer of monophonic samples from the selected channel.
	u16* channel_sample_data(u32 channel_flag) const;
	// Same as above, but returns the samples as floats in [0, 1].
	float* channel_sample_data_f(u32 channel_flag) const;

	// Used to mix mono into stereo. Can provide a stereo pan for the mono samples.
	void mix_mono_in_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples, int pan = 0);
	// Used to mix stereo into mono. Can take the samples from only one channel, or averaged from both.
	void mix_stereo_in_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples, u32 channel_flags = CHANNEL_BOTH);

#ifdef CODEHAPPY_SDL
	// Play and render in SDL.
	Mix_Chunk* sdl_mixchunk(void);
	int play_wav(int loops = 0);
#endif

private:
	// Core mix method.
	void mix_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples);
};

/* Forward declarations */
struct Patch;
class Voices;

const int MIDI_CHANNEL_DRUMS = 9;
const int MIDI_PROGRAM_DRUMS = 128;
const int MIDI_PROGRAM_ALL = -1;

typedef std::vector<int>
	Channels;

class WavRender {
public:
	WavRender() { sr = 44100; nch = 2; }
	WavRender(u32 sample_rate, u32 nchannels) { sr = sample_rate; nch = nchannels; }

	// The render_* functions output a .WAV file object directly.
	WavFile* render_square(u32 msec, double freq, u32 amp = 0, u32 channel_flags = CHANNEL_BOTH) const;
	WavFile* render_sine(u32 msec, double freq, u32 amp = 0, u32 channel_flags = CHANNEL_BOTH) const;
	WavFile* render_saw(u32 msec, double freq, u32 amp = 0, u32 channel_flags = CHANNEL_BOTH) const;
	WavFile* render_sine_squared(u32 msec, double freq, u32 amp = 0, u32 channel_flags = CHANNEL_BOTH) const;
	WavFile* render_build(WavBuild* wb) const;
	WavFile* render_samples(u16* samples, u32 count) const;

	// The build_* functions allow building longer WAVs, melodies, etc. that can be rendered later. Each takes a
	// WavBuild* as a parameter -- if null, a new WavBuild is created, if existing, the build is appended to
	// the passed WavBuild. Return value is the new or appended WavBuild.
	// These use a stereo pan value from -100 to +100 -- negative values pan to left, positive values to right.
	WavBuild* build_square(WavBuild* wb, u32 msec, double freq, u32 amp = 0, int pan = 0) const;
	WavBuild* build_sine(WavBuild* wb, u32 msec, double freq, u32 amp = 0, int pan = 0) const;
	WavBuild* build_saw(WavBuild* wb, u32 msec, double freq, u32 amp = 0, int pan = 0) const;
	WavBuild* build_sine_squared(WavBuild* wb, u32 msec, double freq, u32 amp = 0, int pan = 0) const;
	WavBuild* build_white_noise(WavBuild* wb, u32 msec, u32 amp = 0, bool sep_channels = true) const;
	WavBuild* build_pink_noise(WavBuild* wb, u32 msec, u32 amp = 0, bool sep_channels = true) const;
	WavBuild* build_silence(WavBuild* wb, u32 msec) const;
	WavBuild* build_concatenate(WavBuild* wb1, WavBuild* wb2) const;
	WavBuild* build_samples(WavBuild* wb, u32 nsamples, u16* samples) const;
	WavBuild* build_note(WavBuild* wb, MusicalNote note, int octave, u32 msec, u32 amp = 0, NoteStyle ns = style_legato, Waveform wf = wave_sine, int pan = 0) const;

	// SoundFont rendering. If 'program' is non-negative, we will render only the specified program index,
	// else the entire MIDI will be rendered.
	WavBuild* build_midi(WavBuild* wb, tml_message* midi, tsf* soundfont, int program = MIDI_PROGRAM_ALL);
	WavBuild* build_midi(WavBuild* wb, const char* midi_path, tsf* soundfont, int program = MIDI_PROGRAM_ALL);
	WavBuild* build_midi(WavBuild* wb, const char* midi_path, const char* sf_path, int program = MIDI_PROGRAM_ALL);
	tsf* load_soundfont_for_render(const char* sf_path);

	// Mix wb2 into wb1. The mixing begins at (length wb1 - length wb2), so if wb1 is longer audio, wb2 will be heard at the end.
	void mix_from_end(WavBuild* wb1, WavBuild* wb2) const;
	void mix_from_end(WavBuild* wb1, WavBuild* wb2, u32 msec) const;
	// Mix wb2 into wb1 at msec milliseconds from the start of wb1.
	void mix_from_msec(WavBuild* wb1, WavBuild* wb2, u32 msec) const;
	// Mix wb2 into wb1 at the beginning.
	void mix_from_start(WavBuild* wb1, WavBuild* wb2) const;
	// Mix a vector of voices together into a new WavBuild.
	WavBuild* mix_voices(std::vector<WavBuild*>& wbs) const;

	// Music and melody functions.
#ifdef CODEHAPPY_SDL
	// Use SDL to play the voices together. (SDL audio must be initialized before calling.)
	// Loops: if 0, play once, if -1, loop indefinitely, if > 0, play (loops + 1) times.
	// Returns the audio channels used in ch; a call to quiet_channels() can halt playback.
	// Keep the Voices around until you're really done with them: if they're freed and SDL has the sample
	// buffers still, it won't be nice.
	void play_voices(Voices* wbs, Channels& ch, int loops) const;
	// Stop playback on the passed-in SDL audio channels.
	void quiet_channels(Channels& ch) const;
#endif

	/********************************************************************************

	Music!

	Pass in a melody string, and select a built-in instrument or waveform, and this will 
	render the melody as a new WavBuild. Mix with other WavBuilds for multi-voice mayhem!

	The waveform is used if and only if the instrument passed is nullptr.

	Regardless of whether this WavRender is stereo or mono, the returned WavBuild will
	always be monophonic (1 channel). This can still be mixed into or concatenated onto
	stereo WavBuilds with the same sampling rate.

	Sophisticated melodies and complex rhythms can be defined easily in the formatted
	melody string. It's partly compatible with the PLAY() command from GW-BASIC, if you 
	are familiar with that. Here is the melody string format explained:

	O	Specifies the default octave. Follow with a number. Octaves run from C to B.
		4 is the octave containing middle C.

	T	Tempo. Follow with an integer, which is the number of quarter notes per second.

	L	Specifies the length of the following note. Follow by any of:

		1 - whole note	2 - half note	4 - quarter note	8 - eighth note
		16 - 16th	32 - 32nd	64 - 64th		128 - 128th

		--> N.B.: In general, any other natural number "x" gives you a "1/x" note. If you 
			want a one-seventh note, make a one-seventh note! <--

		. - dotted note (3/2 normal duration)
		    Note that you can repeat dots to get 7/4, 15/8, 31/16, etc. duration.
		| - doubled note (twice normal duration)
		    You can combine double and dot to get a 3x length, etc. and with a non-power of
		    two length can represent many complex rhythms.
		p - triplet -- three of these notes take the same time as two non-triplet notes
		q - quintuplet -- five of these notes take the same time as four non-quintuplet notes
		s - septuplet -- seven of these notes take the same time as eight non-septuplet notes
		n - nonuplet -- nine of these notes take the same time as eight non-nonuplet notes

		You can combine to have dotted tripets, double-dotted quintuplets, etc.

	V	Specifies velocity (loudness) of the note. Can be followed by an integer
		from 0 to 32767, with 0 being inaudibly soft and 32767 being maximum loudness,
		or can be a symbolic velocity from the following table:

			fff, ff, f, mf, mp, p, pp, ppp

		fff is the loudest, ppp the softest; each softer velocity is about 80% the previous
		level.

	R	Rest for the current length.

	A-G	Specifies a note to play. The note can be followed by any of the following:

		# sharp (raises a half-step)		- flat (lowers a half-step)
		x double sharp (up a whole step)	/ double flat (down a whole step)
		: play stacatto				! sforzando (this note will be louder than default)
		+, [b]				synonyms for sharp, flat
		##, ++, --, [bb]		synonyms for double sharps & flats
		* fermata (note will be held longer)
		^ Play with slight emphasis (not quite sforzando)

		0-9	specifies an octave number; this overrides the default set by O
		_	Hold the note. By default, even with legato style, there will be some silence at the
			end of each rendered note. "_" indicates the note should be held the full time.

	<>	Instructions can be enclosed in angled brackets. Recognized instructions include:

		<leg>	Play style legato. This becomes the default style until changed.
		<sta>	Play style stacatto. This becomes the default style until changed.
		<piz>	Play style pizzicato. This becomes the default style until changed.
		<sf>	The current note is sforzando (this should come after the note.) Same as "!". 
		<pp>	Dynamic/velocity change. Can be "ppp", "pp", "p", "mp", "mf", "f", "ff", or "fff".
		<cres>	Crescendo: increase velocity gradually to the next dynamic change.
		<dec>	Decrescendo
			(<cres> and <dec> in practice are the same; whether velocity increases or decreases
			is determined by the current velocity and the new velocity.)

	Whitespace in the melody string is ignored; use liberally to make the text more readable.
	Unrecognized characters or instructions are also ignored: you can group measures in parentheses, for
	example.

	The melody string is case-insensitive.

	********************************************************************************/
	WavBuild* render_melody_str(const char* melody, Waveform wf, const Patch* p);

private:
	WavFile* alloc_init_wav(u32 msec) const;
	WavBuild* new_wav_build(void) const;
	WavBuild* ensure_wav_build(WavBuild* wb) const;
	u32 msec_to_nsamples(u32 msec) const;

	u32 sr;
	u32 nch;
};

class Voices {
public:

	// Construct with default WavRender.
	Voices() {}
	// Construct with passed-in WavRender.
	Voices(WavRender& wr);
	Voices(WavRender* wr);
	~Voices();

	void add_melody(const char* mstr, Waveform wf, const Patch* p);
	u32 nvoices(void) const;
	WavBuild* wb(u32 idx) const;
	void free(void);
#ifdef CODEHAPPY_SDL
	void play_voices(int loops = 0);
	void quiet_voices(void);
#endif

private:
	std::vector<WavBuild*> vox;
	WavRender render;
	Channels ch;
};

/*** Other exports: some functions related to MIDI rendering that may be useful. ***/
extern int midi_programs_used(tml_message* midi, std::unordered_set<int>& programs);
extern int midi_programs_used(const char* midi_path, std::unordered_set<int>& programs);

#ifdef CODEHAPPY_SDL
/* Render a MIDI file using the passed soundfont, and play it in the background. You can poll midi_playing() to
   determine when the music has finished. Returns 0 if OK, -1 if the sound font can't be loaded, -2 if a MIDI is already playing,
   or -3 if the MIDI file can't be loaded.

   If you pass nullptr for 'soundfont' or 'soundfont_filename', the last-used soundfont is used (will return MIDI_SOUNDFONT_ERROR if
   there isn't one.) */
extern int play_midi(const char* midi_filename, const char* soundfont_filename);
extern int play_midi(const char* midi_filename, tsf* soundfont);
extern bool midi_playing();

#define MIDI_OK		0
#define MIDI_SOUNDFONT_ERROR	(-1)
#define MIDI_ALREADY_PLAYING	(-2)
#define MIDI_MIDI_ERROR	(-3)
#define MIDI_SDL_ERROR		(-4)

#endif  // CODEHAPPY_SDL

#endif  // __WAV_RENDER__
/* end wavrender.h */
