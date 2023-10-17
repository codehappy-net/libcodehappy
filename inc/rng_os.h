/***

	rng_os.h
	
	Platform-specific strong RNG functions.

	In a separate file since using these functions under Windows requires
	linking with advapi32.dll / libadvapi32.a.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef _RNG_OS_
#define _RNG_OS_

/*** Do we have access to a cryptographically secure RNG on this platform? ***/
extern bool os_crypto_rng_supported(void);

/*** Get a random number from the platform's cryptographically secure RNG -- check
	that this is supported with os_crypto_rng_supported() first ***/
/*** Under Windows, this means we need to link with advapi32.dll / libadvapi32.a ***/
extern u32 os_crypto_rng(void);

/*** Generate a 32-bit random integer by the "best" available method. Will use hardware entropy sources or
	platform-specific cryptographic random number sources if available. ***/
extern u32 best_rand(void);

#endif  // _RNG_OS_
/* end rng_os.h */