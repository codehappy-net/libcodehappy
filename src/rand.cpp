/***

	rand.cpp

	High quality random number generation and seeding. Some of the algorithms include:

	* ISAAC/ISAAC64, a fast cryptographically strong PRNG
	* WELL512, a fast PRNG designed by Matsumoto/L'Ecuyer/Panneton
	* the Mersenne Twister included in the C++11 stdlib, with workarounds for 
		MinGW std::random_device bugs.
	* Operating system-specific cryptographically strong sources of entropy.
	* Random numbers from the random.org website.
	* High-quality seeding functions.
	* Re-seedable easy to use deterministic RNGs for repeatable results
	* Many historic PRNGs (of varying quality) used by various devices or libraries.

	Copyright (c) 2014-2022 Chris Street.

***/
#include <random>
#include <chrono>
#include <time.h>
#ifdef CODEHAPPY_LINUX
#include <sys/random.h>
#endif

#include "libcodehappy.h"

/*** horrible rigamarole to work around stdlib implementation issues begins ***/

#ifdef CODEHAPPY_NATIVE
#define SEED_STRONG
#endif

namespace {

#ifdef SEED_STRONG
#define	SEED_TYPE	std::seed_seq&
static std::seed_seq __rng_seed_state;
#else  // !SEED_STRONG
#define	SEED_TYPE	u32
#endif

/* needed because of MinGW stdlib bug */
SEED_TYPE __gimme_seed(void) {
#ifdef CODEHAPPY_NATIVE
#ifdef SEED_STRONG
	/* Let's use hardware RNG and hardware counter to create a better seed, if possible. */
	/* A 2^19937 period length per sequence is great, but 2^32 maximum different sequences isn't nearly enough. */
	u32 seeds[8];
	u64 val = hardware_counter();
	seeds[0] = (u32)(val & 0xffffffffull);
	// this one likely only has ~8-16 bits of entropy, but might as well throw it in
	seeds[1] = (u32)(val >> 32ULL);
	if (hardware_rng_supported()) {
		for (int e = 2; e < 8; ++e)
			seeds[e] = hardware_rng();
		__rng_seed_state.generate(seeds, seeds + 8);
	} else {
		__rng_seed_state.generate(seeds, seeds + 2);
	}
	return __rng_seed_state;
#else  // !SEED_STRONG
	/* Use the hardware counter for the (single) seed. */
	return static_cast<u32>(hardware_counter());
#endif // !SEED_STRONG
#else  // !CODEHAPPY_NATIVE
	/* Use the best resolution clock for the (single) seed. */
	// TODO: Can we do better than this in WebAssembly?
	return static_cast<u32>(std::chrono::high_resolution_clock::now().time_since_epoch().count());
	//return static_cast<u32>(time(NULL));
#endif // !CODEHAPPY_NATIVE
}

/*** RNG state. ***/
// MinGW's implementation of std::random_device gives deterministic seeds, what the heck? 
// static std::random_device __rngdev;
static std::mt19937 __twister(__gimme_seed());
static std::uniform_int_distribution<u32> __u32_dist;
static std::uniform_int_distribution<u64> __u64_dist;

}  // anon namespace

/*** horrible rigamarole ends ***/

u32 RandMersenneU32(void) {
	return __u32_dist(__twister);
}

u64 RandMersenneU64(void) {
	return __u64_dist(__twister);
}

void seed_func(u32* buf, u32 len) {
	u32 c = 0;
	if (iszero(len))
		return;

	// Use the current 64-bit hardware counter (if supported) and the epoch time to start
#ifdef CODEHAPPY_NATIVE
	u64 v = hardware_counter();
	buf[c++] = (u32)(v & 0xffffffff);
	if (c >= len)
		return;
	buf[c++] = (u32)(v >> 32ULL);
#endif
	if (c >= len)
		return;
	buf[c++] = static_cast<u32>(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	// Now fill in some hardware-generated entropy, if that's available.
	if (hardware_rng_supported()) {
		for (int e = 0; c < len && e < 8; ++e)
			buf[c++] = hardware_rng();
	}
	
#ifdef CODEHAPPY_LINUX
	// Under Linux, let's try to finish off the buffer with noise from /dev/urandom.
	ssize_t nb;
	nb = getrandom((void *)(buf + c), sizeof(u32) * (len - c), GRND_NONBLOCK);
	if (nb > 0) {
		nb /= sizeof(u32);
		c += (u32)nb;
	}
#endif	

	// Finish the buffer, if needed, with some Mersenne Twister random numbers.
	while (c < len) {
		buf[c++] = RandMersenneU32();
	}
}

/*** ISAAC initialization ***/
static isaac64_ctx __isaac64_state;
static isaac_ctx   __isaac_state;
static bool        __isaac_init = false;
#ifdef CODEHAPPY_NATIVE
static u32         __hardcore_init = 0;
#endif

void seed_isaac(void) {
	u32 seed[32];
	seed_func(seed, 32);
	isaac64_init(&__isaac64_state, (unsigned char*)seed, sizeof(u32) * 32);
	isaac_init(&__isaac_state, (unsigned char*)seed, sizeof(u32) * 32);
	__isaac_init = true;
}

/*** Get a 64-bit random integer from ISAAC64. ***/
u64 RandISAAC64(void) {
	if (!__isaac_init) {
		seed_isaac();
	}
	return isaac64_next_uint64(&__isaac64_state);
}

/*** Get a 32-bit random integer from ISAAC. ***/
u32 RandISAAC32(void) {
	if (!__isaac_init) {
		seed_isaac();
	}
	return isaac_next_uint32(&__isaac_state);
}

/*** Get a 32-bit float in [0, 1) from ISAAC. ***/
float RandISAACFloat(void) {
	if (!__isaac_init) {
		seed_isaac();
	}
	return isaac_next_float(&__isaac_state);
}

/*** Get a 64-bit double in [0, 1) from ISAAC64. ***/
double RandISAACDouble(void) {
	if (!__isaac_init) {
		seed_isaac();
	}
	return isaac64_next_double(&__isaac64_state);
}

u32 RandU32(void) {
	/* Now using the ISAAC cryptographically strong PRNG initialized with at least 
		1024 bits of entropy. On native builds, some of the entropy will come
		from hardware RNG and/or the hardware sub-nanosecond counter. */
	return RandISAAC32();
}

u64 RandU64(void) {
	/* Now using the ISAAC cryptographically strong PRNG initialized with at least 
		1024 bits of entropy. On native builds, some of the entropy will come
		from hardware RNG and/or the hardware sub-nanosecond counter. */
	return RandISAAC64();
}

u32 RandU32Range(u32 min, u32 max) {
	u32 rnd, resid, multiple, diffm;
	SORT2(min, max, u32);
	if (min == max)
		return max;
	diffm = max - min + 1;

	/* Make sure the values returned are uniform. */
	do {
		rnd = RandU32();
		resid = rnd % diffm;
		multiple = rnd - resid;
	} while (((multiple + diffm - 1) & 0xFFFFFFFFUL) < multiple);

	return min + resid;
}

u64 RandU64Range(u64 min, u64 max) {
	u64 rnd, resid, multiple, diffm;
	SORT2(min, max, u64);
	if (min == max)
		return max;
	diffm = max - min + 1;

	/* Make sure the values returned are uniform. */
	do {
		rnd = RandU64();
		resid = rnd % diffm;
		multiple = rnd - resid;
	} while (((multiple + diffm - 1) & 0xFFFFFFFFFFFFFFFFULL) < multiple);

	return min + resid;
}

u16 RandU16(void) {
	static u64 st = RandU64();
	static u32 ct = 0;
	u16 ret;
	if (4 == ct) {
		st = RandU64();
		ct = 0;
	}
	ret = (u16)(st & 0xffff);
	st >>= 16;
	++ct;
	return ret;
}

u8 RandU8(void) {
	static u64 st = RandU64();
	static u32 ct = 0;
	u8 ret;
	if (8 == ct) {
		st = RandU64();
		ct = 0;
	}
	ret = (u8)(st & 0xff);
	st >>= 8;
	++ct;
	return ret;
}

u16 RandU16Range(u16 min, u16 max) {
	return (u16)RandU32Range((u32)min, (u32)max);
}

u8 RandU8Range(u8 min, u8 max) {
	return (u8)RandU32Range((u32)min, (u32)max);
}

bool RandPercent(u32 pct) {
	u32 i = RandU32Range(0, 99);
	return (i < pct);
}

int randint(void) {
	u32 ret = RandU32();
	return (int)ret;
}

i64 RandI64(void) {
	u64 ret = RandU64();
	return (i64)ret;
}

i32 RandI32(void) {
	return (i32)randint();
}

i16 RandI16(void) {
	u16 ret = RandU16();
	return (i16)ret;
}

i8 RandI8(void) {
	u8 ret = RandU8();
	return (i8)ret;
}

u32 randbit(void) {
	static u64 st = RandU64();
	static u32 ct = 0;
	u32 ret;
	if (64 == ct) {
		st = RandU64();
		ct = 0;
	}
	ret = (st & 1);
	st >>= 1;
	++ct;
	return ret;
}

bool RandBool(void) {
	return (randbit() == 1);
}

float RandFloat(float range_min, float range_max) {
#if 0
	double d = std::abs(range_max - range_min);
	u32 u = RandU32();

	d /= double(0xfffffffful);
	d *= double(u);
	return range_min + float(d);
#else
	// let's use ISAAC to directly create random floats
	float delta = std::abs(range_max - range_min);
	float i = RandISAACFloat();
	return std::min(range_min, range_max) + i * delta;
#endif
}

double RandDouble(double range_min, double range_max) {
#if 0
	double d = std::abs(range_max - range_min);
	u64 u = RandU64();

	d *= double(u);
	d /= double(0xffffffffffffffffull);
	return range_min + d;
#else
	// let's use ISAAC-64 to directly create random doubles
	double delta = std::abs(range_max - range_min);
	double i = RandISAACDouble();
	return std::min(range_min, range_max) + i * delta;
#endif
}

/*** Returns a random integer in the range [i1, i2] (inclusive). ***/
i32 randbetween(i32 i1, i32 i2) {
	u32 sz;
	SORT2(i1, i2, i32);
	sz = (u32)(i2 - i1);
	if (0 == sz)
		return(i1);

	return i1 + (i32)(RandU32Range(0, sz));
}

/*** Returns a random double-precision floating point number. Will not return denormalized values, NaN or infinity. ***/
double randf(void) {
	u64 ri;
	double* d;
	assert(sizeof(u64) == sizeof(double));
	forever {
		ri = RandU64();
		d = (double *)(&ri);
		if (!isnan(*d) && isfinite(*d) && isnormal(*d))
			break;
	}
	return(*d);
}

/*** Returns a random double-precision floating point number in [0, 1]. ***/
double randf1(void) {
#if 0
	u64 ri;
	ri = RandU64();
	return (double)(ri) / (double)(0xFFFFFFFFFFFFFFFFULL);
#else
	/* let's do this now. Technically this returns values in [0, 1). */
	return RandISAACDouble();
#endif
}

/*** Returns true 1 in N times. ***/
bool OneIn(u32 N) {
	ship_assert(N > 0);
	return (RandU32Range(1, N) == N);
}

/*** with probability: returns "true" with probability p in [0, 1] */
bool WithProb(double p) {
	return randf1() < p;
}

/*** with rational probability: returns "true" with probability (N / D). */
bool WithProb(u32 N, u32 D) {
	return RandU32Range(0, D - 1) < N;
}

/*** Helper function: Gaussian distribution random number generator with mean 0 and std. dev. 1. ***/
/*** From Marsalgia and Bay, 'A Convenient Method for Generating Normal Variables' ***/
static double __gaussian01(void) {
	// our PRNG state
	static double V1, V2, S;
	static int phase = 0;
	double ret;

	if (iszero(phase)) {
		/* find a random point in the unit circle centered at (0, 0) */
		do {
			double U1 = randf1();
			double U2 = randf1();

			V1 = 2. * U1 - 1.;
			V2 = 2. * U2 - 1.;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1. || S == 0.);
		ret = V1 * sqrt(-2. * log(S) / S);
	} else
		ret = V2 * sqrt(-2. * log(S) / S);
	phase = 1 - phase;
	
	return(ret);
}

/*** Returns a random number following a Gaussian distribution with mean M and standard deviation sigma. ***/
double rand_gaussian(double M, double sigma) {
	return M + __gaussian01() * sigma;
}

WELLState::WELLState() {
	seed();
}

void WELLState::seed() {
	seed_func(state, 16);
	index = 0;
}

u32 RandWELL512(WELLState& s) {
	// Copy-pasta from Chris Lomont (lomont.org)
	u32 a, b, c, d;
	a = s.state[s.index];
	c = s.state[(s.index+13)&15];
	b = a^c^(a<<16)^(c<<15);
	c = s.state[(s.index+9)&15];
	c ^= (c>>11);
	a = s.state[s.index] = b^c;
	d = a^((a<<5)&0xDA442D24UL);
	s.index = (s.index + 15)&15;
	a = s.state[s.index];
	s.state[s.index] = a^b^d^(a<<2)^(b<<18)^(c<<28);
	return s.state[s.index];
}

#define RANDOM_ORG_URI		"https://www.random.org/integers/?num=%d&min=0&max=65535&col=1&base=10&format=plain&rnd=new"
#define RANDOM_ORG_QUOTA_URI	"https://www.random.org/quota/?format=plain"

/*** Get the current quota (in bits) for our IP address from random.org. ***/
int random_org_quota(void) {
	RamFile* rf = FetchURI(RANDOM_ORG_QUOTA_URI);
	NOT_NULL_OR_RETURN(rf, 0);
	if (rf->length() == 0) {
		delete rf;
		return 0;
	}
	char* w = (char *)rf->buffer();
	NOT_NULL_OR_RETURN(w, 0);
	while (isspace(*w))
		++w;
	int ret = atoi(w);
	delete rf;
	return ret;
}

/*** Grab a group of random bytes of the specified size from random.org. ***/
char* random_org_rand(u32 nbytes) {
	char URL[256];
	char* buf, *w;
	char* we;
	char* ret = nullptr;
	u32 nsh;
	int i;
	int quota;
	
	if (!is_between(nbytes, 1, 20000))
		return(nullptr);
	nsh = nbytes >> 1;
	if (truth(nbytes & 1))
		++nsh;

	/* Verify that the current quota can satisfy the request. */
	quota = random_org_quota();
	if (quota < int(nsh) * 16)
		return nullptr;

	sprintf(URL, RANDOM_ORG_URI, nsh);

	RamFile* rf = FetchURI(URL);
	NOT_NULL_OR_RETURN(rf, nullptr);
	buf = (char *)rf->buffer();
	NOT_NULL_OR_RETURN(buf, nullptr);
	/***

		Check for an empty or truncated response, and verify that the response contains
		only the requested random integers.

		If the HTTP request under FetchURI() failed for whatever reason, we're
		probably getting an empty result, or a default page with an error message.
		(We should auto-follow redirects, at least with Wget. Maybe not in WASM.)
		If the response from random.org existed but for some reason was
		truncated, we'd also like to bail. So check the length of the ramfile.

		Any valid response of nsh random 16-bit integers should be at least nsh*2 bytes
		long; even in the nsh=1 case this should work (single digit plus newline),
		but it's important to be right for nsh=1, so we'll skip the lower limit in that 
		case.

		The UPPER limit, however, is pretty important, too. Each response should be,
		at most, nsh*7 bytes. ("65535" plus a Windows 2 byte newline.) We also check 
		against the upper limit: any response longer than that is likely an error message
		or garbage.

		We then do basic validation of the contents of the response.

	***/
	if ((nsh == 1 && rf->length() == 0) || (nsh > 1 && rf->length() < nsh * 2)) {
		goto LErr;
	}
	if (rf->length() > nsh * 7) {
		goto LErr;
	}
	/* Quickly scan through the response. It must contain nothing but whitespace or digits. */
	w = (char *)rf->buffer();
	we = w + rf->length();
	while (w < we) {
		if (*w && !isspace(*w) && !isdigit(*w))
			goto LErr;
		++w;
	}

	// Now allocate the return buffer and fill it with the data.
	ret = NEW_ARRAY(char, nbytes);
	if (is_null(ret))
		goto LErr;

	w = ret;
	while (w < ret + nbytes) {
		/*
		   Each integer gives us 16 random bits. We could have asked for
		   integers in the range [0, (1 << 19) - 1] for a maximum of
		   19,000 bits (2,375 bytes) per request -- or done even wee better
		   than that by requesting [0, 1000000] plus arithmetic encoding -- 
		   but that's extra work for little gain.
		*/
		buf = advance_and_parse_integer(buf, &i);
		*(unsigned char*)w = (i & 0xFF);
		++w;
		i >>= 8;
		if (w >= ret + nbytes)
			break;
		*(unsigned char*)w = (i & 0xFF);
		++w;
		if (*(buf) == 0)
			break;
	}

LErr:	
	delete rf;
	return(ret);
}

/*** Grab an array of random 32 bit unsigned ints of the specified size from random.org. ***/
u32* random_org_randu32(u32 count) {
	char* bytes;
	if (!is_between(count, 1, 5000))
		return(NULL);
	bytes = random_org_rand(count * 4);
	return (u32 *)bytes;
}

/*** Grab an array of (pure noise, zero information, truly) random 16 bit unsigned ints of the specified length from random.org. ***/
u16* random_org_randu16(u16 count) {
	char* bytes;
	if (!is_between(count, 1, 10000))
		return(NULL);
	bytes = random_org_rand(count * 2);
	return (u16 *)bytes;
}

#ifdef CODEHAPPY_NATIVE
/* RDRAND might have a backdoor, and random.org might keep logs of the random numbers it returns,
   but knowing both at once seems pretty unlikely. */
/* Return value:
	0: No seed took place. If you request random numbers from here, the default
	   (1024-bit) seeding will be done.
	1: One source was used. If hardware_rng_supported(), this must be the hardware
	   RNG, else it will be random.org.
	2: Both hardware RNG and random.org were successfully used in the seed. RandU32()
	   draws from ISAAC with a truly random 8192-bit seed, and RandU64() draws from
	   ISAAC64 with a random 16384-bit seed. The initial state is nowhere in RAM.
           Proceed with (some) confidence. */
u32 hardcore_seed(void) {
	u32 crandom = 0;
	/* By default, this value is 256. */
	constexpr u32 szseed = ISAAC_SEED_SZ_MAX / sizeof(u32);
	/* By default, this value is 512. */
	constexpr u32 szseed64 = ISAAC64_SEED_SZ_MAX / sizeof(u32);
	u32 seed[szseed64], cs = 0, ch = 0, e;
	u32* randomorg = nullptr, * hwrng = nullptr;
	BUILD_ASSERT(szseed64 >= szseed);

	if (__hardcore_init > 0) {
		ship_assert(__isaac_init);
		return __hardcore_init;
	}
	if (hardware_rng_supported()) {
		/* Always get szseed hardware RNs; we'll use them in both
		   seeds if random.org quota is used up, and if not, we'll still
		   use them in the ISAAC64 seed, which by default is longer. */
		hwrng = new u32 [szseed];
		NOT_NULL_OR_RETURN(hwrng, 0);
		for (e = 0; e < szseed; ++e) {
			hwrng[e] = hardware_rng();
		}
		crandom++;
	}
	/* Requires network and random.org quota, but a good source of entropy. */
	if (hwrng != nullptr)
		randomorg = random_org_randu32(szseed / 2);
	else
		randomorg = random_org_randu32(szseed);
	if (!is_null(randomorg))
		++crandom;

	/* if no hardware RNG or random.org numbers, bail. */
	if (0 == crandom)
		return crandom;

	/* Now interleave the values into the seed. */
	while (cs < szseed) {
		if (!is_null(hwrng))
			seed[cs++] = hwrng[ch];
		if (!is_null(randomorg))
			seed[cs++] = randomorg[ch];
		++ch;
	}

	/* ISAAC64 takes a longer (2x) buffer; we'll fill that first with 
	   the extra hardware RNGs, then the rest will be the first half 
	   values XORed with the low 32 bits of the hardware counter. */
	e = szseed;
	if (!is_null(hwrng)) {
		for (; e < szseed64 && ch < szseed; ++ch) {
			seed[e] = hwrng[ch];
		}
	}
	assert(szseed64 <= szseed + szseed);
	for (; e < szseed64; ++e) {
		u32 val = (u32)(hardware_counter() & 0xffffffffull);
		seed[e] = seed[e - szseed] ^ val;
	}

	isaac64_init(&__isaac64_state, (unsigned char*)seed, sizeof(u32) * szseed64);
	isaac_init(&__isaac_state, (unsigned char*)seed, sizeof(u32) * szseed);
	__isaac_init = true;
	__hardcore_init = crandom;

	/* Wreck the seed in memory. This also burns off a few random numbers, which 
	   while generally unnecessary with ISAAC, doesn't hurt to do. It does flush
           the initial state from the ISAAC context. After this point the initial seed
	   is (theoretically) unrecoverable. */
	bitblast((u8 *)(seed), sizeof(seed));
	if (!is_null(hwrng)) {
		bitblast((u8 *)(hwrng), sizeof(u32) * szseed);
		if (!is_null(randomorg)) {
			bitblast((u8 *)(randomorg), sizeof(u32) * (szseed / 2));
		}
	} else if (!is_null(randomorg)) {
		bitblast((u8 *)(randomorg), sizeof(u32) * szseed);
	}

	if (!is_null(hwrng))
		delete [] hwrng;
	if (!is_null(randomorg))
		delete [] randomorg;

	return crandom;
}
#endif  // CODEHAPPY_NATIVE


void bitblast(u8* buf, u32 sizeof_buf) {
	/* Overwrite memory with garbage. Exercises both the 32 bit and the 64 bit RNG. */
	u32 v1;
	u64 v2;
	u8* bufe = buf + sizeof_buf;
#define BLAST_BYTE(x)	{ *buf = u8(x & 0xff); ++buf; if (buf >= bufe) break; x >>= 8; }
	forever {
		v1 = RandU32();
		v2 = RandU64();

		BLAST_BYTE(v1);
		BLAST_BYTE(v2);
		BLAST_BYTE(v2);
		BLAST_BYTE(v2);
		BLAST_BYTE(v1);
		BLAST_BYTE(v2);
		BLAST_BYTE(v2);
		BLAST_BYTE(v1);
		BLAST_BYTE(v2);
		BLAST_BYTE(v2);
		BLAST_BYTE(v2);
		BLAST_BYTE(v1);
	}
// in case we want to define this fine identifier again
#undef BLAST_BYTE
}

namespace Dice {

u32 d(u32 n) {
	return RandU32Range(1, n);
}

u32 dd(u32 d, u32 n) {
	u32 acc = 0U;
	for (u32 e = 0; e < d; ++e) {
		acc += Dice::d(n);
	}
	return acc;
}

}  // namespace Dice

/***

	Historical PRNGs (they're mostly linear congruence generators) that, besides
	the usual flaws with LCGs, are known to have been in common use.

	DO NOT USE FOR PRODUCTION WORK, MONTE CARLO SIMULATION, OR EXPERIMENTS,
	if the quality of your PRNG output is of any concern at all. Try using
	the library's hardware entropy functions if your chip supports them, or one
	of the cryptographically strong software PRNGs included in the library.
	Take a look at the fancy seed generator functions as well; they are handy
	and tap into many unpredictable sources for seedy goodness.

	All of these functions are thread-safe; you pass them a pointer to the seed.

	What possible use can these weak PRNGs be, you ask? Here's a few ideas:

	* Emulation: to reproduce behavior in an interpreted language, systems
		environment, or operating system being emulated -- sometimes you
		really do need to generate crappy psuedo random numbers in order
		to make emulated programs work as intended

	* Reverse engineering: you'd be surprised how many modern applications
		use the example PRNG included in K&R (even though it only gives 16
		bit outputs!), and though everybody has known for 30+ years that
		RANDU is not a good PRNG, I wouldn't be surprised if even today there
		are a few apps floating around that are still using it. PRNGs from standard
		libraries of several compilers are included here as well, as these will
		turn up with frequency.

	* Simple historical interest or research.

	C. M. Street

***/

/*** for perfect compatibility with the LCGs shown below, some default initial seeds are given here. ***/
u32 k_and_r_lcg_default_seed = 1UL;
u32 randu_lcg_default_seed = 1UL;

/*** the infamous RANDU() ***/
u32 RANDU(u32* seed) {
	*seed = (*seed * 65539UL);
	*seed &= 0x7FFFFFFF;
	return(*seed);
}

/*** The example LCG given in K&R and the C language standard. At one time this was used as the stdlib
	implementation by Watcom, Digital Mars, and in many other compilers big and small. ***/
/* (Because, hey, what better way to ensure standards compliance than using the
	example implementation given in the standard, right? Even when it's pretty terrible
	and only gives you 16 bits?) */
u32 k_and_r_example_rand(u32* seed) {
	*seed = (*seed * 1103515245UL + 12345UL);
	*seed &= 0x7FFFFFFF;
	*seed >>= 16;
	return(*seed);
}

/*** rand() as implemented in glibc... this is just the K&R rand() function, but keeps all bits except the MSB. ***/
/* (32 bits are better than 16, sure, but keep in mind the low bits are not gonna be random at all.) */
u32 glibc_rand(u32* seed) {
	*seed = (*seed * 1103515245UL + 12345UL);
	*seed &= 0x7FFFFFFF;
	return(*seed);
}

/*** rand() as implemented in the Borland C/C++ stdlib ***/
u32 borland_c_lcg_rand(u32* seed) {
	*seed = (*seed * 22695477UL + 1);
	*seed &= 0x7FFFFFFF;
	return(*seed);
}

/*** Random(L) LCG implemented in Borland Delphi and Virtual Pascal ***/
u32 delphi_rand(u32* seed, u32 L) {
	u64 calc;
	*seed = (*seed * 134775813ULL + 1UL);
	calc = (u64)seed * L;
	calc >>= 32ULL;
	calc &= 0x7FFFFFFFULL;
	return(calc);
}

/*** rand() as implemented in Microsoft Quick C and Microsoft Visual C/C++ ***/
/* Just as bad as the K&R example in giving us 16 bits, but at least they chose a different multiplier. */
u32 microsoft_c_lcg_rand(u32* seed) {
	*seed = (*seed * 214013UL + 2531011);
	*seed &= 0x7FFFFFFF;
	*seed >>= 16;
	return(*seed);
}

/*** PRNG used in Microsoft Visual Basic, versions 6 and older ***/
u32 microsoft_visual_basic_lcg(u32* seed) {
	*seed = (*seed * 1140671485UL + 12820163);
	*seed &= ((1UL << 24) - 1);
	return(*seed);
}

/*** PRNG used in Native API (RtlUniform) ***/
u32 native_api_lcg_RTLUniform(u32* seed) {
	*seed = (*seed * 2147483629UL + 2147483587);
	*seed %= ((1UL << 31) - 1);
	return(*seed);
}

/*** PRNG used in C++11's minstd_rand0, also Apple's CarbonLib ***/
u32 minstd_rand0_lcg_rand(u32* seed) {
	*seed = (*seed * 16807UL);
	*seed %= ((1UL << 31) - 1);
	return(*seed);
}

/*** PRNG used in C++11's minstd_rand ***/
u32 minstd_rand_lcg_rand(u32* seed) {
	*seed = (*seed * 48271UL);
	*seed %= ((1UL << 31) - 1);
	return(*seed);
}

/*** rand() as implemented in older versions of glibc, or in VMS --
	this is another common old PRNG ***/
u32 glibc_old_vms_rand(u32* seed) {
	*seed = (*seed * 69069UL + 1UL);
	return(*seed);
}

/*** MMIX per Dr. Knuth -- for a basic LCG, this is about as good as it gets ***/
u64 knuth_mmix_lcg(u64* seed) {
	*seed = (*seed * 6364136223846793005ULL + 1442695040888963407ULL);
	return(*seed);
}

/*** rand() as implemented in the MUSL C library, and Newlib -- based on Knuth's MMIX above ***/
u32 musl_newlib_lcg_rand(u32* seed) {
	u64 calc;
	calc = ((u64)*seed * 6364136223846793005ULL + 1ULL);
	calc >>= 32ULL;
	calc &= 0x7FFFFFFFULL;
	*seed = (u32)calc;
	return(*seed);
}

/*** Java's java.util.random -- treating these as unsigned, though in Java they'd be signed, because Java ***/
u32 java_util_random_lcg(u32* seed) {
	u64 calc;
	calc = ((u64)*seed * 25214903917ULL + 11ULL);	/* eh, let's not use 1... how about 11? */
	calc >>= 16UL;
	calc &= 0xFFFFFFFFULL;
	*seed = (u32)calc;
	return(*seed);
}

/*** An LCG that I found in some old 8088 code in a AWK port to MS-DOS. Interesting
	in that it's better quality than most PRNGs around on micros in 1988. The original code
	is credited to Rob Duff, so we'll call it duff_lcg(). ***/
u64 duff_lcg(u64* seed) {
	*seed = (*seed * 0x0945A4E35B2769F8DULL) + 1;
	return(*seed);
}

/*** An additive congruential generator found on an old CUG disk. ***/
u32 acg_rand(void) {
	static u32 arg[] = {
          4292,     60,     4947,     3972,     4489,
          1917,     3916,   7579,     3048,     6856,
          1832,     7589,   1798,     4954,     2880,
          5142,     5187,   3045,     1529,     3110,
          4333,     167,    5556,     7237,     5906,
          5419,     6632,   5833,     3760,     1081,
          1434,     80,     6212,     344,      7303,
          3044,     7675,   5420,     457,      3434,
          2657,     700,    6777,     4436,     620,
          2129,     629,    3550,     1639,     4546,
          1220,     6469,   862,      3280,     4664
     };
	static int rp1 = 0, rp2 = 32;

     rp1++; 
     rp2++;  
     rp1 %= 55; 
     rp2 %= 55;
     arg[rp1] ^= arg[rp2];
     return ( arg[rp1] );
}

/*** A floating point PRNG -- returns a value in [0, 1] -- found on another CUG disk. ***/
float jjb_frand(void) {
	static u64 a;
	static int first = 1;
	
	if (first) {
		time((time_t *)&a);
		first = 0;
	}
	
	a = (a * 125) % 2796203;
	return ((float) a / 2796203);
}

/*** Deterministic random number sequences: objects that produce a stream of PRNs from
     a known provided seed. ***/

DetRand::DetRand(u32 init_seed) {
	is = (void*)new isaac_ctx;
	reseed(init_seed);
}

DetRand::DetRand(void* seed_data, u32 seed_len) {
	is = (void*)new isaac_ctx;
	reseed(seed_data, seed_len);
}

DetRand::~DetRand() {
	delete (isaac_ctx*)is;
	is = nullptr;
}

void DetRand::reseed(u32 new_seed) {
	seedi = new_seed;
	seed = &seedi;
	slen = sizeof(u32);
	reset();
}

void DetRand::reseed(void* new_seed, u32 new_seed_len) {
	seedi = 0;
	seed = new_seed;
	slen = new_seed_len;
	reset();
}

void DetRand::reset() {
	isaac_init((isaac_ctx*)is, (unsigned char*)(seed), slen);
	phase = 0;
}

u32 DetRand::RandU32() {
	return isaac_next_uint32((isaac_ctx*)is);
}

u32 DetRand::RandU32Range(u32 min_val, u32 max_val) {
	u32 rnd, resid, multiple, diffm;
	SORT2(min_val, max_val, u32);
	if (min_val == max_val)
		return max_val;
	diffm = max_val - min_val + 1;

	do {
		rnd = RandU32();
		resid = rnd % diffm;
		multiple = rnd - resid;
	} while (((multiple + diffm - 1) & 0xFFFFFFFFUL) < multiple);

	return min_val + resid;
}

u64 DetRand::RandU64() {
	u64 u;
	u = u64(isaac_next_uint32((isaac_ctx*)is));
	u |= (u64(isaac_next_uint32((isaac_ctx*)is)) << 32);
	return u;
}

u64 DetRand::RandU64Range(u64 min_val, u32 max_val) {
	u64 rnd, resid, multiple, diffm;
	SORT2(min_val, max_val, u64);
	if (min_val == max_val)
		return max_val;
	diffm = max_val - min_val + 1;

	do {
		rnd = RandU64();
		resid = rnd % diffm;
		multiple = rnd - resid;
	} while ((multiple + diffm - 1) < multiple);

	return min_val + resid;
}

float DetRand::randf1() {
	u32 u = RandU32();
	float vr = float(u);
	vr /= float(0xfffffffful);
	return vr;
}

double DetRand::randd1() {
	u64 u = RandU64();
	double vr = double(u);	// w?
	vr /= double(0xffffffffffffffffull);
	return vr;
}

float DetRand::randf(float min_range, float max_range) {
	SORT2(min_range, max_range, float);
	return min_range + randf1() * (max_range - min_range);
}

double DetRand::randd(double min_range, double max_range) {
	SORT2(min_range, max_range, double);
	return min_range + randd1() * (max_range - min_range);
}

float DetRand::normalf() {
	float ret;

	if (iszero(phase)) {
		do {
			float U1 = randf1();
			float U2 = randf1();

			V1 = 2. * U1 - 1.;
			V2 = 2. * U2 - 1.;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1. || S == 0.);
		ret = V1 * sqrt(-2. * log(S) / S);
	} else
		ret = V2 * sqrt(-2. * log(S) / S);
	phase = 1 - phase;
	
	return(ret);
}

double DetRand::normald() {
	double ret;

	if (iszero(phase)) {
		do {
			double U1 = randd1();
			double U2 = randd1();

			V1 = 2. * U1 - 1.;
			V2 = 2. * U2 - 1.;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1. || S == 0.);
		ret = V1 * sqrt(-2. * log(S) / S);
	} else
		ret = V2 * sqrt(-2. * log(S) / S);
	phase = 1 - phase;
	
	return(ret);
}

float DetRand::normalf(float M, float S) {
	return M + S * normalf();
}

double DetRand::normald(double M, double S) {
	return M + S * normald();
}

void rand_unit_weight_vector(u32 N, std::vector<double>& wts_out) {
	u32 e;
	double sum = 0.0;
	wts_out.clear();
	if (0 == N)
		return;
	for (e = 0; e < N; ++e) {
		double r = RandDouble(0.0, 1.0);
		wts_out.push_back(r);
		sum += r;
	}
	if (0. == sum) {
		wts_out[0] = 1.;
	} else {
		for (e = 0; e < N; ++e)
			wts_out[e] /= sum;
	}
}

/* end rand.cpp */
