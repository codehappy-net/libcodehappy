/**
 * isaac - A fast, high-quality pseudo-random number generator.
 *
 * ISAAC (Indirect, Shift, Accumulate, Add, and Count) is the most advanced of
 *  a series of pseudo-random number generators designed by Robert J. Jenkins
 *  Jr. in 1996: http://www.burtleburtle.net/bob/rand/isaac.html
 * To quote:
 *   No efficient method is known for deducing their internal states.
 *   ISAAC requires an amortized 18.75 instructions to produce a 32-bit value.
 *   There are no cycles in ISAAC shorter than 2**40 values.
 *   The expected cycle length is 2**8295 values.
 *   ...
 *   ISAAC-64 generates a different sequence than ISAAC, but it uses the same
 *    principles.
 *   It uses 64-bit arithmetic.
 *   It generates a 64-bit result every 19 instructions.
 *   All cycles are at least 2**72 values, and the average cycle length is
 *    2**16583.
 * An additional, important comment from Bob Jenkins in 2006:
 *   Seeding a random number generator is essentially the same problem as
 *    encrypting the seed with a block cipher.
 *   ISAAC should be initialized with the encryption of the seed by some
 *    secure cipher.
 *   I've provided a seeding routine in my implementations, which nobody has
 *    broken so far, but I have less faith in that initialization routine than
 *    I have in ISAAC.
 *
 * A number of attacks on ISAAC have been published.
 * [Pudo01] can recover the entire internal state and has expected running time
 *  less than the square root of the number of states, or 2**4121 (4.67E+1240).
 * [Auma06] reveals a large set of weak states, consisting of those for which
 *  the first value is repeated one or more times elsewhere in the state
 *  vector.
 * These induce a bias in the output relative to the repeated value.
 * The seed values used as input below are scrambled before being used, so any
 *  duplicates in them do not imply duplicates in the resulting internal state,
 *  however the chances of some duplicate existing elsewhere in a random state
 *  are just over 255/2**32, or merely 1 in 16 million.
 * Such states are, of course, much rarer in ISAAC-64.
 * It is not clear if an attacker can tell from just the output if ISAAC is in
 *  a weak state, or deduce the full internal state in any case except that
 *  where all or almost all of the entries in the state vector are identical.
 *   @MISC{Pudo01,
 *     author="Marina Pudovkina",
 *     title="A Known Plaintext Attack on the {ISAAC} Keystream Generator",
 *     howpublished="Cryptology ePrint Archive, Report 2001/049",
 *     year=2001,
 *     note="\url{http://eprint.iacr.org/2001/049}",
 *   }
 *   @MISC{Auma06,
 *     author="Jean-Philippe Aumasson",
 *     title="On the Pseudo-Random Generator {ISAAC}",
 *     howpublished="Cryptology ePrint Archive, Report 2006/438",
 *     year=2006,
 *     note="\url{http://eprint.iacr.org/2006/438}",
 *   }
 *
 * Even if one does not trust the security of this PRNG (and, without a good
 *  source of entropy to seed it, one should not), ISAAC is an excellent source
 *  of high-quality random numbers for Monte Carlo simulations, etc.
 * It is the fastest 32-bit generator among all of those that pass the
 *  statistical tests in the recent survey
 *  http://www.iro.umontreal.ca/~simardr/testu01/tu01.html, with the exception
 *  of Marsa-LFIB4, and it is quite competitive on 64-bit archtectures.
 * Unlike Marsa-LFIB4 (and all other LFib generators), there are no linear
 *  dependencies between successive values, and unlike many generators found in
 *  libc implementations, there are no small periods in the least significant
 *  bits, or seeds which lead to very small periods in general.
 *
 * Example:
 *  #include <stdio.h>
 *  #include <time.h>
 *  #include <ccan/isaac/isaac.h>
 *
 *  int main(void){
 *    static const char *CHEESE[3]={"Cheddar","Provolone","Camembert"};
 *    isaac_ctx     isaac;
 *    unsigned char seed[8];
 *    time_t        now;
 *    int           i;
 *    //N.B.: time() is not a good source of entropy.
 *    //Do not use it for cryptogrpahic purposes.
 *    time(&now);
 *    //Print it out so we can reproduce problems if needed.
 *    printf("Seed: 0x%016llX\n",(long long)now);
 *    //And convert the time to a byte array so that we can reproduce the same
 *    // seed on platforms with different endianesses.
 *    for(i=0;i<8;i++){
 *      seed[i]=(unsigned char)(now&0xFF);
 *      now>>=8;
 *    }
 *    isaac_init(&isaac,seed,8);
 *    printf("0x%08lX\n",(long)isaac_next_uint32(&isaac));
 *    printf("%s\n",CHEESE[isaac_next_uint(&isaac,3)]);
 *    printf("%0.8G\n",isaac_next_float(&isaac));
 *    printf("%0.8G\n",isaac_next_signed_float(&isaac));
 *    printf("%0.18G\n",isaac_next_double(&isaac));
 *    printf("%0.18G\n",isaac_next_signed_double(&isaac));
 *    return 0;
 *  }
 *
 * License: CC0 (Public domain)
 */

/* CC0 (Public domain) - see LICENSE file for details */
#if !defined(_isaac_H)
# define _isaac_H (1)
# include <stdint.h>



typedef struct isaac_ctx isaac_ctx;



/*This value may be lowered to reduce memory usage on embedded platforms, at
   the cost of reducing security and increasing bias.
  Quoting Bob Jenkins: "The current best guess is that bias is detectable after
   2**37 values for [ISAAC_SZ_LOG]=3, 2**45 for 4, 2**53 for 5, 2**61 for 6,
   2**69 for 7, and 2**77 values for [ISAAC_SZ_LOG]=8."*/
#define ISAAC_SZ_LOG      (8)
#define ISAAC_SZ          (1<<ISAAC_SZ_LOG)
#define ISAAC_SEED_SZ_MAX (ISAAC_SZ<<2)



/*ISAAC is the most advanced of a series of pseudo-random number generators
   designed by Robert J. Jenkins Jr. in 1996.
  http://www.burtleburtle.net/bob/rand/isaac.html
  To quote:
    No efficient method is known for deducing their internal states.
    ISAAC requires an amortized 18.75 instructions to produce a 32-bit value.
    There are no cycles in ISAAC shorter than 2**40 values.
    The expected cycle length is 2**8295 values.*/
struct isaac_ctx{
  unsigned n;
  uint32_t r[ISAAC_SZ];
  uint32_t m[ISAAC_SZ];
  uint32_t a;
  uint32_t b;
  uint32_t c;
};


/**
 * isaac_init - Initialize an instance of the ISAAC random number generator.
 * @_ctx:   The instance to initialize.
 * @_seed:  The specified seed bytes.
 *          This may be NULL if _nseed is less than or equal to zero.
 * @_nseed: The number of bytes to use for the seed.
 *          If this is greater than ISAAC_SEED_SZ_MAX, the extra bytes are
 *           ignored.
 */
void isaac_init(isaac_ctx *_ctx,const unsigned char *_seed,int _nseed);

/**
 * isaac_reseed - Mix a new batch of entropy into the current state.
 * To reset ISAAC to a known state, call isaac_init() again instead.
 * @_ctx:   The instance to reseed.
 * @_seed:  The specified seed bytes.
 *          This may be NULL if _nseed is zero.
 * @_nseed: The number of bytes to use for the seed.
 *          If this is greater than ISAAC_SEED_SZ_MAX, the extra bytes are
 *           ignored.
 */
void isaac_reseed(isaac_ctx *_ctx,const unsigned char *_seed,int _nseed);
/**
 * isaac_next_uint32 - Return the next random 32-bit value.
 * @_ctx: The ISAAC instance to generate the value with.
 */
uint32_t isaac_next_uint32(isaac_ctx *_ctx);
/**
 * isaac_next_uint - Uniform random integer less than the given value.
 * @_ctx: The ISAAC instance to generate the value with.
 * @_n:   The upper bound on the range of numbers returned (not inclusive).
 *        This must be greater than zero and less than 2**32.
 *        To return integers in the full range 0...2**32-1, use
 *         isaac_next_uint32() instead.
 * Return: An integer uniformly distributed between 0 and _n-1 (inclusive).
 */
uint32_t isaac_next_uint(isaac_ctx *_ctx,uint32_t _n);
/**
 * isaac_next_float - Uniform random float in the range [0,1).
 * @_ctx: The ISAAC instance to generate the value with.
 * Returns a high-quality float uniformly distributed between 0 (inclusive)
 *  and 1 (exclusive).
 * All of the float's mantissa bits are random, e.g., the least significant bit
 *  may still be non-zero even if the value is less than 0.5, and any
 *  representable float in the range [0,1) has a chance to be returned, though
 *  values very close to zero become increasingly unlikely.
 * To generate cheaper float values that do not have these properties, use
 *   ldexpf((float)isaac_next_uint32(_ctx),-32);
 */
float isaac_next_float(isaac_ctx *_ctx);
/**
 * isaac_next_signed_float - Uniform random float in the range (-1,1).
 * @_ctx: The ISAAC instance to generate the value with.
 * Returns a high-quality float uniformly distributed between -1 and 1
 *  (exclusive).
 * All of the float's mantissa bits are random, e.g., the least significant bit
 *  may still be non-zero even if the magnitude is less than 0.5, and any
 *  representable float in the range (-1,1) has a chance to be returned, though
 *  values very close to zero become increasingly unlikely.
 * To generate cheaper float values that do not have these properties, use
 *   ldexpf((float)isaac_next_uint32(_ctx),-31)-1;
 *  though this returns values in the range [-1,1).
 */
float isaac_next_signed_float(isaac_ctx *_ctx);
/**
 * isaac_next_double - Uniform random double in the range [0,1).
 * @_ctx: The ISAAC instance to generate the value with.
 * Returns a high-quality double uniformly distributed between 0 (inclusive)
 *  and 1 (exclusive).
 * All of the double's mantissa bits are random, e.g., the least significant
 *  bit may still be non-zero even if the value is less than 0.5, and any
 *  representable double in the range [0,1) has a chance to be returned, though
 *  values very close to zero become increasingly unlikely.
 * To generate cheaper double values that do not have these properties, use
 *   ldexp((double)isaac_next_uint32(_ctx),-32);
 */
double isaac_next_double(isaac_ctx *_ctx);
/**
 * isaac_next_signed_double - Uniform random double in the range (-1,1).
 * @_ctx: The ISAAC instance to generate the value with.
 * Returns a high-quality double uniformly distributed between -1 and 1
 *  (exclusive).
 * All of the double's mantissa bits are random, e.g., the least significant
 *  bit may still be non-zero even if the value is less than 0.5, and any
 *  representable double in the range (-1,1) has a chance to be returned,
 *  though values very close to zero become increasingly unlikely.
 * To generate cheaper double values that do not have these properties, use
 *   ldexp((double)isaac_next_uint32(_ctx),-31)-1;
 *  though this returns values in the range [-1,1).
 */
double isaac_next_signed_double(isaac_ctx *_ctx);

#endif
