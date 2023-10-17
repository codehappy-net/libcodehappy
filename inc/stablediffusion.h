/***

	stablediffusion.h
	
	A C++ library interface to my various Python LDM scripts. Gives access to my many Dreamboothed models,
	interpolation and animation, latent space exploration, imgtoimg, etc.
	
	Copyright (c) 2022 Chris Street.

***/
#ifndef _STABLEDIFFUSION_H_
#define _STABLEDIFFUSION_H_

struct SDImg {
	SDImg();
	~SDImg();
	/* The raw image data itself. */
	SBitmap* img;
	/* The index to the name of the model in the StableDiffusion string table */
	u32 model_idx;
	/* PRNG seed for the unguided conditioning (noise.) */
	u32 unguided_seed;
	/* Number of frames in interpolation (0 = no interpolation) */
	u32 interp_frames;
	/* Frame index if interp_frames > 0, iteration index otherwise */
	u32 frame_idx;
	/* The scale value for weighting the unguided conditioning */
	double scale;
	/* Strength value for img2img */
	double strength;
	/* Sampler type */
	int sampler;
	/* Number of denoising steps. */
	u32 steps;
	/* Width and height. */
	u32 w;
	u32 h;
	/* Text conditioning, if any. */
	std::string text_cond;
	/* Second text conditioning (for interpolation), if any. */
	std::string text_cond_2;
	/* Negative prompt, if any. */
	std::string neg_prompt;
	/* Second PRNG random seed (for interpolation), if any. */
	u32 seed2;
	/* The actual latents for the guided conditioning. Shape: 77 x 768. */
	double* latents_guided;
	/* The actual latents for the unguided (empty/negative prompt) conditioning. Shape: 77 x 768. */
	double* latents_unguided;
	/* The noise/start code. Should be X of these, where X = 4 RGBA channels * h // 8 * w // 8 */
	double* start_code;

	/* Fill latents from a text file. */
	bool fill_latents(bool guided, const char* fname, u32 idx);
	/* Fill start code from a text file. */
	bool fill_start_code(const char* fname);
};

/*** Trained model data. ***/
struct LDMModel {
	const char* path;
	std::string char_name;
	const char* mod;
	const char* cls;
	const char* charac;
	int weight;
	int flags;
};

#define SAMPLER_DDIM	0
#define SAMPLER_PLMS	1
#define SAMPLER_MAX	1

typedef std::vector<SDImg> SDImgs;

/* The Stable Diffusion interface. */
class StableDiffusion {
public:
	StableDiffusion();

	void save_imgs(SDImgs& imgs, const char* filename);
	void load_imgs(SDImgs& imgs, const char* filename);
	void give_models(LDMModel* models_, u32 cmodels_) { models = models_; cmodels = cmodels_; }

private:
	LDMModel* models;
	u32 cmodels;
};

#define SCRIPT_LOCATION	"./scripts/stable_txt2img.py"

/*** Model flags ***/

// model created using a cloud Paperspace A6000
#define FLAG_PAPERSPACE	1
// model created using the low-memory 8 bit Adam optimizer DreamBooth implementation
#define FLAG_LOMEM		2
// model created using the modified Penna Dreambooth implementation
#define FLAG_DREAMBOOTH_LOCAL	4
// model generated with a low number of training images
#define FLAG_LOSHOT		8
// model generated with a high number of training images
#define FLAG_HISHOT		16
// model trained deliberately with learning rate of 1e-7 versus 1e-6
#define FLAG_1E7		32
// model regularized on images of its instance (rather than, e.g. generic cartoon mice in regular/)
#define FLAG_REG_SELF		64
// latest model (in a series)
#define FLAG_LATEST		128
// golden model (this is awarded to at most one model per class)
#define FLAG_GOLDEN		256
// an early checkpoint (these recontextualize differently and may be better for certain styles)
#define FLAG_EARLY		512
// perhaps not a golden model, but a good one that should be recognized as such
#define FLAG_SILVER		1024
// descendant of Waifu Diffusion 1.3 instead of just base Stable Diffusion -- although its online
// reputation is big-boobied anime girls, WD's actually pretty good at recontextualizing animation
// in general (Western-style or anime), and it's also been trained on many thousands of booru tags
// that allow more precise CLIP-prompting. That it's also good at furry and NSFW stuff may be a
// plus or a minus, de gustibus non est disputandum. 
#define FLAG_WAIFU		2048
// POT (Potentially Over Trained)
#define FLAG_POT		4096
// Base model was Stable Diffusion 1.5 (instead of 1.4)
#define FLAG_SD15		8192
// the best quality models (currently) will have these flags
#define FLAG_BEST		(FLAG_DREAMBOOTH_LOCAL | FLAG_HISHOT | FLAG_1E7)
// this is very good quality too
#define FLAG_BETTER		(FLAG_DREAMBOOTH_LOCAL | FLAG_1E7)

#endif  // _STABLEDIFFUSION_H_
/* end stablediffusion.h */
