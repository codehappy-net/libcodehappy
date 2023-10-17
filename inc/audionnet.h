/***

	audionnet.h

	Neural network that predicts audio samples based on windows of past audio data.

	Input layout (one audio channel at a time, can predict stereo audio by feeding 
	left/right channels separately):

	X inputs	the last X individual samples
	X inputs	X sliding averages of 2 samples before that
	X inputs	X sliding averages of 4 samples before that
	X inputs	X sliding averages of 8...
	X inputs	...16...
	X inputs	...32...
		(etc., a total of G windows)

	Result: GX total inputs, drawn from (2^G - 1)X past inputs.
	X = 100, G = 10 means a total lookback of 2.32 seconds at 44100 Hz
	Should predict on the order of X outputs.

	Copyright (c) 2022 Chris Street. 

***/
#ifndef __AUDIONNET__
#define __AUDIONNET__

struct NNetThreadContext {
	NNetThreadContext();
	~NNetThreadContext();
	u32 ith;
	u32 nth;
	float* in;
	float* out;
	float** batch_in;
	float** batch_out;
};

const u32 AUDIONNET_SAMPLE_RATE = 44100;

class AudioNNet {
public:
	/* Creates an AudioNNet with X inputs in each of G windows (see banner comment at top). */
	AudioNNet(u32 X, u32 G);

	/* As above, but specify the number of hidden layers and neurons per layer. */
	AudioNNet(u32 X, u32 G, u32 hidden_layers, u32 neurons);

	/* Construct the AudioNNet from file. */
	AudioNNet(const char* pathname);

	/* Accessors. */
	u32 samples_per_window(void) const { return spw; }
	u32 num_windows(void) const { return windows; }
	u32 max_threads(void) const { return maxth; }
	void set_max_threads(u32 m) { maxth = m; }

	/* Persist to disk. */
	void update_on_disk(void);
	void out_to_ramfile(RamFile* rf);

	/* Read from disk. */
	void read_from_file(const char* path);
	void read_from_ramfile(RamFile* rf);

	/* Train from audio input (or a file containing audio input). */
	void train_from_file(const char* path);
	void train_from_audio(WavBuild* wb);

	/* Predict from audio input (or a file containing audio input). */
	WavBuild* predict_from_file(const char* path, u32 channel_flags, u32 sample_index, u32 samples_out);
	WavBuild* predict_from_audio(WavBuild* wb, u32 channel_flags, u32 sample_index, u32 samples_out);

	double iterative_error(WavBuild* wb, u32 channel_flags);

private:
	/* Helper function: return the number of input samples, neural net inputs, and outputs for given G and X. */
	static void ins_and_outs(u32 X, u32 G, u32& nsamples_out, u32& ni_out, u32& no_out);

	/* The number of samples per window. */
	u32 spw;

	/* The number of windows. */
	u32 windows;

	/* Number of inputs and outputs. */
	u32 ni, no;

	/* Maximum number of threads to use. */
	u32 maxth;

	/* Our neural network */
	kann_t*	nnet;

	/* File name for the neural network, if known. */
	std::string fname;

	/* String table, for filenames, etc. */
	StringTable st;

	/* Stopwatch for timing. */
	Stopwatch sw;

	/* Our cache of training data: a map of string index to training data. We save our training history 
	   along with the neural network. This allows us to track the progress in training the neural network, 
	   set variables like learning rate effectively, and improve our training and validation generally. */
	std::unordered_map<u32, TrainVec> train_data;
};

#endif  // __AUDIONNET__
/* end audionnet.h */