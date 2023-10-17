/***

	mp3.cpp

	MP3 player using minimp3 and SDL audio layer.

***/
#define CODEHAPPY_NATIVE_SDL
#include <libcodehappy.h>

void samples_to_u16(i16* buf, u32 nsamples) {
	u16* bufo = (u16 *)buf;
	for (u32 e = 0; e < nsamples; ++e) {
		int val = (int)(*buf);
		val += 32768;
		*bufo = (u16)val;
		++buf;
		++bufo;
	}
}

int app_main() {
	if (app_argc() < 2) {
		std::cout << "Usage: mp3 [filename]\n";
		return 1;
	}

	mp3dec_t mp3;
	mp3dec_file_info_t info;
	if (mp3dec_load(&mp3, app_argv(1), &info, nullptr, nullptr)) {
		std::cout << "Unable to decode file '" << app_argv(1) << "' as an .MP3!\n";
		return 2;
	}
	std::cout << "MP3 loaded: " << info.hz << " Hz samples rate, " << info.channels << " channels, " << info.samples << " total samples.\n";
	if (info.channels < 1) {
		std::cout << "Mixed stereo/monophonic sound not supported!\n";
		return 3;
	}

	WavRender wr(info.hz, info.channels);
	samples_to_u16((i16*)info.buffer, (u32)info.samples);
	WavFile* wf = wr.render_samples((u16*)info.buffer, (u32)(info.samples / info.channels));
	Mix_OpenAudio(44100, AUDIO_S16LSB, 2, 1024);
	Mix_Init(MIX_INIT_OGG);
	wf->play_wav(0);
	while (Mix_Playing(-1) > 0)
		no_op;
	delete wf;

	return 0;
}

/* end mp3.cpp */