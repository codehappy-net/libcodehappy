/***

	pifunc.h

	The pi function.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef __PI_FUNC_H__
#define __PI_FUNC_H__

/* The pi function: pi(i) gives the count of primes less than or equal to that integer. */
extern u32 pi_function(u32 i);

/* Returns the i'th prime, i.e. the smallest n such that pi(n) == i. Returns 0 if the result does not fit in 32 bits. */
extern u32 nth_prime(u32 i);

/* Get the next largest or next smallest prime number. Returns 0 if the result does not fit in 32 bits. */
extern u32 next_prime(u32 i);
extern u32 previous_prime(u32 i);

#endif  // __PI_FUNC_H__
/* end pifunc.h */