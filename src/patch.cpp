/***

	patch.cpp

	Wavetable-based instrument patches, for musical synthesis. Sort of a 
	stripped-down Gravis Ultrasound. Includes many built-in instruments.

	With the library's precision mixing, muxing/demuxing and resampling
	capabilities, these can produce quality MIDI/retro-style music or SFX.

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"
#include <math.h>
#undef DUMP

#ifdef DUMP
#include <fstream>
#endif

/*** Helper functions. ***/
u32 dfreq_to_u32freq(double freq_hz) {
	freq_hz *= 1024.;
	return (u32)floor(freq_hz + 0.5);
}

double u32freq_to_dfreq(u32 freq) {
	return double(freq) / 1024.0;
}

static void amp_modify(s16* data, u32 ns, int max_amp) {
	s16* datae = data + ns;

	while (data < datae) {
		int mod = (int)(*data);

		mod *= max_amp;
		mod /= 32768;
		mod = CLAMP(mod, -32768, 32767);
		*data = s16(mod);

		++data;
	}
}

static void amp_modifyu(u16* data, u32 ns, int max_amp) {
	u16* datae = data + ns;

	while (data < datae) {
		u16 val = *data;
		int mod;

		mod = (int)val;
		mod -= 32768;
		mod *= max_amp;
		mod /= 32768;
		mod += 32768;
		mod = CLAMP(mod, 0, 65535);
		*data = u16(mod);

		++data;
	}
}

static void s16_to_u16(s16* data, u32 len) {
	u16* cp = (u16*)data;
	s16* datae = data + len;

	while (data < datae) {
		// signed signal/unsigned signal
		int ss = (int)(*data);
		u16 us;

		ss += 32768;
		// ...this is buggy as hell... it should surely be 65535! But 32767 gives better results (much less noise/hiss) when mixed.
		// Maybe with a quality low-pass filter, the mixing code wouldn't have so much trouble? But we're discarding basically
		// 1 bit out of 16-bit data, and as these instruments are based on old GUS patches it's not like they're the highest
		// quality in the first place. Maybe OK for now?
		ss = CLAMP(ss, 0, 65535); // 32767); //65535);
		us = (u16)ss;
		*cp = us;

		++data;
		++cp;
	}
}

bool Patch::is_achromatic(void) const {
	NOT_NULL_OR_RETURN(samples, true);
	if (0 == nsamples || nullptr == samples)
		return true;
	for (u32 e = 0; e < nsamples; ++e) {
		if (samples[e]->freq_max - samples[e]->freq_min >= (100UL << 10))
			return false;
	}
	return true;
}

bool Patch::freq_in_range(double freq) const {
	u32 ifreq = dfreq_to_u32freq(freq);
	return freq_in_range(ifreq);
}

bool Patch::freq_in_range(u32 ifreq) const {
	for (u32 e = 0; e < nsamples; ++e) {
		if (ifreq >= samples[e]->freq_min && ifreq <= samples[e]->freq_max)
			return true;
	}
	return false;
}

const WavSample* Patch::sample_for_pitch(u32 ifreq) const {
	const WavSample* ps = nullptr;
	u32 e;
	if (0 == nsamples)
		return nullptr;
	for (e = 0; e < nsamples; ++e) {
		if (ifreq >= samples[e]->freq_min && ifreq <= samples[e]->freq_max) {
			ps = samples[e];
			break;
		}
	}
	if (is_null(ps)) {
		u32 err = 999999999;
		u32 idx = 0;
		for (e = 0; e < nsamples; ++e) {
			if (abs((s32)samples[e]->freq_min - (s32)ifreq) < err) {
				idx = e;
				err = abs((s32)samples[e]->freq_min - (s32)ifreq);
			}
			if (abs((s32)samples[e]->freq_max - (s32)ifreq) < err) {
				idx = e;
				err = abs((s32)samples[e]->freq_max - (s32)ifreq);
			}	
		}
		ps = samples[idx];
	}

	return ps;
}

const WavSample* Patch::sample_for_pitch(double freq) const {
	return sample_for_pitch(dfreq_to_u32freq(freq));
}

WavBuild* Patch::render_core(WavBuild* wb, u32 ns, u32 ns_on, u32 new_rate, u32 new_freq, u32 new_amp, bool ignore_end) const {
	WavRender wr(new_rate, 1);
	const WavSample* ps = sample_for_pitch(new_freq);
	NOT_NULL_OR_RETURN(ps, wb);	
	/* If we're an achromatic instrument, use the frequency of the sample. */
	if (is_achromatic()) {
		new_freq = ps->freq_sample;
	}
	u64 cv_n = (u64)ps->sample_hz * (u64)new_freq, cv_d = (u64)new_rate * (u64)ps->freq_sample;
	u32 ns_end = (u32)((u64)ns - (((u64)ps->length - (u64)ps->loop_end()) * cv_d) / cv_n);
	u32 loop_len = (u32)(((u64)ps->loop_len * cv_d) / cv_n);
	u32 loop_start = (u32)(((u64)ps->loop_start * cv_d) / cv_n);	
	u32 decay_len = (u32)(((u64)ps->decay_samples * cv_d) / cv_n);
	u32 cloop = 0;
	s16* data = new s16 [ns];
	s16* w = data;
	NOT_NULL_OR_RETURN(data, nullptr);

	if (ns_on > ns)
		ns_on = ns;
	if (ignore_end || ((ps->length - ps->loop_end()) * cv_d) / cv_n > ns)
		ns_end = ns;

#ifdef DUMP
	std::ofstream dump;
	dump.open("dump.csv");
	dump << "cv_n," << cv_n << std::endl;
	dump << "cv_d," << cv_d << std::endl;
	dump << "ns_end," << ns_end << std::endl;
	dump << "loop_len," << loop_len << std::endl;
	dump << "loop_start," << loop_start << std::endl;
	dump << "loop_end," << ps->loop_end() << std::endl;
	dump << "decay_len," << decay_len << std::endl;
#endif

	for (u32 e = 0; e < ns; ++e) {
		u32 decay_n, decay_d;
		s64 val;
		u64 i_sam, m_sam;

		if (e < loop_start) {
			i_sam = (e * cv_n) / cv_d;
			m_sam = (e * cv_n) % cv_d;
		} else if (e >= ns_end) {
			i_sam = ps->loop_end() + ((e - ns_end) * cv_n) / cv_d;
			m_sam = ((e - ns_end) * cv_n) % cv_d;
		} else {
			// We're in the loop.
			i_sam = ps->loop_start + (cloop * cv_n) / cv_d;
			m_sam = (cloop * cv_n) % cv_d;
			++cloop;
			if (cloop >= loop_len)
				cloop = 0;
		}

		if (i_sam >= ps->length)
			i_sam = ps->length - 1;
		if (m_sam == 0 || i_sam + 1 == ps->length) {
			val = (s64)ps->data[i_sam];
		} else {
			// Linear interpolation between two samples
			s64 add;
			val = (s64)ps->data[i_sam];
			add = ((s64)ps->data[i_sam + 1] - (s64)ps->data[i_sam]);
			add *= (s64)m_sam;
			add /= (s64)cv_d;
			val += add;
		}

#ifdef DUMP
		s16 dump_v;
		if (i_sam + 1 == ps->length)
			dump_v = ps->data[i_sam];
		else
			dump_v = ps->data[i_sam + 1];
		dump << "sample " << e << "," << i_sam << "," << m_sam << "," << ps->data[i_sam] << "," << dump_v << "," << val;
#endif

		if (e >= ns_on) {
			if (e >= ns_on + decay_len) {
				decay_n = 0;
				decay_d = 1;
			} else {
				decay_d = decay_len + 1;
				decay_n = decay_d - (e - ns_on);
			}
		} else {
			decay_n = 1;
			decay_d = 1;
		}

		val *= decay_n;
		val /= decay_d;
		val = CLAMP(val, -32768, 32767);

#ifdef DUMP
		dump << ",decay_and_clamp," << val << std::endl;
#endif

		*w = (s16)val;
		++w;
	}

	// Adjust amplitude if required. Do it while we're signed, it's faster.
	if (new_amp > 0)
		amp_modify(data, ns, new_amp);
	// Convert signed samples to unsigned (as expected by WavRender.)
	s16_to_u16(data, ns);

	wb = wr.build_samples(wb, ns, (u16 *)data);
	delete [] data;

#ifdef DUMP
	dump.close();
	dump.clear();
#endif

	return wb;
}

WavBuild* Patch::render_msec(WavBuild* wb, u32 msec_note_on, u32 msec_note, u32 new_rate, double freq, u32 new_amp) const {
	u32 new_freq = dfreq_to_u32freq(freq);
	u32 ns = (new_rate * msec_note) / 1000;
	u32 ns_on = (new_rate * msec_note_on) / 1000;

	wb = render_core(wb, ns, ns_on, new_rate, new_freq, new_amp, false);
	return wb;
}

WavBuild* Patch::render_msec_full_decay(WavBuild* wb, u32 msec_note_on, u32 msec_note, u32 new_rate, double freq, u32 new_amp) const {
	u32 new_freq = dfreq_to_u32freq(freq);
	const WavSample* ps = sample_for_pitch(new_freq);
	NOT_NULL_OR_RETURN(ps, wb); 
	u32 msec_len = msec_note_on + (ps->decay_samples * 1000) / (u32)ps->sample_hz + 1;
	if (msec_len < msec_note)
		msec_len = msec_note;
	u32 ns = (new_rate * msec_len) / 1000;
	u32 ns_on = (new_rate * msec_note_on) / 1000;

	wb = render_core(wb, ns, ns_on, new_rate, new_freq, new_amp, false);
	return wb;
}


/* end patch.cpp */