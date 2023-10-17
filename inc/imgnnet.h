/***

	imgnnet.h

	The ImgNNet class, which can predict pixels in a bitmap based on nearby pixels. 
	Can be used as a "magic eraser", as a fun image filter, a despeckler/noise reducer, 
	smart selection of objects or shapes, to extend images, for machine vision, image 
	compression, colorization, removal of digital watermarks, etc.

	VerboseStream and PredictAccum live here too.

	Copyright (c) 2022 Chris Street.

***/
#ifndef _IMGNNET_H_
#define _IMGNNET_H_

#include <fstream>

/* A stream that works like an ostream if verbose, but suppresses output if not. Doesn't implement 
   iostream bits, because they're an all-time horrible API. That does mean you use "\n" instead
   of std::endl. For formatting iobits, use the printf() method instead. */
class VerboseStream {
public:
	VerboseStream() : v(true), o(&std::cout)      {}
	VerboseStream(bool v_) : v(v_), o(&std::cout) {}
	VerboseStream(bool v_, std::ostream& o_) : v(v_), o(&o_) {}

	void verbose()    { v = true; }
	void quiet()      { v = false; }
	bool is_verbose() const { return v; }
	bool is_quiet()   const { return !v; }

#define VSOP(__TYPE__) \
	VerboseStream& operator<<(const __TYPE__ i);

	VSOP(std::string&);
	VSOP(char*);
	VSOP(int);
	VSOP(unsigned int);
	VSOP(int64_t);
	VSOP(uint64_t);
	VSOP(float);
	VSOP(double);
#ifdef __GNUC__
	VSOP(long double);
#endif
	VSOP(unsigned char);
	VSOP(signed char);
	VSOP(int16_t);
	VSOP(uint16_t);
	VSOP(bool);
	VSOP(void * const);
#undef VSOP

	int printf(const char* fmt, ...) const {
		if (is_quiet())	return 0;
		int ret;
		va_list args;
		va_start(args, fmt);
		ret = vprintf(fmt, args);
		va_end(args);
		return ret;
	}

private:
	bool v;
	std::ostream* o;
};

struct RGBOut {
	double r;
	double g;
	double b;
	RGBOut operator+=(const RGBOut& rhs);
	RGBOut operator+(const RGBOut& rhs);
	RGBOut operator*(double rhs);
};

/*** PredictAccum, a class that accumulates RGB predictions from our neural network, and
     on request gives us the average prediction for any pixel within the bitmap (and some
     without.) ***/
class PredictAccum {
public:
	void add_prediction(int x, int y, RGBOut& predict);
	void add_prediction(int x, int y, RGBOut& predict, int weight);
	void get_avg_prediction(int x, int y, RGBOut& p);
	void get_total_prediction(int x, int y, RGBOut& p);
	u32 get_num_predictions(int x, int y);
	void fold_in(PredictAccum& pa);
	void reset();

	struct PredictHash {
	    size_t operator()(std::pair<int, int> p) const noexcept {
	        return (size_t)(p.first * 3557 + p.second);
	    }
	};

	void lock()   { m.lock(); }
	void unlock() { m.unlock(); }

	void get_avg_prediction_lock(int x, int y, RGBOut& p) {
		lock();
		get_avg_prediction(x, y, p);
		unlock();
	}

	void get_total_prediction_lock(int x, int y, RGBOut& p) {
		lock();
		get_total_prediction(x, y, p);
		unlock();
	}

	u32 get_num_predictions_lock(int x, int y) {
		u32 ret;
		lock();
		ret = get_num_predictions(x, y);
		unlock();
		return ret;
	}

private:
	std::unordered_map< std::pair<int, int>, std::pair<RGBOut, int>, PredictHash > predictions;
	std::mutex m;
};

/* Information about a training pass. This is POD and can be (is) written/read directly from disk image. */
struct TrainData {
	TrainData();
	void out_to_ramfile(RamFile* rf, u32 version) const;
	void read_from_ramfile(RamFile* rf, u32 version);
	void dump(StringTable& st, VerboseStream& vs) const;

	u32 citer;		// Number of training iterations done.
	u32 retry;		// Number of retries done.
	double err_in;		// Error function at start of training.
	double err_out;		// Error function at the end of training (the best one encountered.)
	double lrate1;		// Learning rate for the first pass.
	double lrate;		// Learning rate for the last pass.
	double lrate_eff;	// The learning rate used on the last iteration the error improved (if any, 0 if not.)
	u32 fname_idx;		// Image file name, if any (index into the ImgNNet's string table.)
	bool flip;		// Is this the result of a flipped run?
	DateTime tim_start;	// Timestamp of start of training run.
	DateTime tim_end;	// Timestamp of end of training run.
	u32 img_w;		// Image width.
	u32 img_h;		// Image height.
};

typedef std::vector<TrainData> TrainVec; 

/* some default erasures we can do */
enum EraseType {
	ERASE_CENTER,
	ERASE_BANDS,
	ERASE_STATIC,
	ERASE_RANDOM_RECTS,
	ERASE_STATIC2,
	ERASE_STATIC3,
	ERASE_DICE,
	ERASE_HALFSAW,
	ERASE_THICK_LINES,
};

/* We allow clients to provide a PredictWindow, so another thread can easily track the
   current prediction status. */
struct PredictWindow {
	PredictWindow() { pa = nullptr; }
	SBitmap* ret;	// predicted bitmap
	SBitmap* erase;	// working erased bitmap (N.B.: probably not valid if done == true)
	u32 pass;	// pass through the loop
	u32 nerased;	// number of pixels currently erased
	u32 nerased_in;	// number of pixels erased at the start of prediction
	bool done;	// is the prediction run finished?
	double ace;	// average component error
	PredictAccum*pa;// A PredictAccum that, if non-NULL, will be filled with copies of the per-pixel predictions.
};

/* Per-thread data for multi-threaded iterative error calculation. */
struct ErrorThreadData { 
	void lock()   { m.lock(); }
	void unlock() { m.unlock(); }
	u32 progress;	// Current row we're working on.
	i64 comp_error;	// Calculated error in progress.
	i64 cd;		// Count of the number of errors evaluated.
	bool done;	// Am I done?
	std::mutex m;	// For access.
};

/* Per-thread data for multi-threaded missing pixel prediction. */
struct PredictPassThreadData {
	u32 ith;	// Thread index
	u32 nth;	// Total number of threads
	u32 pass;	// Which prediction pass we're in.
	PredictAccum pa;// This thread's predictions.
	SBitmap* erase;	// Tells us which pixels are erased.
	void* nnet;	// Our copy of the neural network.
	double *in;	// Our nnet inputs.
	bool done;	// Are we done yet?
};

/* Per-thread data for multi-threaded colorization. */
struct ColorizationThreadData {
	u32 ith;	// Thread index
	u32 nth;	// Total number of threads
	PredictAccum pa;// This thread's predictions.
	void* nnet;	// Our copy of the neural network.
	double *in;	// Our nnet inputs.
	int row;	// The current row this thread is working on.
	bool done;	// Are we done yet?
};

/* Validation set information. */
struct ValidationSetData {
	ValidationSetData();
	void clear();
	void in_from_ramfile(RamFile* rf);
	void out_to_ramfile(RamFile* rf) const;
	void dump(StringTable& st, VerboseStream& vs) const;

	u32 nf;			// Number of files in the validation set
	u32 rad;		// The radius of the input window (useful to match default validation sets)
	std::vector<u32> fs;	// The file names
	std::vector<u32> off;	// Offset (in number of input/output sets) of each file
	Scratchpad inp;		// The neural net inputs (for all validation files)
	Scratchpad out;		// The expected outputs (for all validation files)
};

/* An image brush -- used for painting or improv (see examples/paint.cpp). */
struct ImgBrush {
public:
	ImgBrush(u32 r);
	~ImgBrush();
	void brush_update(u32 w, u32 h);
	void set_from_predictions(PredictAccum& pa);

	double* in;
	double* velocity_color;
	double velocity_brush[2];
	double x;
	double y;
	u32 ni;
	u32 rad;
};

/* The ImgNNet class itself. Specify the size of the input window and the number of hidden layers
   and you're off to the races. */
class ImgNNet {
public:
	ImgNNet();

	/* Initialize a fresh image neural network with the specified radius. */
	ImgNNet(bool verbose, u32 radius, u32 hidden_layers = 2);

	/* As above, but allows setting the number of neurons in each of the hidden layers as well. */
	ImgNNet(bool verbose, u32 radius, u32 nneurons, u32 hidden_layers = 2);

	/* As above, but allows setting the library used (genann or kann) and identity training flag. */
	ImgNNet(bool verbose, u32 radius, u32 nneurons, u32 hidden_layers, bool idtrain, u32 library);

	/* Create a colorization neural network. */
	ImgNNet(bool verbose, u32 rad, u32 r2, u32 nneurons, u32 hidden_layers, bool idtrain, u32 library);

	/* Load an existing neural network from file. */
	ImgNNet(bool verbose, const char* pathname);

	/* Attempt to load a neural network from file. If the file does not exist, create it. */
	ImgNNet(bool verbose, const char* pathname, u32 radius, u32 hidden_layers = 2);

	/* Destructor. */
	~ImgNNet();

	/* Save the neural network to file. */
	void out_to_file(const char* pathname);

	/* Save the neural network to the ramfile format. */
	void out_to_ramfile(const char* pathname);

	/* Load the neural network from file. Determines whether the file is text genann format
	   or our compressed binary format. */
	void read_from_file(const char* pathname);

	/* Load the neural network from a bare genann neural net. Set other parameters to default. */
	void read_from_genann_file(const char* pathname);

	/* Load the neural network from the ramfile format. */
	void read_ramfile_format(const char* pathname);

	/* Train the neural network on an image (file). The output structure (if non-null)
	   will be filled with information about the training. */
	void train_on_image(SBitmap* bmp, TrainData* traindata_out = nullptr);
	void train_on_image(const char* pathname, TrainData* traindata_out = nullptr);

	/* Predict the portions of the bitmap specified by 'erased' (black pixel = not erased, any
	   other pixel = erased). You can use monochrome 1bpp and 'erased' is a literal bitmap. */
	SBitmap* predict_from_missing(SBitmap* bmp, SBitmap* erased);
	SBitmap* predict_from_missing_mt(SBitmap* bmp, SBitmap* erased);

	/* This is glorious and everything, but I don't want the entire bitmap returned, just the
	   predicted portions. If the predicted portion is a rectangle, you'll just get that. If
	   it's not (i.e. there's non-predicted pixels within its boundaries) you'll get the whole
	   bounding rect, but the non-predicted pixels will be black with full alpha transparency
	   (i.e. with blit_blend() you can just slam them onto another bitmap.) */
	SBitmap* predicted_from_missing(SBitmap* bmp, SBitmap* erased);

	/* Make a prediction based on the specified pixel of the bitmap, and add it to the PredictAccum. */
	void prediction_for_pixel(SBitmap* bmp, int x, int y, PredictAccum& pa);

	/* Make a prediction for the passed in ImgBrush; put the result in the passed in PredictAccum. */
	void prediction_for_brush(ImgBrush* ib, PredictAccum& pa);

	/* Generate a predicted bitmap that is (weight1) parts bmp1 and (1 - weight1) parts bmp2. */
	SBitmap* generate_weighted_prediction(SBitmap* bmp1, SBitmap* bmp2, double weight1);

	/* Colorize a bitmap. (If the bitmap isn't grayscale, it's converted internally and then colorized.) */
	SBitmap* colorize_bitmap(SBitmap* bmp);

	/* Set/get the verbose status. */
	void verbose() { vs.verbose(); }
	void quiet()   { vs.quiet();   }
	bool is_verbose() const { return vs.is_verbose(); }

	/* Set the minimum predictions. Setting mp = 2 often gives about as good results as mp = 4, and it's 
           much faster to converge. Sometimes mp = 4 does give better outputs, however. */
	void min_predict_high(void) { mp = 4; }
	void min_predict_low(void)  { mp = 2; }
	void min_predict_set(u32 v) { mp = v; }

	/* Get or set whether we train flipped versions of the image as well. */
	void train_flips(bool ison) { flip = ison; }
	bool are_flipping() const   { return flip; }

	/* Get or set the maximum length/width of a trained image (0 means no limit.) */
	void set_max_size(u32 ms) { max_size = ms; }
	u32 get_max_size() const  { return max_size; }

	/* Get or set the maximum training iterations or maximum training retries. */
	u32 get_max_iter() const  { return max_iter; }
	u32 get_max_retry() const { return max_retry; }
	void set_max_iter(u32 mi) { max_iter = mi; }
	void set_max_retry(u32 r) { max_retry = r; }
	void set_out_erased(bool e) { out_erased = e; }

	/* Get or set the default learning rate. */
	double get_default_learning_rate() const { return lrate; }
	void set_default_learning_rate(double v) { lrate = v; }

	/* Give the number of inputs and outputs for an ImgNNet of the specified radius. */
	static void inout_from_radius(u32 radius, u32 rd2, u32& in_o, u32& out_o);

	/* Return an appropriate erased bitmap for the passed-in bitmap and erase type. */
	static SBitmap* get_erased_bmp(SBitmap* bmp, EraseType et);

	/* Create a "best neighbors" reconstruction of the missing pixels. (Draws directly on bmp.) */
	static void get_best_neighbors_bmp(SBitmap* bmp, SBitmap* erase);

	/* Dump neural net information to stdout. */
	void dump();

	/* Have we a valid neural network? */
	bool valid() const;

	/* Remove the training and validation data. Useful for nnets that you want to distribute; 
	   you don't need to reveal filenames on your system or your training history. */
	void strip_traindata();
	void strip_validation();

	/* Output training data information to CSV. */
	void traindata_to_csv(const char* fname);

	/* Outputs the neural network to disk, if it knows its filename. */
	void update_on_disk();

	/* Split the passed-in image into 5x5 chunks and have the neural network draw them. */
	SBitmap* image_5x5_filter(SBitmap* bmp);

	/* Split the passed-in image into n x n chunks and have the neural network draw them. */
	SBitmap* image_nxn_filter(SBitmap* bmp, u32 n);

	/* Provide a PredictWindow for the prediction code to update during runs. */
	void set_predict_window(PredictWindow* pw_) { pw = pw_; }

	/* Add an image file to our validation set. This creates a new validation test case. */
	void add_validation_image(const char* pname);

	/* Perform validation over our validation set. Runs all validation test cases. */
	double validation_set_run();

	/* Output the validation set data as a default validation set. */
	void out_validation_set();

	/* Disk training: ensure the neural network knows the identity mapping. */
	void train_identity(u32 niterations);
	void train_identity_optimize(void);
	void mix_in_identity(bool id)      { identity = id; }
	bool identity_mixed_in(void) const { return identity; }

	/* Train another ImgNNet, of the same radius, with our outputs. May yield a better
	   starting point for a	new nnet than random weights. */
	void train_new_nnet(ImgNNet& to_train, u32 niter);

	/* Get or set the maximum number of evaluation threads. */
	void set_max_threads(u32 v);
	u32 get_max_threads() const { return maxmt; }
	void max_threads()          { set_max_threads(UINT32_MAX); }

	/* Return our radius. */
	u32 radius() const { return d; }
	std::string pathname() const { return nn_fname; }

private:
	/* Helper function: core training method. */
	void train_on_image_core(SBitmap* bmp, TrainData* traindata_out, bool isflipped);

	/* Helper function: predict from erased. */
	bool predict_pass_from_missing(SBitmap* bmp, SBitmap* berase, SBitmap* bout);
	void predict_pass_from_missing_mt_t(SBitmap* bmp, PredictPassThreadData* pptd);

	/* Helper function: colorization. */
	void colorize_mt(SBitmap* bmp, ColorizationThreadData* ctd);

	/* Helper function: Determine radius from the number of inputs/outputs. */
	void radius_from_inout(u32& d_out, u32& d2_out, bool coloriz) const;

	/* Helper function: do the pixels at dx, dy represent a pixel in the training or validation sets? */
	bool is_in_training(int dx, int dy) const;
	bool is_in_validation(int dx, int dy) const;

	/* Helper function: set the validation and training pixels randomly at the start of a pass. Although the 
           choice of dx/dy belonging to the sets is changed at the start of each training pass, the validation 
	   and training pixel sets are ALWAYS disjoint for a given image, across passes or even across trainings. */
	void set_training_pixels(void);
	void set_validation_pixels(void);

	/* Helper function: set default parameters when we load from a bare genann file or create a new nnet. */
	void set_default_parameters(void);

	/* Helper functions: Persist or load the cached training data to disk. */
	void traindata_to_ramfile(RamFile* rf) const;
	void traindata_from_ramfile(RamFile* rf);

	/* Helper: Calculate the average component error of the neural network's prediction of the passed-in bitmap. */
	double iterative_error(SBitmap* bmp);
	double iterative_error_mt(SBitmap* bmp);

	/* Helper function: Count and update the number of erased pixels. */
	void count_erased(SBitmap* bmp);

	/* The per-thread function for iterative error calculation. */
	void iterative_error_mt_t(SBitmap* bmp, u32 ith, u32 nth, ErrorThreadData* etd);

	/* The per-thread function for validation. */
	void validation_eval_mt_t(void* nnet_use, double* inp, double* outp, u32 np, u32 ith, u32 nth, double* err);

	/* Helper functions that operate on the neural nets, calling the appropriate underlying library functions. */
	void* nnet_copy(void* nnet_);
	void nnet_train(void *nnet_, double* in_, double* out_, double clrate_);
	void nnet_free(void* nnet_);
	void* nnet_from_ramfile(RamFile* rf);
	void nnet_out_to_ramfile(RamFile* rf);
	void* nnet_init(u32 n_in, u32 hidden_layers, u32 neurons, u32 n_out);
	double const* nnet_run(void* nnet_, double const* in_, u32 ithread = 0);
	void in_to_fin(double const* in_);
	void out_to_fout(double const* out_);
	void fout_to_out(float const* fout_);

	/* Batch training: used for KANN format. We bunch up the training data and perform multiple epochs
	   of training when we've queued up enough. For genann, just does the training as ordinary. */
	void batch_train_begin(void);
	void batch_train(void* nnet_, double* in_, double* out_, double clrate_);
	void batch_train_end(void* nnet_, double clrate_);

	/* One iteration training the identity mapping. */
	void train_identity_iter(void);

	/* Set the inputs and outputs for training. Supports colorization nnets. */
	void fill_train_inout(SBitmap* bmp, int x, int y, u32& ci, u32& co);
	void fill_train_inout(SBitmap* bmp, int x, int y, u32& ci, u32& co, double* vin, double* vout);
	void fill_train_in(SBitmap* bmp, int x, int y, u32& ci, double* tin);

	/* The radius. The inputs for a location will be all pixels within [0, d] distance of that location,
	   the d-circle, while the outputs will be the (d+1)-perimeter, all pixels within (d, d + 1]. */
	u32 d;

	/* Number of inputs and outputs. There are 3 for each pixel, one for each chroma channel. */
	u32 ni;
	u32 no;

	/* Number of neurons in each hidden layer. */
	u32 neurons;

	/* The colorspace used by this neural net. */
	colorspace space;

	/* Secondary radius: this gives the size of the output for colorization neural nets. */
	u32 d2;

	/* Default learning rate. This may be adjusted. */
	double lrate;

	/* Current learning rate. */
	double clrate;

	/* The current working copy of our ANN. */
	void* nnet;

	/* The 'last best' copy of our ANN. Note that this and nnet can and often do both point to the same place! */
	void* nnet_best;

	/* Do we use neighboring pixels to estimate erased pixels in inputs to make more predictions? And 
	   if so, how many do we allow? And for how many prediction passes should they be used? */
	bool use_neighbors;
	u32 max_neighbors;
	u32 max_neighbor_pass;

	/* The output stream. This will only print anything to stdout if we're verbose. */
	VerboseStream vs;

	/* Neural network inputs and outputs. */
	double* in;
	double* out;
	// Float equivalents, for use with KANN.
	float* fin;
	float* fout;
	// For batch training.
	float** batch_in, ** batch_out;
	u32 batch_i;

	/* Version information. */
	bool verdata;	// Was the version data block present in the .rfn?
	u32 nnet_ver;	// What neural net format are we using? (genann and kann supported currently.)
	bool colorize;	// Is this a colorization neural network?
	bool identity;	// Do we mix identity training in with image training?
#ifdef IMGNNET_CONVOL
	bool convol;	// Are we using a convolution layer?
#endif

	/* Parameters used during training. */
	bool flip;	// do I flip the image horizontally?
	u32 max_size;	// 0 for no limit. If non-zero, images with sides longer than this get resized.
	u32 training;	// the pixel in the 4x4 block that is currently in the training set. Changes for each training pass.
	u32 validation;	// the pixel in the 4x4 block that is currently in the validation set. Changes for each image.
	u32 max_iter;	// Maximum number of iterations in this round of training.
	u32 max_retry;	// Maximum number of retries in this round of training.
	u32 fn_index;	// Index to the filename in the string table.

	/* Parameters used during prediction. */
	u32 mp;		// the minimum number of accumulated predictions required to un-erase a pixel using the nnet.
	u32 pass;	// the current prediction pass.
	bool out_erased;// do I output the erased bitmap?
	bool out_neighb;// do I output the best neighbor bitmap?
	u32 ce_st, ce_n;// Number of erased pixels at the start vs. now (for the timer)

	/* Maximum number of threads to use in evaluation. */
	u32 maxmt;

	/* PredictWindow provided by the consumer. */
	PredictWindow* pw;

	/* Stopwatch for timing. */
	Stopwatch sw;

	/* The pathname we loaded this ANN from; empty if none. */
	std::string nn_fname;

	/* String table, for filenames, etc. */
	StringTable st;

	/* The validation suite. */
	ValidationSetData vsd;

	/* Our cache of training data: a map of string index to training data. We save our training history 
	   along with the neural network. This allows us to track the progress in training the neural network, 
	   set variables like learning rate effectively, and improve our training and validation generally. */
	std::unordered_map<u32, TrainVec> train_data;

	friend class ImgDiscrim;
};

/* An image discriminator. This is given outputs from the ImgNNet, and predicts the probability that
   they came from an original image (as opposed to an ImgNNet-generated image.) Can be used to improve 
   accuracy or for 'improv'. */
class ImgDiscrim {
public:
	ImgDiscrim();
	ImgDiscrim(const char* imgnnet, bool verbose, u32 nneurons = 700, u32 hidden_layers = 3);
	~ImgDiscrim();

	/* Load and save. */
	void persist_to_file(const char* path);
	void persist(RamFile* rf);
	void load_from_file(const char* path);
	void load(RamFile* rf);

	/* Training. Outputs from the image give the result 1, while synthetic outputs produced from 
	   the image give the result 0. */
	void train_on_image(const char* path);
	void train_on_image(SBitmap* bmp);

	/* Loss estimation. */
	float loss_est(const char* path);
	float loss_est(SBitmap* bmp);

	/* Evaluation at a single point. */
	float eval(SBitmap* bmp, int x, int y);

	/* Evaluation for an ImgBrush. */
	float eval(ImgBrush* ib, PredictAccum& pa);

	/* Free data on our object. */
	void free();

private:
	static u32 inputs_from_radius(u32 r);

	/* Training helpers. */
	void train_img_point(SBitmap* bmp, int x, int y);
	void batch_train();
	void batch_end();

	/* Calculate loss at a point. */
	float loss_point(SBitmap* bmp, int x, int y);

	/* Create the discriminator neural net. */
	void init(u32 hl, u32 n);

	kann_t* nnet;
	ImgNNet inet;
	char* inet_path;
	u32 ni, no;
	u32 rad;
	float* in;
	float* out;
	float** bin;
	float** bout;
	u32 bt;
	VerboseStream vs;
};

/*** Other exports. ***/

/* Convenient color distance metric. */
extern u32 color_distance(RGBColor c1, RGBColor c2);

/* Channel intensity, as a double in [0, 1]. */
extern double channel_intensity(SBitmap* bmp, u32 ch, int x, int y);

/* Grayscale intensity, as a double in [0, 1]. */
extern double gray_intensity(SBitmap* bmp, int x, int y);

/* Express a time interval (in ms) as a string of the format "MM:SS.mmm". */
extern const char* timepr(u64 mills);

/* Is the named file a text file? If 'false' it's probably a binary file. */
extern bool file_is_text(const char* pathname);

#endif  // _IMGNNET_H_
/*** end imgnnet.h ***/