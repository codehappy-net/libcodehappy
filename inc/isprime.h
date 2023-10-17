/***

	isprime.h

	Super fast functions isprime() and prime_factor(),
	for 32-bit integer input.

	Copyright (c) 2014-2022 C. M. Street

***/
#ifndef ISPRIME_H
#define ISPRIME_H

/***

	int isprime(uint32_t i)

	Returns 1 iff i is prime.
	If i is composite, 0, or 1, returns 0.

***/
extern int isprime(uint32_t i);


/***

	int prime_factor(uint32_t i)

	Returns the smallest prime factor of a composite i.
	If i is a prime number, returns i.
	If i is 0 or 1, returns i.

***/
extern uint32_t prime_factor(uint32_t i);

/***

	int isprime_small(uint32_t i)

	Returns 1 iff i is prime.
	If i is composite, 0, or 1, returns 0.

***/
extern int isprime_small(uint32_t i);


/***

	int prime_factor_small(uint32_t i)

	Returns the smallest prime factor of a composite i.
	If i is a prime number, returns i.
	If i is 0 or 1, returns i.

***/
extern uint32_t prime_factor_small(uint32_t i);

/***

	uint32_t pi_function_small(uint32_t i)

	Returns the count of prime integers smaller than
	or equal to i.

***/
extern uint32_t pi_function_small(uint32_t i);

/***

	uint32_t nth_prime_small(uint32_t i)

	Returns the nth prime, or 0 if it does not fit in 32 bits.
	For the purposes of this function, nth_prime(0) = 0.

***/
extern uint32_t nth_prime_small(uint32_t i);

/***

	uint32_t next_prime_small(uint32_t i)

	Returns the next prime strictly greater than i, or 0 if it
	does not fit in 32 bits.

***/
extern uint32_t next_prime_small(uint32_t i);

/***

	uint32_t previous_prime_small(uint32_t i)

	Returns the next prime strictly lower than i, or 0 if it does
	not exist (i.e., i is 2 or less.)

***/
extern uint32_t previous_prime_small(uint32_t i);

#endif // ISPRIME_H