/***

	ldm.h
	
	Latent diffusion model (LDM) code. The Stable Diffusion server lives
	here, capable of traditional image generation as well as various kinds
	of interpolation, variations, progressive sampling animations, or classifier
	free guidance strength animations.

	Copyright 2023, C. M. Street

***/
#ifndef __LDM_H__
#define __LDM_H__

enum SDSamplerType {
	sd_euler_ancestral = 0, sd_euler, sd_heun, sd_dpm2, sd_dpmpp2sa, sd_dpmpp2m, sd_dpmpp2mv2,
	sd_max_sampler_valid = sd_dpmpp2mv2
};

enum SDSchedulerType {
	sd_default = 0, sd_discrete, sd_karras,
	sd_max_scheduler_valid = sd_karras
};

/* Default negative prompt: by default, it's empty, but you can set it to something else if you like, and it allows us
   to use default arguments in the API. Any prompt specified in txt2img() or img2img() overrides it. */
extern std::string default_neg_prompt;

class SDServer {
public:
	SDServer();
	~SDServer();

	/* Note that w (width) and h (height) below for generated images should be divisible by 8. The code will
	   round these values UP to the next multiple of 8 if necessary. */

	/* Initial images used for img2img must have dimensions divisible by 64. You can use the helper function
	   stretch_for_img2img to automatically resize images to the next largest legal size for img2img use. */

	/* generate an image using a text prompt as conditioning. */
	SBitmap* txt2img(const std::string& prompt,
	                 const std::string& neg_prompt = default_neg_prompt,
	                 u32 w = 512,
	                 u32 h = 512,
	                 double cfg_scale = 7.0,
	                 i64 rng_seed = -1ll,
	                 double variation_weight = 0.0,
	                 i64* seed_return = nullptr);

	/* generate an interpolated image, between two prompts and noise seeds, by spherical linear interpolation ('slerping').
	   these can do morphs and all kinds of interesting vfx. v is the parameter [0.0, 1.0]: 0.0 fully first image, 1.0 fully the second. */
	SBitmap* txt2img_slerp(double v,
	                       const std::string& prompt_1,
	                       const std::string& prompt_2,
	                       const std::string& neg_prompt_1 = default_neg_prompt,
	                       const std::string& neg_prompt_2 = default_neg_prompt,
	                       i64 rng_seed_1 = -1ll,
	                       i64 rng_seed_2 = -1ll,
	                       u32 w = 512,
	                       u32 h = 512,
	                       double cfg_scale = 7.0,
	                       i64* seed_return_1 = nullptr,
	                       i64* seed_return_2 = nullptr);

	/* does the interpolation as above, and places the images in the vector imgs_out. */
	void txt2img_slerp(std::vector<SBitmap*>& imgs_out,
	                   int n_images,
	                   const std::string& prompt_1,
	                   const std::string& prompt_2,
	                   const std::string& neg_prompt_1 = default_neg_prompt,
	                   const std::string& neg_prompt_2 = default_neg_prompt,
	                   i64 rng_seed_1 = -1ll,
	                   i64 rng_seed_2 = -1ll,
	                   u32 w = 512,
	                   u32 h = 512,
	                   double cfg_scale = 7.0,
	                   i64* seed_return_1 = nullptr,
	                   i64* seed_return_2 = nullptr);

	/* TBI: generate a sequence of images with increasing CFG (classifier-free guidance scale) for fixed noise and text input
	   conditioning. Because this is a kind of thing normal people might want to do. */

	/* generate an image using an input image as conditioning. */
	SBitmap* img2img(SBitmap* init_img,
	                 double img_strength,
	                 const std::string& prompt,
	                 const std::string& neg_prompt = default_neg_prompt,
	                 double cfg_scale = 7.0,
	                 i64 rng_seed = -1ll,
	                 double variation_weight = 0.0,
	                 i64* seed_return = nullptr);


	/* load a Stable Diffusion model (1.x or 2.x) from the specified path. Returns true on success. */
	bool load_from_file(const std::string& path);

	/* attempt to load a default SD model of the given version (1 or 2, 0 will search for 2 then 1); if not available, .gguf
	   files in the current directory are checked, if those aren't available, a model can be downloaded from the internet
	   (sd_version == 0 means we will download a 2.x model). Returns true on success. */
	bool load_default_model(int sd_version = 0, bool download_if_missing = false);

	/* get or set secondary parameters (computation threads, sampler type, sampler steps, or scheduler type.)
	   these are set to reasonably sane values by default. */
	void set_nthreads(u32 nt);
	u32 get_nthreads() const			{ return nthreads; }
	void set_sampler_type(SDSamplerType sampler_type);
	SDSamplerType get_sampler_type() const	{ return sampler; }
	void set_scheduler_type(SDSchedulerType scheduler_type);
	SDSchedulerType get_scheduler_type() const	{ return scheduler; }
	void set_steps(u32 st);
	u32 get_steps() const				{ return steps; }

	/* we keep track of the last used RNG seed, in case you want to potentially reproduce the image. You can also ask
	   for it to be returned in (*seed_return) in txt2img()/img2img() above. */
	i64 get_last_seed() const			{ return last_seed; }

	/* for reproducibility, the variation seed used can also be set/requested. */
	void set_variation_seed(i64 seed);
	i64 get_variation_seed() const		{ return variation_seed; }

	/* get the model path */
	const std::string& get_model_path() const	{ return model_path; }

private:
	bool ensure_model();

	/* ggml format model (you can use the Python script by leejet in /inc/external/stable-diffusion/models to convert
	   your own checkpoints, if you like) */
	StableDiffusion* sd_model;

	std::string model_path;
	u32 nthreads;	
	u32 steps;
	i64 last_seed;
	i64 variation_seed;
	SDSamplerType sampler;
	SDSchedulerType scheduler;
};

/* the default Stable Diffusion server. */
extern SDServer sd_server;

const int sd_sampler_count = 7;
const int sd_scheduler_count = 3;

/* string names for the sampler or schedulers */
extern const std::string sd_sampler_names[sd_sampler_count];
extern const std::string sd_schedule_names[sd_scheduler_count];

/* helper function: create an SBitmap from a vector of u8 intensities */
extern SBitmap* bmp_from_vecu8(const std::vector<uint8_t>& invec, u32 w, u32 h);
/* ...and the reverse operation */
extern void vecu8_from_bmp(std::vector<uint8_t>& outvec, SBitmap* bmp);

/* Stretches an image for img2img: the first returns a new bitmap with the stretched content, while the second
   replaces the original bitmap with its stretched version.
   
   If the SBitmap is already a legal size for img2img use, stretch_for_img2img_replace() does nothing and
   stretch_for_img2img() returns a simple copy of bmp. */
extern SBitmap* stretch_for_img2img(SBitmap* bmp);
extern void stretch_for_img2img_replace(SBitmap* bmp);

/* are an image's dimensions legal for img2img? */
extern bool legal_img2img(SBitmap* bmp);

#endif  // __LDM_H__
/*** end ldm.h ***/
