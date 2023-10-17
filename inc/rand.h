/***

	rand.h

	High quality random number generation and seeding.

	Copyright (c) 2014-2022 Chris Street.

***/
#ifndef RAND_H
#define RAND_H

/* Return a random unsigned integer of the specified size. */
extern u64 RandU64(void);
extern u32 RandU32(void);
extern u16 RandU16(void);
extern u8 RandU8(void);

/* Return a random 32 or 64 bit integer, using the Mersenne Twister. */
extern u32 RandMersenneU32(void);
extern u64 RandMersenneU64(void);

/* Return a random integer within a given range (inclusive). The values will 
	have a uniform distribution, even if (max - min + 1) does not divide 2**64. */
extern u64 RandU64Range(u64 min, u64 max);
extern u32 RandU32Range(u32 min, u32 max);
extern u16 RandU16Range(u16 min, u16 max);
extern u8 RandU8Range(u8 min, u8 max);

/* Returns "true" pct% of the time. */
extern bool RandPercent(u32 pct);

/* Random signed integers. */
extern int randint(void);
extern i64 RandI64(void);
extern i32 RandI32(void);
extern i16 RandI16(void);
extern i8 RandI8(void);

/* Random bool/bit value. */
extern bool RandBool(void);
extern u32 randbit(void);

/* Random floats & doubles within a specified range. */
extern float RandFloat(float range_min, float range_max);
extern double RandDouble(double range_min, double range_max);

/*** Get a 32-bit random integer from ISAAC. ***/
extern u32 RandISAAC32(void);

/*** Get a 64-bit random integer from ISAAC64. ***/
extern u64 RandISAAC64(void);

/*** Get a 32-bit float in [0, 1) from ISAAC. ***/
extern float RandISAACFloat(void);

/*** Get a 64-bit double in [0, 1) from ISAAC64. ***/
extern double RandISAACDouble(void);

struct WELLState {
	WELLState();
	// Default seeding function.
	void seed();
	// Initialize this array to seed.
	u32 state[16];	
	// When seeding, set this to zero.
	u32 index;
};

/* Random 32 bit integer using the WELL512 PRNG. */
extern u32 RandWELL512(WELLState& s);

/* Generate a good seed of length "len" 32-bit integers. */
extern void seed_func(u32* buf, u32 len);

/*** Returns a random integer in the range [i1, i2] (inclusive). ***/
extern i32 randbetween(i32 i1, i32 i2);

/*** Returns a random double-precision floating point number. Will not return denormalized values, NaN or infinity. ***/
extern double randf(void);

/*** Returns a random double-precision floating point number in [0, 1]. ***/
extern double randf1(void);

/*** Returns true 1 in N times. ***/
extern bool OneIn(u32 N);

/*** with probability: returns "true" with probability p in [0, 1] */
extern bool WithProb(double p);

/*** with rational probability: returns "true" with probability (N / D). */
extern bool WithProb(u32 N, u32 D);

/*** Returns a random number following a Gaussian distribution with mean M and standard deviation sigma. ***/
extern double rand_gaussian(double M, double sigma);

/*** Grab a group of (pure noise, zero information, truly) random bytes of the specified size from random.org. ***/
/* nbytes must be in the interval [1, 20000]. */
extern char* random_org_rand(u32 nbytes);

/*** Grab an array of (pure noise, zero information, truly) random 16 bit unsigned ints of the specified length from random.org. ***/
/* count must be in the interval [1, 10000]. */
extern u16* random_org_randu16(u16 count);

/*** Grab an array of (pure noise, zero information, truly) random 32 bit unsigned ints of the specified length from random.org. ***/
/* count must be in the interval [1, 5000]. */
extern u32* random_org_randu32(u32 count);

/*** Get the current quota (in bits) for your IP address from random.org. libcodehappy does not send
	requests that would exceed your quota. ***/
extern int random_org_quota(void);

#ifdef CODEHAPPY_NATIVE
/*** Call this at the beginning of the app (before generating any random numbers) to seed with the largest possible
	seed. Grabs the seed values from hardware RNG and random.org, if possible. The return value is the number
	of different sources of true random values used: the higher the better. Not fast, and uses network and
	random.org quota; use this when you want the largest high-quality seed for both 32-bit and 64-bit generators. ***/
extern u32 hardcore_seed(void);
#endif

/* return positive random weights that add up to one */
extern void rand_unit_weight_vector(u32 N, std::vector<double>& wts_out);

/* DetRand: a PRNG that you initialize with a given seed. This class will then give you a
   series of PRNs that can be replicated. */
class DetRand {
public:
	/* construct with initial seed */
	DetRand(u32 init_seed);
	DetRand(void* seed_data, u32 seed_len);
	~DetRand();

	/* change the seed */
	void reseed(u32 new_seed);
	void reseed(void* new_seed, u32 new_seed_len);

	/* reset to initial state -- be aware, if you initialized us with a pointer we're
	   going to assume it's still good */
	void reset();

	/* generate unsigned integers */
	u32 RandU32();
	u32 RandU32Range(u32 min_val, u32 max_val);
	u64 RandU64();
	u64 RandU64Range(u64 min_val, u32 max_val);

	/* generate floating point numbers */
	/* uniform in [0, 1] */
	float randf1();
	double randd1();
	/* uniform in [min_range, max_range] */
	float randf(float min_range, float max_range);
	double randd(double min_range, double max_range);
	/* normal with mean 0 and standard deviation 1 */
	float normalf();
	double normald();
	/* normal with mean M and standard deviation S */
	float normalf(float M, float S);
	double normald(double M, double S);

private:
	void* is;
	double V1, V2, S;
	int phase;
	u32 seedi;
	void* seed;
	u32 slen;
};

/*** Overwrite the specified memory buffer with random garbage. ***/
extern void bitblast(u8* buf, u32 sizeof_buf);

namespace Dice {

/* Roll an n-sided die. */
extern u32 d(u32 n);

/* Roll d n-sided dies. */
extern u32 dd(u32 d, u32 n);

}

/*** Historical PRNGs -- not for regular production use. See the long banner comment in rand.cpp. ***/
extern u32 RANDU(u32* seed);
extern u32 k_and_r_example_rand(u32* seed);
extern u32 glibc_rand(u32* seed);
extern u32 borland_c_lcg_rand(u32* seed);
extern u32 delphi_rand(u32* seed, u32 L);
extern u32 microsoft_c_lcg_rand(u32* seed);
extern u32 microsoft_visual_basic_lcg(u32* seed);
extern u32 native_api_lcg_RTLUniform(u32* seed);
extern u32 minstd_rand0_lcg_rand(u32* seed);
extern u32 minstd_rand_lcg_rand(u32* seed);
extern u32 glibc_old_vms_rand(u32* seed);
extern u64 knuth_mmix_lcg(u64* seed);
extern u32 musl_newlib_lcg_rand(u32* seed);
extern u32 java_util_random_lcg(u32* seed);
extern u64 duff_lcg(u64* seed);
extern u32 acg_rand(void);
extern float jjb_frand(void);

extern u32 k_and_r_lcg_default_seed;
extern u32 randu_lcg_default_seed;

#endif  // RAND_H