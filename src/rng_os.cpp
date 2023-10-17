/***

	rng_os.cpp
	
	Platform-specific strong RNG functions.

	In a separate file since using these functions under Windows requires
	linking with advapi32.dll / libadvapi32.a.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifdef CODEHAPPY_WINDOWS
/* let's support calls to RtlGenRandom() under Windows */
#include <ntsecapi.h>
#endif

/*** Do we have access to a cryptographically secure RNG on this platform? ***/
bool os_crypto_rng_supported(void) {
#ifdef CODEHAPPY_WINDOWS
	return(true);
#else
	// TODO: Under Linux or other UNIX-like environments, we should be able to access dev/random or dev/urandom for goodies
	return(false);
#endif
}

/*** Get a random number from the platform's cryptographically secure RNG -- check
	that this is supported with os_crypto_rng_supported() first ***/
/*** Under Windows, this means we need to link with advapi32.dll / libadvapi32.a ***/
u32 os_crypto_rng(void) {
#ifdef CODEHAPPY_WINDOWS
	unsigned long buf[1];
	// TODO: check this, it is probably faster to read a bunch of random ULONGs into a buffer and return from those?
	RtlGenRandom(buf, sizeof(buf[0]));
	return((u32)buf[0]);
#else
	return(0UL);
#endif
}

/*** Generate a 32-bit random integer by the "best" available method. Will use hardware entropy sources or
	platform-specific cryptographic random number sources if available. ***/
u32 best_rand(void) {
	if (hardware_rng_supported())
		return (hardware_rng());
	if (os_crypto_rng_supported())
		return (os_crypto_rng());
	return(RandU32());
}

/*** end rng_os.cpp ***/