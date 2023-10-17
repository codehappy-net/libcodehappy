/***

	testwav.cpp

	A short test program for .WAV rendering support in libcodehappy.

	Copyright (c) 2022, Chris Street.

***/
#include "libcodehappy.h"

int main(void) {
	WavRender wv;
	WavFile* wf, *concat[2];

	wf = wv.render_sine(2000, 440);
	wf->out_to_file("440.wav");
	delete wf;

	wf = wv.render_sine(2000, 440, 0, CHANNEL_RIGHT);
	wf->out_to_file("440r.wav");
	delete wf;

	wf = wv.render_sine(2000, 440, 0, CHANNEL_LEFT);
	wf->out_to_file("440l.wav");
	delete wf;

	wf = wv.render_square(2000, 440);
	wf->out_to_file("440sq.wav");
	delete wf;

	wf = wv.render_saw(2000, 440);
	wf->out_to_file("440sw.wav");
	delete wf;

	concat[0] = wv.render_sine(1000, 440);
	concat[1] = wv.render_sine(1000, 220);
	wf = concat[0]->concatenate(concat[1]);
	delete concat[0];
	delete concat[1];
	concat[0] = wf;
	concat[1] = wv.render_square(1000, 440);
	wf = concat[0]->concatenate(concat[1]);
	delete concat[0];
	delete concat[1];
	wf->out_to_file("concat.wav");
	delete wf;

	return(0);
}
