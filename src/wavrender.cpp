/***

	wavrender.cpp

	Render WAV files, to use as music/sound effects, etc. Supports 16-bit PCM format
	with 1 or 2 channels and any valid sample rate. Also includes various methods used
	for musical synthesis. Melodies can be played from MIDI-like strings of note events.
	Includes software mixing, resampling, muxing and demuxing capabilities.

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"

#undef MIXDUMP

#ifdef MIXDUMP
#include <fstream>
#endif

/* some important positions in the .WAV header, and other constants */
const u32 WAV_OFFS_FILELEN = 4;
const u32 WAV_OFFS_NUM_CHANNELS = 22;
const u32 WAV_OFFS_SAMPLE_RATE = 24;
const u32 WAV_OFFS_DATALEN = 40;
const u32 WAV_OFFS_WAVDATA = 44;
const u32 WAV_HEADER_LEN = 44;
const u16 NEUTRAL_SIGNAL = 32768;
const u16 DEFAULT_AMPLITUDE = AMPLITUDE_DEFAULT;
const u32 AMPLITUDE_FFF = 32000;
const u32 AMPLITUDE_FF = 29000;
const u32 AMPLITUDE_F = 25000;
const u32 AMPLITUDE_MF = 22000;
const u32 AMPLITUDE_MP = 19000;
const u32 AMPLITUDE_P = 16000;
const u32 AMPLITUDE_PP = 12000;
const u32 AMPLITUDE_PPP = 8000;

/* Write the basic .WAV header, right up to the start of the data chunk, for our samples */
static void basic_wave_hdr(u8* dest, u32 nchannels, u32 sample_rate) {
	*dest++ = 'R';
	*dest++ = 'I';
	*dest++ = 'F';
	*dest++ = 'F';
	*((u32 *)dest) = 0;	// File size (minus 8)
	dest += sizeof(u32);
	*dest++ = 'W';
	*dest++ = 'A';
	*dest++ = 'V';
	*dest++ = 'E';
	*dest++ = 'f';
	*dest++ = 'm';
	*dest++ = 't';
	*dest++ = ' ';
	*((u32 *)dest) = CPU_TO_LE32(16);
	dest += sizeof(u32);
	*((u16 *)dest) = CPU_TO_LE16(1);
	dest += sizeof(u16);
	*((u16 *)dest) = CPU_TO_LE16((u16)nchannels);
	dest += sizeof(u16);
	*((u32 *)dest) = CPU_TO_LE32(sample_rate);
	dest += sizeof(u32);
	*((u32 *)dest) = CPU_TO_LE32((nchannels * sample_rate) * 2);	// 16 bits per sample
	dest += sizeof(u32);
	*((u16 *)dest) = CPU_TO_LE16((u16)nchannels * 2);		// 16 bits per channel
	dest += sizeof(u16);
	*((u16 *)dest) = CPU_TO_LE16(16);				// 16 bits per sample
	dest += sizeof(u16);
	*dest++ = 'd';
	*dest++ = 'a';
	*dest++ = 't';
	*dest++ = 'a';
	*((u32 *)dest) = 0;	// Data size
}

/* Calculate the full number of samples required for the given time, sample rate & number of channels. */
static u32 wav_samples_required(u32 msec, u32 sample_rate, u32 nchannels) {
	u64 ret = sample_rate;
	ret *= msec;
	ret *= nchannels;
	ret /= 1000;
	return (u32)ret;
}

/* Returns the number of data bytes required for the sample. */
static u32 wav_data_bytes(u32 msec, u32 sample_rate, u32 nchannels) {
	// 16 bits each
	return wav_samples_required(msec, sample_rate, nchannels) * 2;
}

/* Returns the total file/allocated size of the WAV file. */
static u32 wav_size(u32 msec, u32 sample_rate, u32 nchannels) {
	return wav_data_bytes(msec, sample_rate, nchannels) + WAV_HEADER_LEN;
}

/* Given a signed amplitude, convert it into a 16-bit unsigned signal required by the PCM .WAV format. */
static u16 wav_signal16(double amp) {
	// first, clamp the amp
	int iamp = (int)floor(amp + 0.5);
	iamp = CLAMP(iamp, -32768, 32767);
	// then, bring it into u16 range.
	iamp += 32768;
	return (u16)iamp;
}

/* Give a random unsigned 16-bit integer. */
static u16 randu16(void) {
	static u64 rand_bits = RandU64();
	static int used = 0;
	u16 ret = (u16)(rand_bits & 0xFFFF);
	rand_bits >>= 16;
	used++;
	if (4 == used) {
		rand_bits = RandU64();
		used = 0;
	}
	return ret;
}

/* Random u16 amplitude up to the value v_max. */
static u16 randu16_maxamp(u16 v_max) {
	u16 ret;
	v_max = CLAMP(v_max, 1000, 32767);
	do {
		ret = randu16();
	} until (ret <= NEUTRAL_SIGNAL + v_max && ret >= NEUTRAL_SIGNAL - v_max);
	return ret;
}

/* Adjust the amplitude of a PCM sample. The scale value is a percentage. */
static u16 amp_adjust_u16(u16 sample, int pct) {
	int val = (int)sample;
	val -= 32768;
	val *= pct;
	val /= 100;
	val = CLAMP(val, -32768, 32767);
	val += 32768;
	return (u16)val;
}

static int __maxamp(u16* samples, u32 nsamples) {
	u16* c = samples, * ce = samples + nsamples;
	int ma = 0;

	while (c < ce) {
		int v;
		v = (int)(*c);
		v -= 32768;
		v = abs(v);
		if (v > ma)
			ma = v;
		++c;
	}

	return ma;
}

static int __maxampadd(u16* s1, u16* s2, u32 ns1, u32 ns2) {
	u32 ns = std::min(ns1, ns2);
	u16* c1, *c2, *ce;
	int ma = 0;

	c1 = s1;
	c2 = s2;
	ce = s1 + ns;
	while (c1 < ce) {
		int v1, v2;

		v1 = (int)(*c1);
		v2 = (int)(*c2);
		v1 -= 32768;
		v2 -= 32768;
		v1 += v2;
		v1 = abs(v1);
		if (v1 > ma)
			ma = v1;

		++c1;
		++c2;
	}

	return ma;
}

#ifdef MIXDUMP
static bool mixdump = false;
static std::ofstream mixo;
#endif

/* Mix two PCM samples. */
static u16 __mixin(u16 mix, u16 add) {
#ifdef MIXDUMP
	if (!mixdump) {
		mixo.open("mixdump.csv");
		mixo << "S1,S2,Mix,MixCast\n";
		mixdump = true;
	}
#endif

	i32 mixs;
	u32 s1 = mix, s2 = add;
	if (s1 < 32768 || s2 < 32768)
		mixs = (int)((s1 * s2) / 32768UL);
	else
		mixs = 2 * (s1 + s2) - (i32)(((u64)s1 * (u64)s2) / 32768ULL) - 65536;
	mixs = CLAMP(mixs, 0, 65535);

#ifdef MIXDUMP
	mixo << mix << "," << add << "," << mixs << "," << ((u16)mixs) << "\n";
#endif
	return ((u16)mixs);
}

/* Older mix code, with an amplitude adjustment. */
static u16 __mixin_ampadj(u16 mix, u16 add, int target_amp, int max_amp_add) {
	i32 mixs = (i32)mix;
	i32 adds = (i32)add;

	mixs -= 32768;
	adds -= 32768;

	if (sign_function(mixs) == sign_function(adds)) {
		if (mixs < 0)
			mixs = (mixs + adds) + ((mixs * adds) / 32768);
		else
			mixs = (mixs + adds) - ((mixs * adds) / 32768);
	} else {
		mixs = (mixs + adds);
	}

	if (target_amp < max_amp_add) {
		i64 scale = mixs;
		scale *= target_amp;
		scale /= max_amp_add;
		mixs = (int)scale;
	}

	mixs += 32768;
	mixs = CLAMP(mixs, 0, 65535);
	return ((u16)mixs);
}

double frequency_note(MusicalNote note, int octave) {
	// frequencies of notes C4 - B4
	const double note_freq[] =
		{ 261.63, 277.18, 293.66, 311.13, 329.63, 349.23, 369.99, 392.00, 415.30, 440.00, 466.16, 493.88 };
	double fq = note_freq[note];
	while (octave < 4) {
		++octave;
		fq /= 2.0;
	}
	while (octave > 4) {
		--octave;
		fq *= 2.0;
	}
	return(fq);
} 

u32 msec_beat(u32 tempo_bpm) {
	return 60000 / tempo_bpm;
}

/* Render various .WAV files. */
WavFile* WavRender::render_square(u32 msec, double freq, u32 amp, u32 channel_flags) const {
	WavFile* ret = alloc_init_wav(msec);
	u16* data = (u16 *)ret->data_chunk_start();
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;

	u32 nsamples = wav_samples_required(msec, sr, 1);
	double count = 0.;
	const double srd = (double)sr, ampd = (double)amp;
	for (int e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		signal = wav_signal16(v);
		if (signal >= 32768)
			signal = 32768 + (u16)amp;
		else
			signal = 32768 - (u16)amp;
		if (1 == nch || (channel_flags & CHANNEL_LEFT) != 0) {
			*data++ = CPU_TO_LE16(signal);
		} else {
			*data++ = NEUTRAL_SIGNAL;
		}
		// If we're in stereo, output the second sample.
		if (2 == nch) {
			if ((channel_flags & CHANNEL_RIGHT) != 0) {
				*data++ = CPU_TO_LE16(signal);
			} else {
				*data++ = NEUTRAL_SIGNAL;
			}
		}
		count += freq;
	}

	return ret;
}

WavFile* WavRender::render_sine(u32 msec, double freq, u32 amp, u32 channel_flags) const {
	WavFile* ret = alloc_init_wav(msec);
	u16* data = (u16 *)ret->data_chunk_start();
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;

	u32 nsamples = wav_samples_required(msec, sr, 1);
	double count = 0.;
	const double srd = (double)sr, ampd = (double)amp;
	for (int e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch || (channel_flags & CHANNEL_LEFT) != 0) {
			*data++ = CPU_TO_LE16(signal);
		} else {
			*data++ = NEUTRAL_SIGNAL;
		}
		// If we're in stereo, output the second sample.
		if (2 == nch) {
			if ((channel_flags & CHANNEL_RIGHT) != 0) {
				*data++ = CPU_TO_LE16(signal);
			} else {
				*data++ = NEUTRAL_SIGNAL;
			}
		}
		count += freq;
	}

	return ret;
}

WavFile* WavRender::render_sine_squared(u32 msec, double freq, u32 amp, u32 channel_flags) const {
	WavFile* ret = alloc_init_wav(msec);
	u16* data = (u16 *)ret->data_chunk_start();
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;

	u32 nsamples = wav_samples_required(msec, sr, 1);
	double count = 0.;
	const double srd = (double)sr, ampd = (double)amp;
	for (int e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		if (v > 0.)
			v = v * v;
		else
			v = -(v * v);
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch || (channel_flags & CHANNEL_LEFT) != 0) {
			*data++ = CPU_TO_LE16(signal);
		} else {
			*data++ = NEUTRAL_SIGNAL;
		}
		// If we're in stereo, output the second sample.
		if (2 == nch) {
			if ((channel_flags & CHANNEL_RIGHT) != 0) {
				*data++ = CPU_TO_LE16(signal);
			} else {
				*data++ = NEUTRAL_SIGNAL;
			}
		}
		count += freq;
	}

	return ret;
}

WavFile* WavRender::render_saw(u32 msec, double freq, u32 amp, u32 channel_flags) const {
	WavFile* ret = alloc_init_wav(msec);
	u16* data = (u16 *)ret->data_chunk_start();
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;

	u32 nsamples = wav_samples_required(msec, sr, 1);
	double count = 0.;
	const double srd = (double)sr, ampd = (double)amp;
	for (int e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count / srd;
		v -= floor(v);
		v -= 0.5;
		v *= 2.0;
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch || (channel_flags & CHANNEL_LEFT) != 0) {
			*data++ = CPU_TO_LE16(signal);
		} else {
			*data++ = NEUTRAL_SIGNAL;
		}
		// If we're in stereo, output the second sample.
		if (2 == nch) {
			if ((channel_flags & CHANNEL_RIGHT) != 0) {
				*data++ = CPU_TO_LE16(signal);
			} else {
				*data++ = NEUTRAL_SIGNAL;
			}
		}
		count += freq;
	}

	return ret;
}

WavFile* WavRender::alloc_init_wav(u32 msec) const {
	WavFile* ret = new WavFile;
	u32 sz = wav_size(msec, sr, nch);
	ret->len = sz;
	ret->data = new u8 [sz];
	basic_wave_hdr(ret->data, nch, sr);
	*((u32 *)(ret->data + WAV_OFFS_FILELEN)) = CPU_TO_LE32(sz - 8);
	*((u32 *)(ret->data + WAV_OFFS_DATALEN)) = CPU_TO_LE32(wav_data_bytes(msec, sr, nch));
	return ret;
}

WavFile* WavRender::render_build(WavBuild* wb) const {
	wb = ensure_wav_build(wb);
	return wb->render();
}

WavFile* WavRender::render_samples(u16* samples, u32 count) const {
	WavFile* ret = new WavFile;
	u32 sz = count * nch * 2 + WAV_HEADER_LEN;

	ret->len = sz;
	ret->data = new u8 [sz];
	basic_wave_hdr(ret->data, nch, sr);

	memcpy(ret->data + WAV_HEADER_LEN, samples, count * sizeof(u16));

	*((u32 *)(ret->data + WAV_OFFS_FILELEN)) = CPU_TO_LE32(sz - 8);
	*((u32 *)(ret->data + WAV_OFFS_DATALEN)) = CPU_TO_LE32(count * nch * 2);

	return ret;
}

WavBuild* WavRender::build_square(WavBuild* wb, u32 msec, double freq, u32 amp, int pan) const {
	wb = ensure_wav_build(wb);
	wb->square_wave(msec, freq, amp, pan);
	return wb;
}

WavBuild* WavRender::build_sine(WavBuild* wb, u32 msec, double freq, u32 amp, int pan) const {
	wb = ensure_wav_build(wb);
	wb->sine_wave(msec_to_nsamples(msec), freq, amp, pan);
	return wb;
}

WavBuild* WavRender::build_saw(WavBuild* wb, u32 msec, double freq, u32 amp, int pan) const {
	wb = ensure_wav_build(wb);
	wb->saw_wave(msec_to_nsamples(msec), freq, amp, pan);
	return wb;
}

WavBuild* WavRender::build_sine_squared(WavBuild* wb, u32 msec, double freq, u32 amp, int pan) const {
	wb = ensure_wav_build(wb);
	wb->sine_sq_wave(msec_to_nsamples(msec), freq, amp, pan);
	return wb;
}

WavBuild* WavRender::build_white_noise(WavBuild* wb, u32 msec, u32 amp, bool sep_channels) const {
	wb = ensure_wav_build(wb);
	wb->white_noise(msec_to_nsamples(msec), amp, sep_channels);
	return wb;
}

WavBuild* WavRender::build_pink_noise(WavBuild* wb, u32 msec, u32 amp, bool sep_channels) const {
	wb = ensure_wav_build(wb);
	wb->pink_noise(msec_to_nsamples(msec), amp, sep_channels);
	return wb;
}

WavBuild* WavRender::build_silence(WavBuild* wb, u32 msec) const {
	wb = ensure_wav_build(wb);
	wb->silence(msec_to_nsamples(msec));
	return wb;
}

WavBuild* WavRender::build_concatenate(WavBuild* wb1, WavBuild* wb2) const {
	WavBuild* wb = ensure_wav_build(wb1);
	wb->concatenate(wb2);
	return wb;
}

WavBuild* WavRender::build_samples(WavBuild* wb, u32 nsamples, u16* samples) const {
	wb = ensure_wav_build(wb);
	wb->samples(nsamples, samples);
	return wb;
}

void WavRender::mix_from_end(WavBuild* wb1, WavBuild* wb2) const {
	wb1->mix_from_end(wb2, wb2->num_samples());
	return;
}

void WavRender::mix_from_end(WavBuild* wb1, WavBuild* wb2, u32 msec) const {
	wb1->mix_from_end(wb2, msec_to_nsamples(msec));
	return;
}

void WavRender::mix_from_msec(WavBuild* wb1, WavBuild* wb2, u32 msec) const {
	wb1->mix_from_start(wb2, msec_to_nsamples(msec));
	return;
}

WavBuild* WavRender::build_note(WavBuild* wb, MusicalNote note, int octave, u32 msec, u32 amp, NoteStyle ns, Waveform wf, int pan) const {
	wb = ensure_wav_build(wb);
	u32 msec_note, msec_silence;
	switch (ns) {
	case style_pizzicato:
		msec_note = std::min((u32)100UL, msec / 8);
		break;
	case style_stacatto:
		msec_note = msec / 5;
		break;
	case style_legato:
		msec_note = (msec * 4) / 5;
		break;
	case style_legato_full:
		msec_note = msec;
		break;
	case style_fermata:
		msec_note = (msec * 5) / 4;
		break;
	}
	if (msec_note >= msec)
		msec_silence = 0;
	else
		msec_silence = msec - msec_note;

	double freq = frequency_note(note, octave);

	switch (wf) {
	case wave_sine:
		wb->sine_wave(msec_to_nsamples(msec_note), freq, amp, pan);
		wb->silence(msec_silence);
		break;
	case wave_square:
		wb->square_wave(msec_to_nsamples(msec_note), freq, amp, pan);
		wb->silence(msec_silence);
		break;
	case wave_saw:
		wb->saw_wave(msec_to_nsamples(msec_note), freq, amp, pan);
		wb->silence(msec_silence);
		break;
	case wave_sine_sq:
		wb->sine_sq_wave(msec_to_nsamples(msec_note), freq, amp, pan);
		wb->silence(msec_silence);
		break;
	default:
		break;
	}

	return wb;
}


WavBuild* WavRender::new_wav_build(void) const {
	WavBuild* ret = new WavBuild;
	u8 hdr[WAV_HEADER_LEN];
	basic_wave_hdr(hdr, nch, sr);
	NOT_NULL_OR_RETURN(ret, nullptr);
	ret->sp.memcat(hdr, WAV_HEADER_LEN);
	ret->ensure_header();
	return ret;
}

WavBuild* WavRender::ensure_wav_build(WavBuild* wb) const {
	if (is_null(wb))
		return new_wav_build();
	if (wb->sp.length() == 0) {
		// This can happen with user-allocated WavBuild objects, or previously-rendered WavBuilds.
		// Rebuild the header in this case.
		u8 hdr[WAV_HEADER_LEN];
		basic_wave_hdr(hdr, nch, sr);
		wb->sp.memcat(hdr, WAV_HEADER_LEN);
		wb->ensure_header();
	}
	return wb;
}

u32 WavRender::msec_to_nsamples(u32 msec) const {
	u64 ret = (u64)msec * (u64)sr;
	ret /= 1000ULL;
	return (u32)ret;
}

WavFile::WavFile() {
	data = nullptr;
	len = 0;
}

WavFile::~WavFile() {
	if (nullptr != data)
		delete [] data;
	data = nullptr;
	len = 0;
}

void WavFile::out_to_file(const char* pname) const {
	FILE* f;
	f = fopen(pname, "wb");
	fwrite(data, sizeof(u8), len, f);
	fclose(f);
}

u8* WavFile::data_chunk_start(void) const {
	NOT_NULL_OR_RETURN(data, nullptr);
	return data + WAV_OFFS_WAVDATA;
}

u16 WavFile::num_channels(void) const {
	NOT_NULL_OR_RETURN(data, 0);
	return *(u16*)(data + WAV_OFFS_NUM_CHANNELS);
}

u32 WavFile::sample_rate(void) const {
	NOT_NULL_OR_RETURN(data, 0);
	return *(u32*)(data + WAV_OFFS_SAMPLE_RATE);
}

/* Concatenate two .WAV files. Requires that the sample rate and number of channels be identical. Returns nullptr on error. */
WavFile* WavFile::concatenate(WavFile* wf) {
	if (num_channels() != wf->num_channels())
		return nullptr;
	if (sample_rate() != wf->sample_rate())
		return nullptr;

	u32 new_sz = len + wf->len - WAV_HEADER_LEN;
	u8* new_data = new u8 [new_sz];
	NOT_NULL_OR_RETURN(new_data, nullptr);

	memcpy(new_data, data, len);
	memcpy(new_data + len, wf->data_chunk_start(), wf->len - WAV_HEADER_LEN);
	*((u32 *)(new_data + WAV_OFFS_FILELEN)) = CPU_TO_LE32(new_sz - 8);
	*((u32 *)(new_data + WAV_OFFS_DATALEN)) = CPU_TO_LE32(new_sz - WAV_HEADER_LEN);

	WavFile* ret = new WavFile;
	if (is_null(ret)) {
		delete [] new_data;
		return nullptr;
	}
	ret->len = new_sz;
	ret->data = new_data;
	return(ret);
}

void WavFile::load_from_file(const char* path) {
	RamFile rf(path, RAMFILE_READONLY);
	len = rf.length();
	data = rf.relinquish_buffer();
}

#ifdef CODEHAPPY_SDL
Mix_Chunk* WavFile::sdl_mixchunk(void) {
	SDL_RWops* rw;
	Mix_Chunk* mxc;

	rw = SDL_RWFromConstMem(data, len);
	mxc = Mix_LoadWAV_RW(rw, 0);

	return mxc;
}

void WavFile::play_wav(int loops) {
	Mix_Chunk* mxc = sdl_mixchunk();
	Mix_PlayChannel(-1, mxc, loops);
}
#endif  // CODEHAPPY_SDL

u16 WavBuild::num_channels(void) const {
	NOT_NULL_OR_RETURN(data(), 0);
	return *(u16*)(data() + WAV_OFFS_NUM_CHANNELS);
}

u32 WavBuild::sample_rate(void) const {
	NOT_NULL_OR_RETURN(data(), 0);
	return *(u32*)(data() + WAV_OFFS_SAMPLE_RATE);
}

void WavBuild::ensure_header(void) {
	*((u32 *)(data() + WAV_OFFS_FILELEN)) = CPU_TO_LE32(sp.length() - 8);
	*((u32 *)(data() + WAV_OFFS_DATALEN)) = CPU_TO_LE32(sp.length() - WAV_HEADER_LEN);
}

u32 WavBuild::num_samples(void) const {
	return (sp.length() - WAV_HEADER_LEN) / (2 * num_channels());
}

u32 WavBuild::len_msec(void) const {
	u64 ret = num_samples();
	ret *= 1000ULL;
	ret /= sample_rate();
	return (u32)ret;
}

/* NOTE: On success (returns non-NULL), after rendering, the WavBuild object will be empty. */	
WavFile* WavBuild::render(void) {
	WavFile* ret = new WavFile;
	NOT_NULL_OR_RETURN(ret, nullptr);
	ret->len = sp.length();
	ret->data = sp.relinquish_buffer();
	return(ret);
}

u8* WavBuild::data(void) const {
	return sp.buffer();
}

u8* WavBuild::data_chunk(void) const {
	if (0 == sp.length())
		return nullptr;
	return sp.buffer() + WAV_HEADER_LEN;
}

u16* WavBuild::sample_data(void) const {
	return (u16*)data_chunk();
}

u16* WavBuild::sample_pos(u32 nsample) const {
	return sample_data() + (nsample * num_channels());
}

u32 WavBuild::data_len(void) const {
	if (0 == sp.length())
		return 0;
	return sp.length() - WAV_HEADER_LEN;
}

void WavBuild::square_wave(u32 nsamples, double freq, u32 amp, int pan) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	double count = 0.;
	const double srd = (double)sample_rate(), ampd = (double)amp;
	const u16 nch = num_channels();
	pan = CLAMP(pan, -100, 100);
	for (u32 e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		signal = wav_signal16(v);
		if (signal >= 32768)
			signal = 32768 + (u16)amp;
		else
			signal = 32768 - (u16)amp;
		if (1 == nch) {
			// Monophonic sound; we can ignore the pan value.
			sp.addle_u16(signal);
		} else {
			// Handle stereo pan.
			if (0 == pan) {
				sp.addle_u16(signal);
				sp.addle_u16(signal);
			} else if (pan < 0) {
				sp.addle_u16(signal);
				sp.addle_u16(amp_adjust_u16(signal, 100 + pan));
			} else {
				sp.addle_u16(amp_adjust_u16(signal, 100 - pan));
				sp.addle_u16(signal);
			}
		}
		count += freq;
	}
	ensure_header();
}

void WavBuild::sine_wave(u32 nsamples, double freq, u32 amp, int pan) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	double count = 0.;
	const double srd = (double)sample_rate(), ampd = (double)amp;
	const u16 nch = num_channels();
	pan = CLAMP(pan, -100, 100);
	for (u32 e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch) {
			// Monophonic sound; we can ignore the pan value.
			sp.addle_u16(signal);
		} else {
			// Handle stereo pan.
			if (0 == pan) {
				sp.addle_u16(signal);
				sp.addle_u16(signal);
			} else if (pan < 0) {
				sp.addle_u16(signal);
				sp.addle_u16(amp_adjust_u16(signal, 100 + pan));
			} else {
				sp.addle_u16(amp_adjust_u16(signal, 100 - pan));
				sp.addle_u16(signal);
			}
		}
		count += freq;
	}
	ensure_header();
}

void WavBuild::saw_wave(u32 nsamples, double freq, u32 amp, int pan) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	double count = 0.;
	const double srd = (double)sample_rate(), ampd = (double)amp;
	const u16 nch = num_channels();
	pan = CLAMP(pan, -100, 100);
	for (u32 e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count / srd;
		v -= floor(v);
		v -= 0.5;
		v *= 2.0;
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch) {
			// Monophonic sound; we can ignore the pan value.
			sp.addle_u16(signal);
		} else {
			// Handle stereo pan.
			if (0 == pan) {
				sp.addle_u16(signal);
				sp.addle_u16(signal);
			} else if (pan < 0) {
				sp.addle_u16(signal);
				sp.addle_u16(amp_adjust_u16(signal, 100 + pan));
			} else {
				sp.addle_u16(amp_adjust_u16(signal, 100 - pan));
				sp.addle_u16(signal);
			}
		}
		count += freq;
	}
	ensure_header();
}

void WavBuild::sine_sq_wave(u32 nsamples, double freq, u32 amp, int pan) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	double count = 0.;
	const double srd = (double)sample_rate(), ampd = (double)amp;
	const u16 nch = num_channels();
	pan = CLAMP(pan, -100, 100);
	for (u32 e = 0; e < nsamples; ++e) {
		u16 signal;
		double v;
		v = count * 2. * M_PI / srd;
		v = sin(v);
		if (v > 0.)
			v = v * v;
		else
			v = -(v * v);
		v *= ampd;
		signal = wav_signal16(v);
		if (1 == nch) {
			// Monophonic sound; we can ignore the pan value.
			sp.addle_u16(signal);
		} else {
			// Handle stereo pan.
			if (0 == pan) {
				sp.addle_u16(signal);
				sp.addle_u16(signal);
			} else if (pan < 0) {
				sp.addle_u16(signal);
				sp.addle_u16(amp_adjust_u16(signal, 100 + pan));
			} else {
				sp.addle_u16(amp_adjust_u16(signal, 100 - pan));
				sp.addle_u16(signal);
			}
		}
		count += freq;
	}
	ensure_header();
}

void WavBuild::white_noise(u32 nsamples, u32 amp, bool sep_channels) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	const u16 nch = num_channels();
	for (u32 e = 0; e < nsamples; ++e) {
		u16 r = randu16_maxamp(amp);
		sp.addle_u16(r);
		if (2 == nch) {
			if (sep_channels) {
				r = randu16_maxamp(amp);
			}
			sp.addle_u16(r);
		}
	}
	ensure_header();
}

void WavBuild::pink_noise(u32 nsamples, u32 amp, bool sep_channels) {
	if (0 == amp)
		amp = DEFAULT_AMPLITUDE;
	ensure_header();
	// TODO: implement.
	assert(false);
}

void WavBuild::silence(u32 nsamples) {
	const u16 nch = num_channels();
	for (u32 e = 0; e < nsamples; ++e) {
		sp.addle_u16(NEUTRAL_SIGNAL);
		if (2 == nch) {
			sp.addle_u16(NEUTRAL_SIGNAL);
		}
	}
	ensure_header();
}

void WavBuild::samples(u32 nsamples, u16* samples) {
	sp.memcat((u8*)samples, nsamples * sizeof(u16));
	ensure_header();
}

bool WavBuild::concatenate(WavBuild* wb) {
	NOT_NULL_OR_RETURN(wb, true);
	if (num_channels() != wb->num_channels() || sample_rate() != wb->sample_rate())
		return false;
	sp.memcat(wb->data_chunk(), wb->data_len());
	ensure_header();
	return true;
}

void WavBuild::mix_from_end(WavBuild* wb2) {
	mix_from_end(wb2, wb2->num_samples());
}

void WavBuild::mix_from_end(WavBuild* wb2, u32 nsamples) {
	u32 ns = num_samples();
	if (nsamples > ns) {
		mix_from_pos(wb2, 0, nsamples - ns, ns);
		return;
	}
	mix_from_pos(wb2, ns - nsamples, 0, nsamples);
}

void WavBuild::mix_from_start(WavBuild* wb2, u32 nsamples) {
	u32 ns = num_samples();
	if (nsamples >= ns)
		return;
	mix_from_pos(wb2, nsamples, 0, wb2->num_samples());
}

void WavBuild::mix_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples) {
	NOT_NULL_OR_RETURN_VOID(wb2);
	const u16 nc = num_channels();
	if (sample_rate() != wb2->sample_rate())
		return;
	if (nc != wb2->num_channels()) {
		if (1 == nc)
			mix_stereo_in_from_pos(wb2, ns_start1, ns_start2, nsamples);
		else if (2 == nc)
			mix_mono_in_from_pos(wb2, ns_start1, ns_start2, nsamples);
		return;
	}
	// Data chunk & data chunk end.
	u16* dc, * dce;
	dc = (u16 *)data_chunk();
	dce = dc + (num_samples() * nc);

	// Copy chunk & copy chunk end.
	u16* cc, * cce;
	cc = (u16 *)wb2->data_chunk();
	cce = cc + (wb2->num_samples() * nc);
	cc = wb2->sample_pos(ns_start2);

#if 0
	// Amplitude checks -- find target amplitude and maximum (summed) amplitude.
	// You only need this if you're using __mixin_ampadj().
	int amp_t, amp_mx;
	amp_t = __maxamp(sample_pos(ns_start1), dce - sample_pos(ns_start1));
	amp_mx = __maxampadd(sample_pos(ns_start1), wb2->sample_pos(ns_start2), dce - sample_pos(ns_start1), cce - cc);
#endif

	// Do the mix.
	u16* p, * pe;
	p = sample_pos(ns_start1);
	pe = p + (nsamples * nc);
	while (p < pe && p < dce && cc < cce) {
		*p = __mixin(*p, *cc);
//		*p = __mixin_ampadj(*p, *cc, amp_t, amp_mx);
		++p;
		++cc;
		if (2 == nc) {
			*p = __mixin(*p, *cc);
//			*p = __mixin_ampadj(*p, *cc, amp_t, amp_mx);
			++p;
			++cc;
		}
	}
}

void WavBuild::mix_mono_in_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples, int pan) {
	// Sanity checks.
	NOT_NULL_OR_RETURN_VOID(wb2);
	if (sample_rate() != wb2->sample_rate())
		return;
	if (num_channels() != 2 || wb2->num_channels() != 1)
		return;

	// Data chunk & data chunk end.
	u16* dc, * dce;
	dc = (u16 *)data_chunk();
	dce = dc + (num_samples() * 2);

	// Copy chunk & copy chunk end.
	u16* cc, * cce;
	cc = (u16 *)wb2->data_chunk();
	cce = cc + wb2->num_samples();
	cc = wb2->sample_pos(ns_start2);

	// Do the mix.
	u16* p, * pe;
	p = sample_pos(ns_start1);
	pe = p + (nsamples * 2);
	while (p < pe && p < dce && cc < cce) {
		u16 mono_signal = *cc;
		if (0 == pan) {
			*p = __mixin(*p, mono_signal);
			++p;
			*p = __mixin(*p, mono_signal);
		} else if (pan < 0) {
			*p = __mixin(*p, mono_signal);
			++p;
			*p = __mixin(*p, amp_adjust_u16(mono_signal, 100 + pan));
		} else {
			*p = __mixin(*p, amp_adjust_u16(mono_signal, 100 - pan));
			++p;
			*p = __mixin(*p, mono_signal);
		}
		++p;
		++cc;
	}
}

void WavBuild::mix_stereo_in_from_pos(WavBuild* wb2, u32 ns_start1, u32 ns_start2, u32 nsamples, u32 channel_flags) {
	// Sanity checks.
	NOT_NULL_OR_RETURN_VOID(wb2);
	if (sample_rate() != wb2->sample_rate())
		return;
	if (num_channels() != 1 || wb2->num_channels() != 2)
		return;

	// Data chunk & data chunk end.
	u16* dc, * dce;
	dc = (u16 *)data_chunk();
	dce = dc + num_samples();

	// Copy chunk & copy chunk end.
	u16* cc, * cce;
	cc = (u16 *)wb2->data_chunk();
	cce = cc + wb2->num_samples() * 2;
	cc = wb2->sample_pos(ns_start2);

	// Do the mix.
	u32 u;
	u16* p, * pe;
	p = sample_pos(ns_start1);
	pe = p + nsamples;
	while (p < pe && p < dce && cc < cce) {
		switch (channel_flags) {
		case CHANNEL_LEFT:
			*p = __mixin(*p, *cc);
			++cc;
			++cc;
			break;
		case CHANNEL_RIGHT:
			++cc;
			*p = __mixin(*p, *cc);
			++cc;
			break;
		case CHANNEL_BOTH:
		default:
			u = u32(*cc);
			++cc;
			u += u32(*cc);
			u >>= 1;
			++cc;
			*p = __mixin(*p, u16(u));
			break;
		}
		++p;
	}
}

WavBuild* WavBuild::copy(void) {
	WavBuild* ret = new WavBuild;
	NOT_NULL_OR_RETURN(ret, ret);

	ret->sp.memcat(sp.buffer(), sp.length());

	return ret;
}

u16* WavBuild::channel_sample_data(u32 channel_flag) const {
	u16* ret = new u16 [num_samples()];
	assert(channel_flag == CHANNEL_LEFT || channel_flag == CHANNEL_RIGHT);
	NOT_NULL_OR_RETURN(ret, ret);
	if (num_channels() == 1) {
		// We're mono already.
		memcpy(ret, sample_data(), num_samples() * sizeof(u16));
		return ret;
	}

	u16* data = sample_data();
	u16* datae = data + (num_samples() * num_channels());
	u16* out = ret;
	while (data < datae) {
		if (channel_flag == CHANNEL_LEFT) {
			*out = *data;
			++out;
		}
		++data;
		if (channel_flag == CHANNEL_RIGHT) {
			*out = *data;
			++out;
		}
		++data;
	}
	ship_assert(out == (ret + num_samples()));

	return ret;
}

static float __u16tof(u16 s) {
	float ret = (float)s;
	ret /= 65535.0f;
	return ret;
}

float* WavBuild::channel_sample_data_f(u32 channel_flag) const {
	float* ret = new float [num_samples()];
	float* out = ret;
	assert(channel_flag == CHANNEL_LEFT || channel_flag == CHANNEL_RIGHT);
	NOT_NULL_OR_RETURN(ret, ret);
	if (num_channels() == 1) {
		// We're mono already.
		for (u32 e = 0; e < num_samples(); ++e) {
			*out = __u16tof(sample_data()[e]);
			++out;
		}
		return ret;
	}

	u16* data = sample_data();
	u16* datae = data + (num_samples() * num_channels());
	while (data < datae) {
		if (channel_flag == CHANNEL_LEFT) {
			*out = __u16tof(*data);
			++out;
		}
		++data;
		if (channel_flag == CHANNEL_RIGHT) {
			*out = __u16tof(*data);
			++out;
		}
		++data;
	}
	ship_assert(out == (ret + num_samples()));

	return ret;
}

#ifdef CODEHAPPY_SDL

Mix_Chunk* WavBuild::sdl_mixchunk(void) {
	SDL_RWops* rw;
	Mix_Chunk* mxc;

	rw = SDL_RWFromConstMem(sp.buffer(), sp.length());
	mxc = Mix_LoadWAV_RW(rw, 0);

	return mxc;
}

void WavBuild::play_wav(int loops) {
	Mix_Chunk* mxc = sdl_mixchunk();
	Mix_PlayChannel(-1, mxc, loops);
}

#endif

struct NoteData {
	NoteData();
	void reset(void);
	void reset(int oct, int recip, int bpm, NoteStyle style, u32 amp_);

	int key;
	int transpose;
	int octave;
	int nrecip;
	int ndots;
	NoteStyle ns;
	int tuplet;
	int tempo;
	bool doub;
	bool emphasis;
	bool sforzando;
	u32 amp;
	bool valid;
};

NoteData::NoteData() {
	reset();
}

void NoteData::reset(void) {
	key = 'C';
	transpose = 0;
	octave = 4;
	nrecip = 4;
	ndots = 0;
	ns = style_legato;
	valid = false;
	tempo = 120;
	tuplet = 0;
	doub = false;
	sforzando = false;
	emphasis = false;
	amp = DEFAULT_AMPLITUDE;
}

void NoteData::reset(int oct, int recip, int bpm, NoteStyle style, u32 amp_) {
	reset();
	octave = oct;
	nrecip = recip;
	tempo = bpm;	
	ns = style;
	amp = amp_;
}

static void render_note_wb(WavBuild* wb, const NoteData& nd, Waveform wf, const Patch* p) {
	u64 ns_note, ns_silence;
	u32 spm = wb->sample_rate() * 60UL;
	u32 b_n = 4, b_d = nd.nrecip * nd.tempo;
	u32 amp = nd.amp;

	/* Determine the number of samples to sound the note, and the number of samples of silence after. */
	if (nd.ndots > 0) {
		b_n *= (2 << nd.ndots) - 1;
		b_d *= (1 << nd.ndots);
	}
	if (nd.doub) {
		b_n <<= 1;
	}
	switch (nd.tuplet) {
	default:
		break;
	case 3:
		b_n <<= 1;
		b_d *= 3;
		break;
	case 5:
		b_n <<= 2;
		b_d *= 5;
		break;
	case 7:
		b_n <<= 3;
		b_d *= 7;
		break;
	case 9:
		b_n <<= 3;
		b_d *= 9;
		break;
	}

	/* Adjust the amplitude for emphases and sforzandoes. */
	if (nd.emphasis) {
		amp *= 5;
		amp >>= 2;
	}
	if (nd.sforzando) {
		amp *= 3;
		amp >>= 1;
	}
	if (amp > AMPLITUDE_MAX)
		amp = AMPLITUDE_MAX;

	/* Calculate the number of samples for both sounding the note and silence. */
	ns_note = (u64)spm * b_n;
	ns_note /= b_d;
	ns_silence = ns_note;

	if (nd.key == 'R') {
		// It's a rest; we just have to render silence.
		wb->silence((u32)ns_silence);
		return;
	}

	switch (nd.ns) {
	default:
	case style_legato:
		ns_note <<= 2;
		ns_note /= 5;
		ns_silence -= ns_note;
		break;
	case style_legato_full:
		ns_silence = 0;
		break;
	case style_pizzicato:
		ns_note >>= 3;
		ns_silence -= ns_note;
		break;
	case style_stacatto:
		ns_note /= 5;
		ns_silence -= ns_note;
		break;
	case style_fermata:
		ns_note <<= 2;
		ns_note /= 5;
		ns_silence -= ns_note;
		ns_note *= 25;
		ns_note >>= 4;
		break;
	}

	/* Calculate the frequency of the note. */
	const int note[7] = { (int)note_A, (int)note_B, (int)note_C, (int)note_D, (int)note_E, (int)note_F, (int)note_G };
	int nidx = nd.key - 'A';
	int octave = nd.octave;

	if (nidx < 0 || nidx > 6)
		return;	// invalid note
	nidx = note[nidx];
	nidx += nd.transpose;
	while (nidx < 0) {
		nidx += 12;
		--octave;
	}
	while (nidx >= 12) {
		nidx -= 12;
		++octave;
	}
	double freq = frequency_note((MusicalNote)nidx, octave);

	if (p != nullptr && p->is_achromatic()) {
		/* Special case: for achromatic patches, just render the waveform. */
		u64 ns_total = ns_silence + ns_note;
		const WavSample* ws = p->sample_for_pitch(freq);
		ns_note = ws->length;
		ns_note *= wb->sample_rate();
		ns_note /= ws->sample_hz;
		if (ns_note > ns_total)
			ns_note = ns_total;
		ns_silence = ns_total - ns_note;
	}

	/* And render into the WavBuild. */
	if (nullptr == p) {
		switch (wf) {
		case wave_sine:
		default:
			wb->sine_wave(ns_note, freq, amp, 0);
			wb->silence(ns_silence);
			break;
		case wave_square:
			wb->square_wave(ns_note, freq, amp, 0);
			wb->silence(ns_silence);
			break;
		case wave_saw:
			wb->saw_wave(ns_note, freq, amp, 0);
			wb->silence(ns_silence);
			break;
		case wave_sine_sq:
			wb->sine_sq_wave(ns_note, freq, amp, 0);
			wb->silence(ns_silence);
			break;
		}
	} else {
		u32 ns_on;
		ns_on = ns_note;
		ns_on <<= 2;
		ns_on /= 5;
		p->render_core(wb, ns_note, ns_on, wb->sample_rate(), dfreq_to_u32freq(freq), amp, false);
		wb->silence(ns_silence);
	}
}

WavBuild* WavRender::render_melody_str(const char* melody, Waveform wf, const Patch* p) {
	// Defaults
	int octave = 4;
	int recip = 4;
	u32 bpm = 120;
	u32 amp = DEFAULT_AMPLITUDE;
	NoteStyle style = style_legato;

	// Create a mono WavBuild with our sampling rate.
	WavBuild* wb = new WavBuild;
	u8 hdr[WAV_HEADER_LEN];
	basic_wave_hdr(hdr, 1, sr);
	NOT_NULL_OR_RETURN(wb, nullptr);
	wb->sp.memcat(hdr, WAV_HEADER_LEN);
	wb->ensure_header();

	// Parse the melody string into notes.
	std::vector<NoteData> notes;
	NoteData nd;
	const char* w = melody;
	forever {
		if (!(*w)) {
			if (nd.valid)
				notes.push_back(nd);
			break;
		}

		switch (toupper(*w)) {
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'G':
			// Rests are a special case: they're treated as notes here.
		case 'R':
			if (nd.valid) {
				notes.push_back(nd);
				nd.reset(octave, recip, bpm, style, amp);
			}
			nd.key = toupper(*w);
			nd.valid = true;
			++w;
			break;

		case 'O':
			++w;
			octave = atoi(w);
			while (*w == '-' || isdigit(*w))
				++w;
			break;

		case '.':
			nd.ndots++;
			++w;
			break;

		case '|':
			nd.doub = true;
			++w;
			break;

		case 'P':
			++w;
			nd.tuplet = 3;
			break;

		case 'Q':
			nd.tuplet = 5;
			++w;
			break;

		case 'S':
			nd.tuplet = 7;
			++w;
			break;

		case 'N':
			nd.tuplet = 9;
			++w;
			break;

		case '_':
			++w;
			nd.ns = style_legato_full;
			break;

		case '*':
			++w;
			nd.ns = style_fermata;
			break;

		case 'T':
			++w;
			bpm = (u32)atoi(w);
			while (*w && isdigit(*w))
				++w;
			break;

		case 'L':
			++w;
			recip = atoi(w);
			if (recip < 1)
				recip = 4;
			while (*w == '-' || isdigit(*w))
				++w;
			break;

		case '#':
		case '+':
			++w;
			++nd.transpose;
			break;

		case '-':
			++w;
			--nd.transpose;
			break;

		case '^':
			nd.emphasis = true;
			++w;
			break;

		case '!':
			nd.sforzando = true;
			++w;
			break;

		case 'X':
			nd.transpose += 2;
			++w;
			break;

		case '/':
			nd.transpose -= 2;
			++w;
			break;

		case ':':
			nd.ns = style_stacatto;
			++w;
			break;

		case '<':
			if (!strncmp(w, "<leg>", 5)) {
				style = style_legato;
				w += 5;
				break;
			}
			if (!strncmp(w, "<sta>", 5)) {
				style = style_stacatto;
				w += 5;
				break;
			}
			if (!strncmp(w, "<piz>", 5)) {
				style = style_pizzicato;
				w += 5;
				break;
			}
			if (!strncmp(w, "<sf>", 4)) {
				nd.sforzando = true;
				w += 4;
				break;
			}
			if (!strncmp(w, "<fff>", 5)) {
				w += 2;
				goto LFFF;
			}
			if (!strncmp(w, "<ff>", 4)) {
				w += 2;
				goto LFF;
			}
			if (!strncmp(w, "<f>", 3)) {
				w += 2;
				goto LF;
			}
			if (!strncmp(w, "<mf>", 4)) {
				w += 2;
				goto LMF;
			}
			if (!strncmp(w, "<mp>", 4)) {
				w += 2;
				goto LMP;
			}
			if (!strncmp(w, "<p>", 3)) {
				w += 2;
				goto LP;
			}
			if (!strncmp(w, "<pp>", 4)) {
				w += 2;
				goto LPP;
			}
			if (!strncmp(w, "<ppp>", 5)) {
				w += 2;
				goto LPPP;
			}
			// Unrecognized instruction: skip it.
			while (*w && *w != '>')
				++w;
			break;

		case 'V':
			++w;
			amp = (u32)atoi(w);
			if (0UL == amp) {
				amp = DEFAULT_AMPLITUDE;
				if (!strncmp(w, "fff", 3)) {
LFFF:					amp = AMPLITUDE_FFF;
					w += 3;
					break;
				}
				if (!strncmp(w, "ff", 2)) {
LFF:					amp = AMPLITUDE_FF;
					w += 2;
					break;
				}
				if (*w == 'f') {
LF:					amp = AMPLITUDE_F;
					++w;
					break;
				}
				if (!strncmp(w, "mf", 2)) {
LMF:					amp = AMPLITUDE_MF;
					w += 2;
					break;
				}
				if (!strncmp(w, "mp", 2)) {
LMP:					amp = AMPLITUDE_MP;
					w += 2;
					break;
				}
				if (!strncmp(w, "ppp", 3)) {
LPPP:					amp = AMPLITUDE_PPP;
					w += 3;
					break;
				}
				if (!strncmp(w, "pp", 2)) {
LPP:					amp = AMPLITUDE_PP;
					w += 2;
					break;
				}
				if (*w == 'p') {
LP:					amp = AMPLITUDE_P;
					++w;
					break;
				}
			} else {
				while (*w && isdigit(*w))
					++w;
			}
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			nd.octave = atoi(w);
			while (isdigit(*w) && *w)
				++w;
			break;

		default:
			++w;
			break;
		}
	}

	// Now render the notes in order into the WavBuild object.
	for (int e = 0; e < notes.size(); ++e) {
		render_note_wb(wb, notes[e], wf, p);
	}

	return wb;
}

void WavRender::mix_from_start(WavBuild* wb1, WavBuild* wb2) const {
	wb1->mix_from_start(wb2, 0);
}

bool wb_comp(const WavBuild* wb1, const WavBuild* wb2) {
	return wb1->num_samples() > wb2->num_samples();
}

static void mix_amp_rational_adj(WavBuild* wb1, WavBuild* wb2, u32 size) {
	const int n[] = { 1, 1, 2896, 2365, 2048, 1832, 1672, 1548, 1448, 1365, 1295 };
	const int d[] = { 1, 1, 4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096, 4096 };
	u32 idx = size - 1;
	if (idx > 10)
		idx = 10;

	if (wb1->sample_rate() != wb2->sample_rate())
		return;
	if (wb1->num_channels() != wb2->num_channels())
		return;

	// Data chunk & data chunk end.
	u16* dc, * dce;
	dc = (u16 *)wb1->data_chunk();
	dce = dc + (wb1->num_samples() * wb1->num_channels());

	// Copy chunk & copy chunk end.
	u16* cc, * cce;
	cc = (u16 *)wb2->data_chunk();
	cce = cc + (wb2->num_samples() * wb2->num_channels());

	// Do the mix.
	while (dc < dce && cc < cce) {
		int v = (int)(*cc);
		v -= 32768;
		v *= n[idx];
		v /= d[idx];
		v += 32768;
		v = CLAMP(v, 0, 65535);
		*dc = __mixin(*dc, (u16)v);
		++dc;
		++cc;
		if (2 == wb1->num_channels()) {
			v = (int)(*cc);
			v -= 32768;
			v *= n[idx];
			v /= d[idx];
			v += 32768;
			v = CLAMP(v, 0, 65535);
			*dc = __mixin(*dc, (u16)v);
			++dc;
			++cc;
		}
	}
}

WavBuild* WavRender::mix_voices(std::vector<WavBuild*>& wbs) const {
	// Sanity checks
	if (wbs.empty())
		return nullptr;
	for (u32 e = 1; e < wbs.size(); ++e) {
		if (wbs[0]->num_channels() != wbs[e]->num_channels())
			return nullptr;
		if (wbs[0]->sample_rate() != wbs[e]->sample_rate())
			return nullptr;
	}

	// First, sort the voices by number of samples.
	std::sort(wbs.begin(), wbs.end(), wb_comp);

	WavBuild* wb = wbs[0]->copy();
	i32* mixd;
	u32 ns = wb->num_samples() * wb->num_channels();
	mixd = new i32 [ns];
	NOT_NULL_OR_RETURN(mixd, wb);
	u16* d = wb->sample_data(), * de = d + ns;
	i32* c = mixd;

#if 0
	// Try the original c_library algorithm here.
	while (d < de) {
		*c = (i32)(*d) - 32768;
		++d;
		++c;
	}
	for (u32 e = 1; e < wbs.size(); ++e) {
		u32 nsc = wbs[e]->num_samples() * wbs[e]->num_channels();
		u16* cc = wbs[e]->sample_data(), * cce = cc + nsc;
		c = mixd;
		while (cc < cce) {
			i32 mixs = *c;
			i32 adds = (i32)(*cc) - 32768;
			if (sign_function(mixs) == sign_function(adds)) {
				if (mixs < 0)
					mixs = (mixs + adds) + ((mixs * adds) / 32768);
				else
					mixs = (mixs + adds) - ((mixs * adds) / 32768);
			} else {
				mixs = (mixs + adds);
			}
			*c = mixs;

			++cc;
			++c;
		}
	}

	i32* ce = mixd + ns;
	c = mixd;
	d = wb->sample_data();
	while (c < ce) {
		i32 mixs = *c;
		mixs += 32768;
		mixs = CLAMP(mixs, 0, 65535);
		*d = (u16)(mixs);
		++c;
		++d;
	}
#endif

#if 1
	while (d < de) {
		*c = (i32)(*d);
		++d;
		++c;
	}

	// Use the __mixin() algorithm, but with less clipping.
	for (u32 e = 1; e < wbs.size(); ++e) {
		u32 nsc = wbs[e]->num_samples() * wbs[e]->num_channels();
		u16* cc = wbs[e]->sample_data(), * cce = cc + nsc;
		c = mixd;
		while (cc < cce) {
			i32 mixs;
			i32 s1 = *c, s2 = (i32)(*cc);
			if (s1 < 0) s1 = 0;
			if (s2 < 0) s2 = 0;
			if (s1 < 32768 || s2 < 32768)
				mixs = ((s1 * s2) / 32768UL);
			else
				mixs = 2 * (s1 + s2) - (i32)(((u64)s1 * (u64)s2) / 32768ULL) - 65536;
			*c = mixs;
			++cc;
			++c;
		}
	}

	i32* ce = mixd + ns;
	c = mixd;
	d = wb->sample_data();
	while (c < ce) {
		i32 mixs = *c;
		mixs = CLAMP(mixs, 0, 65535);
		*d = (u16)(mixs);
		++c;
		++d;
	}
#endif

#if 0
	// Now we're going to do a straight additive mix of all samples, in 32 bits to avoid clipping as much as possible.
	while (d < de) {
		*c = (i32)(*d) - 32768;
		++d;
		++c;
	}

	for (u32 e = 1; e < wbs.size(); ++e) {
		u32 nsc = wbs[e]->num_samples() * wbs[e]->num_channels();
		u16* cc = wbs[e]->sample_data(), * cce = cc + nsc;
		c = mixd;
		while (cc < cce) {
			*c += ((i32)(*cc) - 32768);
			++cc;
			++c;
		}
	}

	// The amplitude adjustments below work, but make the mix too quiet.

	// Determine the maximum amplitude, and if more than 1 in 128 samples is beyond dynamic range, scale
	// the maximum amplitude down.
	u32 mx_a = 0, c_clip = 0;
	i32* ce = mixd + ns;
	c = mixd;
	while (c < ce) {
		if (abs(*c) > mx_a)
			mx_a = abs(*c);
		if (*c < -32768 || *c > 32767)
			++c_clip;
		++c;
	}
	c_clip <<= 7;
	if (c_clip > ns) {
		i64 calc;
		c = mixd;
		while (c < ce) {
			calc = (i64)(*c);
			calc *= 32768;
			calc /= mx_a;
			*c = (i32)calc;
			++c;
		}
	}

	// Convert back to u16.
	d = wb->sample_data();
	c = mixd;
	while (d < de) {
		i32 val = *c;
		val += 32768;
		val = CLAMP(val, 0, 65535);
		*d = (u16)val;
		++c;
		++d;
	}
#endif

	delete [] mixd;

	return wb;	
}

Voices::Voices(WavRender& wr) {
	render = wr;
}

Voices::Voices(WavRender* wr) {
	render = *wr;
}

Voices::~Voices() {
	free();
}

void Voices::add_melody(const char* mstr, Waveform wf, const Patch* p) {
	WavBuild* wb = render.render_melody_str(mstr, wf, p);
	vox.push_back(wb);
}

u32 Voices::nvoices(void) const {
	return vox.size();
}

WavBuild* Voices::wb(u32 idx) const {
	if (idx >= nvoices())
		return nullptr;
	return vox[idx];
}

void Voices::free(void) {
	for (u32 e = 0; e < vox.size(); ++e)
		delete vox[e];
	vox.clear();
	ch.clear();
}

#ifdef CODEHAPPY_SDL

void Voices::play_voices(int loops) {
	render.play_voices(this, ch, loops);
}

void Voices::quiet_voices(void) {
	render.quiet_channels(ch);
}

void WavRender::play_voices(Voices* wbs, Channels& ch, int loops) const {
	SDL_RWops* rw[256];
	Mix_Chunk* mxc[256];
	u32 nv = wbs->nvoices();
	if (nv > 256)
		return;
	u32 e;

	for (e = 0; e < nv; ++e) {
		WavBuild* wb = wbs->wb(e);
		rw[e] = SDL_RWFromConstMem(wb->sp.buffer(), wb->sp.length());
	}
	for (e = 0; e < nv; ++e) {
		mxc[e] = Mix_LoadWAV_RW(rw[e], 0);
	}
	for (e = 0; e < nv; ++e) {
		ch.push_back(Mix_PlayChannel(-1, mxc[e], loops));
	}
}

void WavRender::quiet_channels(Channels& ch) const {
	u32 nc = ch.size();
	for (e = 0; e < nc; ++e) {
		if (ch[e] < 0)
			continue;
		Mix_HaltChannel(ch[e]);
	}
}

#endif  // CODEHAPPY_SDL

struct MidiRender {
	tsf*		soundfont;
	tml_message*	msg;
	u8*		stream;
	int		len;
	u32		nch;
	double		msec;
	double		sr;
	int		program;
	std::unordered_map<int, int> channel_to_prog;
};

static bool __render_midi(MidiRender& mr) {
	const u32 RENDER_SAMPLES_ITERATION = 64;
	int sample_block, sample_count = (mr.len / (mr.nch * sizeof(short)));
	u8* stream = mr.stream;

	for (sample_block = RENDER_SAMPLES_ITERATION; sample_count; sample_count -= sample_block, stream += (sample_block * mr.nch * sizeof(short))) {
		sample_block = std::min(sample_block, sample_count);

		/* Execute all MIDI messages up to time msec */
		for (mr.msec += sample_block * (1000.0 / mr.sr); mr.msg && mr.msec >= mr.msg->time; mr.msg = mr.msg->next) {
			switch (mr.msg->type) {
				case TML_PROGRAM_CHANGE:
					mr.channel_to_prog[mr.msg->channel] = mr.msg->program;
					tsf_channel_set_presetnumber(mr.soundfont, mr.msg->channel, mr.msg->program, (mr.msg->channel == MIDI_CHANNEL_DRUMS));
					break;

				case TML_NOTE_ON:
					if (mr.program >= 0 && mr.channel_to_prog[mr.msg->channel] != mr.program)
						break;
					tsf_channel_note_on(mr.soundfont, mr.msg->channel, mr.msg->key, mr.msg->velocity / 127.0f);
					break;

				case TML_NOTE_OFF:
					if (mr.program >= 0 && mr.channel_to_prog[mr.msg->channel] != mr.program)
						break;
					tsf_channel_note_off(mr.soundfont, mr.msg->channel, mr.msg->key);
					break;

				case TML_PITCH_BEND:
					tsf_channel_set_pitchwheel(mr.soundfont, mr.msg->channel, mr.msg->pitch_bend);
					break;

				case TML_CONTROL_CHANGE:
					tsf_channel_midi_control(mr.soundfont, mr.msg->channel, mr.msg->control, mr.msg->control_value);
					break;
			}
		}

		tsf_render_short(mr.soundfont, (short*)stream, sample_block, 0);
	}

	return (mr.msg != nullptr);
}

WavBuild* WavRender::build_midi(WavBuild* wb, tml_message* midi, tsf* soundfont, int program) {
	MidiRender mr;
	wb = ensure_wav_build(wb);

	mr.soundfont = soundfont;
	mr.msg = midi;
	mr.len = 16384;
	mr.stream = new u8 [mr.len];
	NOT_NULL_OR_RETURN(mr.stream, nullptr);
	mr.nch = nch;
	mr.msec = 0.0;
	mr.sr = (double)sr;
	mr.program = program;

	do {
		__render_midi(mr);
		wb->sp.memcat(mr.stream, mr.len);
	} while (mr.msg != nullptr);
	wb->ensure_header();

	delete [] mr.stream;
	tsf_reset(soundfont);

	return wb;
}

WavBuild* WavRender::build_midi(WavBuild* wb, const char* midi_path, const char* sf_path, int program) {
	tml_message* midi;
	tsf* soundfont;
	WavBuild* ret;
	midi = tml_load_filename(midi_path);
	NOT_NULL_OR_RETURN(midi, nullptr);
	soundfont = load_soundfont_for_render(sf_path);
	if (is_null(soundfont)) {
		tml_free(midi);
		return nullptr;
	}
	ret = build_midi(wb, midi, soundfont, program);
	tsf_close(soundfont);
	tml_free(midi);
	return ret;
}

WavBuild* WavRender::build_midi(WavBuild* wb, const char* midi_path, tsf* soundfont, int program) {
	tml_message* midi;
	midi = tml_load_filename(midi_path);
	NOT_NULL_OR_RETURN(midi, nullptr);
	WavBuild* ret = build_midi(wb, midi, soundfont, program);
	tml_free(midi);
	return ret;
}

tsf* WavRender::load_soundfont_for_render(const char* sf_path) {
	tsf* soundfont = tsf_load_filename(sf_path);
	tsf_channel_set_bank_preset(soundfont, MIDI_CHANNEL_DRUMS, 128, 0);
	tsf_set_output(soundfont, (nch == 1 ? TSF_MONO : TSF_STEREO_INTERLEAVED), sr, 0.0f);
	return soundfont;
}

int midi_programs_used(tml_message* midi, std::unordered_set<int>& programs) {
	programs.clear();
	while (midi != nullptr) {
		if (midi->type == TML_PROGRAM_CHANGE) {
			programs.insert(midi->program);
		}
		midi = midi->next;
	}
	return programs.size();
}

int midi_programs_used(const char* midi_path, std::unordered_set<int>& programs) {
	tml_message* midi;
	programs.clear();
	midi = tml_load_filename(midi_path);
	NOT_NULL_OR_RETURN(midi, 0);
	int ret = midi_programs_used(midi, programs);
	tml_free(midi);
	return ret;
}

/* end wavrender.cpp */
