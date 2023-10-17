/***

	python_rpc.h

	Functions for calling Python ML scripts, etc. Includes client
	interface to LWGAN models.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __RPC_H
#define __RPC_H

/* The default Python command. */
extern std::string python_cmd;

/* Make a system command call, but suppress any child window (under Windows.) */
extern void system_call(const char* cmd);

class PythonScript {
public:
	PythonScript(const char* script_path)         { path = script_path; }
	PythonScript(const std::string& script_path)  { path = script_path; }

	void add_option(const char* opt, const char* val);
	void add_option(const std::string& opt, const std::string& val);

	void remove_option(const char* opt);
	void remove_option(const std::string& opt);
	void clear_options();

	void run_script() const;

private:
	std::string path;
	std::unordered_map<std::string, std::string> args;
};

/* location of LWGAN models. */
extern std::string lwgan_root_loc;
/* location of LWGAN eval.py script. */
extern std::string lwgan_eval_loc;

/* some helpers to work with latent vectors */
typedef std::vector<double> LatentVector;
extern void lv_zero(LatentVector& lv, u32 N);
extern void lv_mul(LatentVector& lv, double m);
extern void lv_add(const LatentVector& lv1, LatentVector& lv2_dest);
extern void lv_sub(LatentVector& lv1_dest, const LatentVector& lv2);
extern void lv_basis_vector(LatentVector& lv_out, u32 N, u32 axis, double magnitude);
extern void lv_rand(LatentVector& lv, u32 N);
extern void lv_norm(const LatentVector& lv, LatentVector& lv_out);
extern double lv_mag(const LatentVector& lv);
extern void lv_slerp(const LatentVector& v1, const LatentVector& v2, LatentVector& lv_out, double t);
extern void spherical_interpolate(const LatentVector& v1, const LatentVector& v2, double t, LatentVector& lv);

struct GANImg {
	GANImg() { bmp = nullptr; seed = 0; checkpoint = -1; model = ""; }
        GANImg(SBitmap* b, const LatentVector& l, u32 sed, int chptk, const std::string& m) {
		bmp = b; lv = l; seed = sed; checkpoint = chptk; model = m;
	}
	~GANImg() { bmp = nullptr; lv.clear(); }
	SBitmap* bmp;
	LatentVector lv;
	u32 seed;
	int checkpoint;	
	std::string model;
};

/* Large batches (over MAX_BATCH_GEN) are done in randomized order, so each 
   sample has a comparable range of batch feature information in their latents. */
struct LatentVecGANImgOrder {
	u32 idx_orig;
	u32 idx_sort;
	LatentVector lv;
	GANImg gi;
};

/* Save returned GAN images as an image grid. */
extern SBitmap* save_image_grid(const char* fname, std::vector<GANImg>& imgs, bool number_grid = false);

/* Free a vector of GANImgs. */
extern void free_ganimgs(std::vector<GANImg>& imgs);

/* Class for making RPCs to Python for one of my LWGAN models. */
class LWGAN {
public:
	/* we always sync state with the running server, so you don't need to provide model_name/latent_dim
	   at startup */
	LWGAN();
	LWGAN(const char* model_name, u32 latent_dim = 256);
	LWGAN(const std::string& model_name, u32 latent_dim = 256);
	~LWGAN();

	/* Returns a random vector (or N random vectors) of the same dimensionality of our latent space. */
	void random_latent(LatentVector& z) const;
	void random_latent(std::vector<LatentVector>& lvs, u32 N) const;

	/* Set a desired checkpoint (or -1 to load the latest available checkpoint, the default behavior.) */
	void set_checkpoint(int checkpoint);

	/* Generate images corresponding to the passed latent vectors. The value returned is the size (each side; the LWGAN output is square) of the images. */
	u32 generate(const std::vector<LatentVector>& lvs, std::vector<GANImg>& data, bool use_buffer_lv = true);

	/* Generate N images based on random latent space vectors. The value returned is the size of each images' sides. */
	u32 generate(u32 N, std::vector<GANImg>& data, bool use_buffer_lv = true);

	/* Interpolate N images between the two specified latent vectors. The value returned is the size of each images' sides. */
	u32 interpolate(const LatentVector& v1, const LatentVector& v2, u32 N, std::vector<GANImg>& data, bool use_buffer_lv = true);

	/* Interpolate N points between the two specified latent vectors, and fill the passed in vector of LatentVectors with
	   the points. Use to generate in bigger batches. */
	void interpolate(const LatentVector& v1, const LatentVector& v2, u32 N, std::vector<LatentVector>& lv_out);

	/* Interpolate N images along the specified latent basis vector, from min_magnitude to max_magnitude. */
	u32 interpolate_basis(u32 axis, u32 N, double min_mag, double max_mag, std::vector<GANImg>& data);

	/* as above, but fills the latent vector vector. */
	void interpolate_basis(u32 axis, u32 N, double min_mag, double max_mag, std::vector<LatentVector>& lv_out);

	/* Accumulate a latent vector, and generate from it on demand. Allows us to add or average features together,
	   subtract features, or discover a feature cluster. */
	void accumulator_clear();
	void accumulator_add(const LatentVector& lv);
	void accumulator_sub(const LatentVector& lv);
	void accumulator_neg();
	void accumulator_mul(double mlplcand);
	void accumulator_ret(LatentVector& lv_out);
	void accumulator_avg(LatentVector& lv_out);
	u32 number_lvs_accumulated() const;

	/* Set/get the one-at-a-time or exponential moving average model options. */
	bool use_oaat() const  { return oaat; }
	void set_oaat(bool v);
	bool ema_model() const { return use_ema; }
	void set_ema(bool v);

	/* Set the desired output aspect ratio. */
	void set_aspect_ratio(u32 w, u32 h) { aw = w; ah = h; }

	/* Set the desired model seed for PRNG generation. */
	void set_seed(u32 new_seed);
	u32 get_seed() const { return seed; }
	int get_checkpoint() const { return chkpt; }
	const std::string& model_name() const { return mname; }
	u32 latent_dimensionality() const { return ldim; }

	static const u32 MAX_BATCH_GEN = 100;

private:
	void validate_script();
	u32 generate_batch(std::vector<LatentVecGANImgOrder>& lvs, u32 idx, u32 ngen, bool use_buffer_lv = true);
	void generate_buffer_latents();
	void sync_checkpoint_model();
	u32 guess_img_size() const;

	std::string mname;
	u32 ldim;
	int chkpt;
	u32 seed;
	/* Do we generate the images one at a time? There is some state in the generator, so when it works in batches,
           we get different results than when we get images one at a time. If we need determinism, this should be true. 
           NOTE: as of 26-Jul-22, this order of generation issue has been fixed; this flag is probably no longer needed
           and should be false. -- CMS */
	bool oaat;
	/* Do we use the regular generator model, or the EMA generator model with smoothed weights? The EMA generator can 
           output some clean/smooth images, but there may be more training set quotes or other oddities. */
	bool use_ema;
	PythonScript* ps;
	LatentVector acc;
	std::vector<LatentVector> buf_lat;
	u32 nacc;
	u32 ah, aw;
	/* Chosen so that any generate request (even 1 image) becomes large enough to contain a variety of style
	   within the batch, and so that MAX_BATCH_GEN + NUM_BUFFER_LATENTS is a perfect square (doesn't waste any
	   space in output.png.) */
	const u32 NUM_BUFFER_LATENTS = 44;
};

#endif  // __RPC_H
/* end rpc.h */