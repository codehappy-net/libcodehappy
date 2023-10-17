/***

	patch.h

	Wavetable-based instrument patches, for musical synthesis. Sort of a 
	stripped-down Gravis Ultrasound. Includes many built-in instruments.

	With the library's precision mixing, muxing/demuxing and resampling
	capabilities, these can produce quality MIDI/retro-style music or SFX.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __PATCH_H__
#define __PATCH_H__

/* Wavetable instrument parameters for a specific frequency range, and the PCM samples (always signed 16-bit monophonic.) */
struct WavSample {
	u32 length;		// Length of the waveform, in samples
	u32 sample_hz;		// The number of samples per second
	u32 loop_start;		// Beginning and length of the looped portion of the sample
	u32 loop_len;
	u32 freq_min;		// Minimum (lowest) and maximum (highest) pitches recommended for this sample.
	u32 freq_max;		// For achromatic instruments, these numbers will be the same or very close.	
	u32 freq_sample;	// The fundamental frequency of the sample. Note that these three frequencies are in 1/1024 Hz units.
	u32 decay_samples;	// Number of samples after the note is released required to decay fully.
	const s16* data;	// The waveform sample data.

	u32 loop_end(void) const { return loop_start + loop_len; }
};

/* An instrument patch. */
struct Patch {
	// Each instrument patch contains one or more WavSamples, describing the waveform to generate for specific frequency ranges.
	u32 nsamples;
	const WavSample** samples;

	// Does this waveform represent an instrument producing varied pitches, or is it achromatic (e.g., a tambourine)?
	bool is_achromatic() const;

	// Is the passed in frequency in range for this instrument? (You can synthesize sounds of any pitch with any Patch,
	// though achromatic patches will always use the fundamental frequency of the original sample to further reduce noise.)
	bool freq_in_range(double freq) const;
	bool freq_in_range(u32 ifreq) const;

	// Returns the best-match sample for the specified frequency. 
	const WavSample* sample_for_pitch(u32 ifreq) const;
	const WavSample* sample_for_pitch(double freq) const;

	/***
		Render a note using this instrument patch.

		wb: The WavBuild object we're building with. Can be nullptr; a new one will be created and returned.
		ns: The total count of samples (using the passed-in sampling rate) to output.
		ns_on: The number of samples, from the beginning, that the note is considered "on" (actively being produced.)
			Notes do not decay until after this time; some instruments should use low values for ns_on.
		new_rate: The sampling rate to render at, in number of samples per second.
		new_freq: The frequency (in 1/1024 Hz units) at which to render the note.
		new_amp: The maximum amplitude (from 1 to 32767) permitted in the output. 20000-28000 are typical values. Note
			that the amplitude adjustment is done at the end of render, and the loudest part of the output will
			have amplitude approximately equal to this value. If amp == 0, no amplitude adjustment is made.
		ignore_end: If true, we ignore any part of the sample beyond the looping portion.

		Resampling and frequency shifts are done with 64-bit arithmetic and linear interpolation. The output
		is monophonic 16-bit *unsigned* PCM and can be mixed or concatenated onto WavRender/WavBuild with any stereo pan.
	***/
	WavBuild* render_core(WavBuild* wb, u32 ns, u32 ns_on, u32 new_rate, u32 new_freq, u32 new_amp, bool ignore_end) const;

	/* Render a note that is on and sounding for the specified number of milliseconds with the specified frequency & amplitude.
		Decay will occur according to patch parameters after msec_note_on has elapsed. */
	WavBuild* render_msec(WavBuild* wb, u32 msec_note_on, u32 msec_note, u32 new_rate, double freq, u32 new_amp) const;

	/* Render a note that is on and sounding for the specified number of milliseconds, but extend the output until the note 
		has fully decayed. */
	WavBuild* render_msec_full_decay(WavBuild* wb, u32 msec_note_on, u32 msec_note, u32 new_rate, double freq, u32 new_amp) const;
};

/* The built-in instrument patches. */
#include "patches.i"

/* Convert between double frequencies (in units of Hz) and u32 frequencies (in units of 1/1024 Hz). */
extern u32 dfreq_to_u32freq(double freq_hz);
extern double u32freq_to_dfreq(u32 freq);

#endif  // __PATCH_H__
/* end patch.h */