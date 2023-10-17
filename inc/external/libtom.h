/****

	libtom.h

	An aggregation of the tommath and tomfloat arbitrary-precision integer and
	floating point math libraries.

****/
#ifndef __LIBTOM__
#define __LIBTOM__

/* LibTomMath, multiple-precision integer library -- Tom St Denis
 *
 * LibTomMath is a library that provides multiple-precision
 * integer arithmetic as well as number theoretic functionality.
 *
 * The library was designed directly after the MPI library by
 * Michael Fromberger but has been written from scratch with
 * additional optimizations in place.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://math.libtomcrypt.com
 */
#ifndef BN_H_
#define BN_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>

#if !(defined(LTM1) && defined(LTM2) && defined(LTM3))
#if defined(LTM2)
#define LTM3
#endif
#if defined(LTM1)
#define LTM2
#endif
#define LTM1

#if defined(LTM_ALL)
#define BN_ERROR_C
#define BN_FAST_MP_INVMOD_C
#define BN_FAST_MP_MONTGOMERY_REDUCE_C
#define BN_FAST_S_MP_MUL_DIGS_C
#define BN_FAST_S_MP_MUL_HIGH_DIGS_C
#define BN_FAST_S_MP_SQR_C
#define BN_MP_2EXPT_C
#define BN_MP_ABS_C
#define BN_MP_ADD_C
#define BN_MP_ADD_D_C
#define BN_MP_ADDMOD_C
#define BN_MP_AND_C
#define BN_MP_CLAMP_C
#define BN_MP_CLEAR_C
#define BN_MP_CLEAR_MULTI_C
#define BN_MP_CMP_C
#define BN_MP_CMP_D_C
#define BN_MP_CMP_MAG_C
#define BN_MP_CNT_LSB_C
#define BN_MP_COPY_C
#define BN_MP_COUNT_BITS_C
#define BN_MP_DIV_C
#define BN_MP_DIV_2_C
#define BN_MP_DIV_2D_C
#define BN_MP_DIV_3_C
#define BN_MP_DIV_D_C
#define BN_MP_DR_IS_MODULUS_C
#define BN_MP_DR_REDUCE_C
#define BN_MP_DR_SETUP_C
#define BN_MP_EXCH_C
#define BN_MP_EXPT_D_C
#define BN_MP_EXPTMOD_C
#define BN_MP_EXPTMOD_FAST_C
#define BN_MP_EXTEUCLID_C
#define BN_MP_FREAD_C
#define BN_MP_FWRITE_C
#define BN_MP_GCD_C
#define BN_MP_GET_INT_C
#define BN_MP_GROW_C
#define BN_MP_INIT_C
#define BN_MP_INIT_COPY_C
#define BN_MP_INIT_MULTI_C
#define BN_MP_INIT_SET_C
#define BN_MP_INIT_SET_INT_C
#define BN_MP_INIT_SIZE_C
#define BN_MP_INVMOD_C
#define BN_MP_INVMOD_SLOW_C
#define BN_MP_IS_SQUARE_C
#define BN_MP_JACOBI_C
#define BN_MP_KARATSUBA_MUL_C
#define BN_MP_KARATSUBA_SQR_C
#define BN_MP_LCM_C
#define BN_MP_LSHD_C
#define BN_MP_MOD_C
#define BN_MP_MOD_2D_C
#define BN_MP_MOD_D_C
#define BN_MP_MONTGOMERY_CALC_NORMALIZATION_C
#define BN_MP_MONTGOMERY_REDUCE_C
#define BN_MP_MONTGOMERY_SETUP_C
#define BN_MP_MUL_C
#define BN_MP_MUL_2_C
#define BN_MP_MUL_2D_C
#define BN_MP_MUL_D_C
#define BN_MP_MULMOD_C
#define BN_MP_N_ROOT_C
#define BN_MP_NEG_C
#define BN_MP_OR_C
#define BN_MP_PRIME_FERMAT_C
#define BN_MP_PRIME_IS_DIVISIBLE_C
#define BN_MP_PRIME_IS_PRIME_C
#define BN_MP_PRIME_MILLER_RABIN_C
#define BN_MP_PRIME_NEXT_PRIME_C
#define BN_MP_PRIME_RABIN_MILLER_TRIALS_C
#define BN_MP_PRIME_RANDOM_EX_C
#define BN_MP_RADIX_SIZE_C
#define BN_MP_RADIX_SMAP_C
#define BN_MP_RAND_C
#define BN_MP_READ_RADIX_C
#define BN_MP_READ_SIGNED_BIN_C
#define BN_MP_READ_UNSIGNED_BIN_C
#define BN_MP_REDUCE_C
#define BN_MP_REDUCE_2K_C
#define BN_MP_REDUCE_2K_L_C
#define BN_MP_REDUCE_2K_SETUP_C
#define BN_MP_REDUCE_2K_SETUP_L_C
#define BN_MP_REDUCE_IS_2K_C
#define BN_MP_REDUCE_IS_2K_L_C
#define BN_MP_REDUCE_SETUP_C
#define BN_MP_RSHD_C
#define BN_MP_SET_C
#define BN_MP_SET_INT_C
#define BN_MP_SHRINK_C
#define BN_MP_SIGNED_BIN_SIZE_C
#define BN_MP_SQR_C
#define BN_MP_SQRMOD_C
#define BN_MP_SQRT_C
#define BN_MP_SUB_C
#define BN_MP_SUB_D_C
#define BN_MP_SUBMOD_C
#define BN_MP_TO_SIGNED_BIN_C
#define BN_MP_TO_SIGNED_BIN_N_C
#define BN_MP_TO_UNSIGNED_BIN_C
#define BN_MP_TO_UNSIGNED_BIN_N_C
#define BN_MP_TOOM_MUL_C
#define BN_MP_TOOM_SQR_C
#define BN_MP_TORADIX_C
#define BN_MP_TORADIX_N_C
#define BN_MP_UNSIGNED_BIN_SIZE_C
#define BN_MP_XOR_C
#define BN_MP_ZERO_C
#define BN_PRIME_TAB_C
#define BN_REVERSE_C
#define BN_S_MP_ADD_C
#define BN_S_MP_EXPTMOD_C
#define BN_S_MP_MUL_DIGS_C
#define BN_S_MP_MUL_HIGH_DIGS_C
#define BN_S_MP_SQR_C
#define BN_S_MP_SUB_C
#define BNCORE_C
#endif

#if defined(BN_ERROR_C)
   #define BN_MP_ERROR_TO_STRING_C
#endif

#if defined(BN_FAST_MP_INVMOD_C)
   #define BN_MP_ISEVEN_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_COPY_C
   #define BN_MP_MOD_C
   #define BN_MP_SET_C
   #define BN_MP_DIV_2_C
   #define BN_MP_ISODD_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_C
   #define BN_MP_ISZERO_C
   #define BN_MP_CMP_D_C
   #define BN_MP_ADD_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_FAST_MP_MONTGOMERY_REDUCE_C)
   #define BN_MP_GROW_C
   #define BN_MP_RSHD_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_FAST_S_MP_MUL_DIGS_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_FAST_S_MP_MUL_HIGH_DIGS_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_FAST_S_MP_SQR_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_2EXPT_C)
   #define BN_MP_ZERO_C
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_ABS_C)
   #define BN_MP_COPY_C
#endif

#if defined(BN_MP_ADD_C)
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_ADD_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_SUB_D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_ADDMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_ADD_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_AND_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_CLAMP_C)
#endif

#if defined(BN_MP_CLEAR_C)
#endif

#if defined(BN_MP_CLEAR_MULTI_C)
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_CMP_C)
   #define BN_MP_CMP_MAG_C
#endif

#if defined(BN_MP_CMP_D_C)
#endif

#if defined(BN_MP_CMP_MAG_C)
#endif

#if defined(BN_MP_CNT_LSB_C)
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_COPY_C)
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_COUNT_BITS_C)
#endif

#if defined(BN_MP_DIV_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_COPY_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_SET_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_ABS_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CMP_C
   #define BN_MP_SUB_C
   #define BN_MP_ADD_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_INIT_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_LSHD_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_D_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DIV_2_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_DIV_2D_C)
   #define BN_MP_COPY_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_RSHD_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_DIV_3_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DIV_D_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_COPY_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DR_IS_MODULUS_C)
#endif

#if defined(BN_MP_DR_REDUCE_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_DR_SETUP_C)
#endif

#if defined(BN_MP_EXCH_C)
#endif

#if defined(BN_MP_EXPT_D_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_SET_C
   #define BN_MP_SQR_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MUL_C
#endif

#if defined(BN_MP_EXPTMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_INVMOD_C
   #define BN_MP_CLEAR_C
   #define BN_MP_ABS_C
   #define BN_MP_CLEAR_MULTI_C
   #define BN_MP_REDUCE_IS_2K_L_C
   #define BN_S_MP_EXPTMOD_C
   #define BN_MP_DR_IS_MODULUS_C
   #define BN_MP_REDUCE_IS_2K_C
   #define BN_MP_ISODD_C
   #define BN_MP_EXPTMOD_FAST_C
#endif

#if defined(BN_MP_EXPTMOD_FAST_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MONTGOMERY_SETUP_C
   #define BN_FAST_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_DR_SETUP_C
   #define BN_MP_DR_REDUCE_C
   #define BN_MP_REDUCE_2K_SETUP_C
   #define BN_MP_REDUCE_2K_C
   #define BN_MP_MONTGOMERY_CALC_NORMALIZATION_C
   #define BN_MP_MULMOD_C
   #define BN_MP_SET_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_EXTEUCLID_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_SET_C
   #define BN_MP_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_C
   #define BN_MP_MUL_C
   #define BN_MP_SUB_C
   #define BN_MP_NEG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_FREAD_C)
   #define BN_MP_ZERO_C
   #define BN_MP_S_RMAP_C
   #define BN_MP_MUL_D_C
   #define BN_MP_ADD_D_C
   #define BN_MP_CMP_D_C
#endif

#if defined(BN_MP_FWRITE_C)
   #define BN_MP_RADIX_SIZE_C
   #define BN_MP_TORADIX_C
#endif

#if defined(BN_MP_GCD_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_ABS_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_S_MP_SUB_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_GET_INT_C)
#endif

#if defined(BN_MP_GROW_C)
#endif

#if defined(BN_MP_INIT_C)
#endif

#if defined(BN_MP_INIT_COPY_C)
   #define BN_MP_COPY_C
#endif

#if defined(BN_MP_INIT_MULTI_C)
   #define BN_MP_ERR_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_INIT_SET_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
#endif

#if defined(BN_MP_INIT_SET_INT_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_INT_C
#endif

#if defined(BN_MP_INIT_SIZE_C)
   #define BN_MP_INIT_C
#endif

#if defined(BN_MP_INVMOD_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_ISODD_C
   #define BN_FAST_MP_INVMOD_C
   #define BN_MP_INVMOD_SLOW_C
#endif

#if defined(BN_MP_INVMOD_SLOW_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_ISEVEN_C
   #define BN_MP_SET_C
   #define BN_MP_DIV_2_C
   #define BN_MP_ISODD_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_C
   #define BN_MP_CMP_D_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_IS_SQUARE_C)
   #define BN_MP_MOD_D_C
   #define BN_MP_INIT_SET_INT_C
   #define BN_MP_MOD_C
   #define BN_MP_GET_INT_C
   #define BN_MP_SQRT_C
   #define BN_MP_SQR_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_JACOBI_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MOD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_KARATSUBA_MUL_C)
   #define BN_MP_MUL_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_SUB_C
   #define BN_MP_ADD_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_KARATSUBA_SQR_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_SQR_C
   #define BN_MP_SUB_C
   #define BN_S_MP_ADD_C
   #define BN_MP_LSHD_C
   #define BN_MP_ADD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_LCM_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_GCD_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_DIV_C
   #define BN_MP_MUL_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_LSHD_C)
   #define BN_MP_GROW_C
   #define BN_MP_RSHD_C
#endif

#if defined(BN_MP_MOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_DIV_C
   #define BN_MP_CLEAR_C
   #define BN_MP_ADD_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_MOD_2D_C)
   #define BN_MP_ZERO_C
   #define BN_MP_COPY_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MOD_D_C)
   #define BN_MP_DIV_D_C
#endif

#if defined(BN_MP_MONTGOMERY_CALC_NORMALIZATION_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_2EXPT_C
   #define BN_MP_SET_C
   #define BN_MP_MUL_2_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_MONTGOMERY_REDUCE_C)
   #define BN_FAST_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
   #define BN_MP_RSHD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_MONTGOMERY_SETUP_C)
#endif

#if defined(BN_MP_MUL_C)
   #define BN_MP_TOOM_MUL_C
   #define BN_MP_KARATSUBA_MUL_C
   #define BN_FAST_S_MP_MUL_DIGS_C
   #define BN_S_MP_MUL_C
   #define BN_S_MP_MUL_DIGS_C
#endif

#if defined(BN_MP_MUL_2_C)
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_MUL_2D_C)
   #define BN_MP_COPY_C
   #define BN_MP_GROW_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MUL_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MULMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_MUL_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_N_ROOT_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
   #define BN_MP_COPY_C
   #define BN_MP_EXPT_D_C
   #define BN_MP_MUL_C
   #define BN_MP_SUB_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_C
   #define BN_MP_CMP_C
   #define BN_MP_SUB_D_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_NEG_C)
   #define BN_MP_COPY_C
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_OR_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_FERMAT_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_INIT_C
   #define BN_MP_EXPTMOD_C
   #define BN_MP_CMP_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_IS_DIVISIBLE_C)
   #define BN_MP_MOD_D_C
#endif

#if defined(BN_MP_PRIME_IS_PRIME_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_PRIME_IS_DIVISIBLE_C
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
   #define BN_MP_PRIME_MILLER_RABIN_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_MILLER_RABIN_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_SUB_D_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_EXPTMOD_C
   #define BN_MP_CMP_C
   #define BN_MP_SQRMOD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_NEXT_PRIME_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_SET_C
   #define BN_MP_SUB_D_C
   #define BN_MP_ISEVEN_C
   #define BN_MP_MOD_D_C
   #define BN_MP_INIT_C
   #define BN_MP_ADD_D_C
   #define BN_MP_PRIME_MILLER_RABIN_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_RABIN_MILLER_TRIALS_C)
#endif

#if defined(BN_MP_PRIME_RANDOM_EX_C)
   #define BN_MP_READ_UNSIGNED_BIN_C
   #define BN_MP_PRIME_IS_PRIME_C
   #define BN_MP_SUB_D_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_D_C
#endif

#if defined(BN_MP_RADIX_SIZE_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_RADIX_SMAP_C)
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_RAND_C)
   #define BN_MP_ZERO_C
   #define BN_MP_ADD_D_C
   #define BN_MP_LSHD_C
#endif

#if defined(BN_MP_READ_RADIX_C)
   #define BN_MP_ZERO_C
   #define BN_MP_S_RMAP_C
   #define BN_MP_RADIX_SMAP_C
   #define BN_MP_MUL_D_C
   #define BN_MP_ADD_D_C
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_READ_SIGNED_BIN_C)
   #define BN_MP_READ_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_READ_UNSIGNED_BIN_C)
   #define BN_MP_GROW_C
   #define BN_MP_ZERO_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_REDUCE_C)
   #define BN_MP_REDUCE_SETUP_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_C
   #define BN_S_MP_MUL_HIGH_DIGS_C
   #define BN_FAST_S_MP_MUL_HIGH_DIGS_C
   #define BN_MP_MOD_2D_C
   #define BN_S_MP_MUL_DIGS_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_D_C
   #define BN_MP_SET_C
   #define BN_MP_LSHD_C
   #define BN_MP_ADD_C
   #define BN_MP_CMP_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MUL_D_C
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_L_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MUL_C
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_SETUP_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_2EXPT_C
   #define BN_MP_CLEAR_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_REDUCE_2K_SETUP_L_C)
   #define BN_MP_INIT_C
   #define BN_MP_2EXPT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_IS_2K_C)
   #define BN_MP_REDUCE_2K_C
   #define BN_MP_COUNT_BITS_C
#endif

#if defined(BN_MP_REDUCE_IS_2K_L_C)
#endif

#if defined(BN_MP_REDUCE_SETUP_C)
   #define BN_MP_2EXPT_C
   #define BN_MP_DIV_C
#endif

#if defined(BN_MP_RSHD_C)
   #define BN_MP_ZERO_C
#endif

#if defined(BN_MP_SET_C)
   #define BN_MP_ZERO_C
#endif

#if defined(BN_MP_SET_INT_C)
   #define BN_MP_ZERO_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_SHRINK_C)
#endif

#if defined(BN_MP_SIGNED_BIN_SIZE_C)
   #define BN_MP_UNSIGNED_BIN_SIZE_C
#endif

#if defined(BN_MP_SQR_C)
   #define BN_MP_TOOM_SQR_C
   #define BN_MP_KARATSUBA_SQR_C
   #define BN_FAST_S_MP_SQR_C
   #define BN_S_MP_SQR_C
#endif

#if defined(BN_MP_SQRMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_SQR_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_SQRT_C)
   #define BN_MP_N_ROOT_C
   #define BN_MP_ISZERO_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_DIV_C
   #define BN_MP_ADD_C
   #define BN_MP_DIV_2_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_SUB_C)
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_SUB_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_ADD_D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_SUBMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_SUB_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_TO_SIGNED_BIN_C)
   #define BN_MP_TO_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_TO_SIGNED_BIN_N_C)
   #define BN_MP_SIGNED_BIN_SIZE_C
   #define BN_MP_TO_SIGNED_BIN_C
#endif

#if defined(BN_MP_TO_UNSIGNED_BIN_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_TO_UNSIGNED_BIN_N_C)
   #define BN_MP_UNSIGNED_BIN_SIZE_C
   #define BN_MP_TO_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_TOOM_MUL_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_TOOM_SQR_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_TORADIX_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_TORADIX_N_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_UNSIGNED_BIN_SIZE_C)
   #define BN_MP_COUNT_BITS_C
#endif

#if defined(BN_MP_XOR_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_ZERO_C)
#endif

#if defined(BN_PRIME_TAB_C)
#endif

#if defined(BN_REVERSE_C)
#endif

#if defined(BN_S_MP_ADD_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_S_MP_EXPTMOD_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
   #define BN_MP_REDUCE_SETUP_C
   #define BN_MP_REDUCE_C
   #define BN_MP_REDUCE_2K_SETUP_L_C
   #define BN_MP_REDUCE_2K_L_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_C
   #define BN_MP_SET_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_S_MP_MUL_DIGS_C)
   #define BN_FAST_S_MP_MUL_DIGS_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_MUL_HIGH_DIGS_C)
   #define BN_FAST_S_MP_MUL_HIGH_DIGS_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_SQR_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_SUB_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BNCORE_C)
#endif

#ifdef LTM3
#define LTM_LAST
#endif
/* super class file for PK algos */

/* default ... include all MPI */
#define LTM_ALL

/* RSA only (does not support DH/DSA/ECC) */
/* #define SC_RSA_1 */

/* For reference.... On an Athlon64 optimizing for speed...

   LTM's mpi.o with all functions [striped] is 142KiB in size.

*/

/* Works for RSA only, mpi.o is 68KiB */
#ifdef SC_RSA_1
   #define BN_MP_SHRINK_C
   #define BN_MP_LCM_C
   #define BN_MP_PRIME_RANDOM_EX_C
   #define BN_MP_INVMOD_C
   #define BN_MP_GCD_C
   #define BN_MP_MOD_C
   #define BN_MP_MULMOD_C
   #define BN_MP_ADDMOD_C
   #define BN_MP_EXPTMOD_C
   #define BN_MP_SET_INT_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_CLEAR_MULTI_C
   #define BN_MP_UNSIGNED_BIN_SIZE_C
   #define BN_MP_TO_UNSIGNED_BIN_C
   #define BN_MP_MOD_D_C
   #define BN_MP_PRIME_RABIN_MILLER_TRIALS_C
   #define BN_REVERSE_C
   #define BN_PRIME_TAB_C

   /* other modifiers */
   #define BN_MP_DIV_SMALL                    /* Slower division, not critical */

   /* here we are on the last pass so we turn things off.  The functions classes are still there
    * but we remove them specifically from the build.  This also invokes tweaks in functions
    * like removing support for even moduli, etc...
    */
#ifdef LTM_LAST
   #undef  BN_MP_TOOM_MUL_C
   #undef  BN_MP_TOOM_SQR_C
   #undef  BN_MP_KARATSUBA_MUL_C
   #undef  BN_MP_KARATSUBA_SQR_C
   #undef  BN_MP_REDUCE_C
   #undef  BN_MP_REDUCE_SETUP_C
   #undef  BN_MP_DR_IS_MODULUS_C
   #undef  BN_MP_DR_SETUP_C
   #undef  BN_MP_DR_REDUCE_C
   #undef  BN_MP_REDUCE_IS_2K_C
   #undef  BN_MP_REDUCE_2K_SETUP_C
   #undef  BN_MP_REDUCE_2K_C
   #undef  BN_S_MP_EXPTMOD_C
   #undef  BN_MP_DIV_3_C
   #undef  BN_S_MP_MUL_HIGH_DIGS_C
   #undef  BN_FAST_S_MP_MUL_HIGH_DIGS_C
   #undef  BN_FAST_MP_INVMOD_C

   /* To safely undefine these you have to make sure your RSA key won't exceed the Comba threshold
    * which is roughly 255 digits [7140 bits for 32-bit machines, 15300 bits for 64-bit machines] 
    * which means roughly speaking you can handle upto 2536-bit RSA keys with these defined without
    * trouble.  
    */
   #undef  BN_S_MP_MUL_DIGS_C
   #undef  BN_S_MP_SQR_C
   #undef  BN_MP_MONTGOMERY_REDUCE_C
#endif

#endif

/* $Source$ */
/* $Revision: 0.36 $ */
/* $Date: 2005-08-01 16:37:28 +0000 $ */

#if !(defined(LTM1) && defined(LTM2) && defined(LTM3))
#if defined(LTM2)
#define LTM3
#endif
#if defined(LTM1)
#define LTM2
#endif
#define LTM1

#if defined(LTM_ALL)
#define BN_ERROR_C
#define BN_FAST_MP_INVMOD_C
#define BN_FAST_MP_MONTGOMERY_REDUCE_C
#define BN_FAST_S_MP_MUL_DIGS_C
#define BN_FAST_S_MP_MUL_HIGH_DIGS_C
#define BN_FAST_S_MP_SQR_C
#define BN_MP_2EXPT_C
#define BN_MP_ABS_C
#define BN_MP_ADD_C
#define BN_MP_ADD_D_C
#define BN_MP_ADDMOD_C
#define BN_MP_AND_C
#define BN_MP_CLAMP_C
#define BN_MP_CLEAR_C
#define BN_MP_CLEAR_MULTI_C
#define BN_MP_CMP_C
#define BN_MP_CMP_D_C
#define BN_MP_CMP_MAG_C
#define BN_MP_CNT_LSB_C
#define BN_MP_COPY_C
#define BN_MP_COUNT_BITS_C
#define BN_MP_DIV_C
#define BN_MP_DIV_2_C
#define BN_MP_DIV_2D_C
#define BN_MP_DIV_3_C
#define BN_MP_DIV_D_C
#define BN_MP_DR_IS_MODULUS_C
#define BN_MP_DR_REDUCE_C
#define BN_MP_DR_SETUP_C
#define BN_MP_EXCH_C
#define BN_MP_EXPT_D_C
#define BN_MP_EXPTMOD_C
#define BN_MP_EXPTMOD_FAST_C
#define BN_MP_EXTEUCLID_C
#define BN_MP_FREAD_C
#define BN_MP_FWRITE_C
#define BN_MP_GCD_C
#define BN_MP_GET_INT_C
#define BN_MP_GROW_C
#define BN_MP_INIT_C
#define BN_MP_INIT_COPY_C
#define BN_MP_INIT_MULTI_C
#define BN_MP_INIT_SET_C
#define BN_MP_INIT_SET_INT_C
#define BN_MP_INIT_SIZE_C
#define BN_MP_INVMOD_C
#define BN_MP_INVMOD_SLOW_C
#define BN_MP_IS_SQUARE_C
#define BN_MP_JACOBI_C
#define BN_MP_KARATSUBA_MUL_C
#define BN_MP_KARATSUBA_SQR_C
#define BN_MP_LCM_C
#define BN_MP_LSHD_C
#define BN_MP_MOD_C
#define BN_MP_MOD_2D_C
#define BN_MP_MOD_D_C
#define BN_MP_MONTGOMERY_CALC_NORMALIZATION_C
#define BN_MP_MONTGOMERY_REDUCE_C
#define BN_MP_MONTGOMERY_SETUP_C
#define BN_MP_MUL_C
#define BN_MP_MUL_2_C
#define BN_MP_MUL_2D_C
#define BN_MP_MUL_D_C
#define BN_MP_MULMOD_C
#define BN_MP_N_ROOT_C
#define BN_MP_NEG_C
#define BN_MP_OR_C
#define BN_MP_PRIME_FERMAT_C
#define BN_MP_PRIME_IS_DIVISIBLE_C
#define BN_MP_PRIME_IS_PRIME_C
#define BN_MP_PRIME_MILLER_RABIN_C
#define BN_MP_PRIME_NEXT_PRIME_C
#define BN_MP_PRIME_RABIN_MILLER_TRIALS_C
#define BN_MP_PRIME_RANDOM_EX_C
#define BN_MP_RADIX_SIZE_C
#define BN_MP_RADIX_SMAP_C
#define BN_MP_RAND_C
#define BN_MP_READ_RADIX_C
#define BN_MP_READ_SIGNED_BIN_C
#define BN_MP_READ_UNSIGNED_BIN_C
#define BN_MP_REDUCE_C
#define BN_MP_REDUCE_2K_C
#define BN_MP_REDUCE_2K_L_C
#define BN_MP_REDUCE_2K_SETUP_C
#define BN_MP_REDUCE_2K_SETUP_L_C
#define BN_MP_REDUCE_IS_2K_C
#define BN_MP_REDUCE_IS_2K_L_C
#define BN_MP_REDUCE_SETUP_C
#define BN_MP_RSHD_C
#define BN_MP_SET_C
#define BN_MP_SET_INT_C
#define BN_MP_SHRINK_C
#define BN_MP_SIGNED_BIN_SIZE_C
#define BN_MP_SQR_C
#define BN_MP_SQRMOD_C
#define BN_MP_SQRT_C
#define BN_MP_SUB_C
#define BN_MP_SUB_D_C
#define BN_MP_SUBMOD_C
#define BN_MP_TO_SIGNED_BIN_C
#define BN_MP_TO_SIGNED_BIN_N_C
#define BN_MP_TO_UNSIGNED_BIN_C
#define BN_MP_TO_UNSIGNED_BIN_N_C
#define BN_MP_TOOM_MUL_C
#define BN_MP_TOOM_SQR_C
#define BN_MP_TORADIX_C
#define BN_MP_TORADIX_N_C
#define BN_MP_UNSIGNED_BIN_SIZE_C
#define BN_MP_XOR_C
#define BN_MP_ZERO_C
#define BN_PRIME_TAB_C
#define BN_REVERSE_C
#define BN_S_MP_ADD_C
#define BN_S_MP_EXPTMOD_C
#define BN_S_MP_MUL_DIGS_C
#define BN_S_MP_MUL_HIGH_DIGS_C
#define BN_S_MP_SQR_C
#define BN_S_MP_SUB_C
#define BNCORE_C
#endif

#if defined(BN_ERROR_C)
   #define BN_MP_ERROR_TO_STRING_C
#endif

#if defined(BN_FAST_MP_INVMOD_C)
   #define BN_MP_ISEVEN_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_COPY_C
   #define BN_MP_MOD_C
   #define BN_MP_SET_C
   #define BN_MP_DIV_2_C
   #define BN_MP_ISODD_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_C
   #define BN_MP_ISZERO_C
   #define BN_MP_CMP_D_C
   #define BN_MP_ADD_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_FAST_MP_MONTGOMERY_REDUCE_C)
   #define BN_MP_GROW_C
   #define BN_MP_RSHD_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_FAST_S_MP_MUL_DIGS_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_FAST_S_MP_MUL_HIGH_DIGS_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_FAST_S_MP_SQR_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_2EXPT_C)
   #define BN_MP_ZERO_C
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_ABS_C)
   #define BN_MP_COPY_C
#endif

#if defined(BN_MP_ADD_C)
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_ADD_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_SUB_D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_ADDMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_ADD_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_AND_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_CLAMP_C)
#endif

#if defined(BN_MP_CLEAR_C)
#endif

#if defined(BN_MP_CLEAR_MULTI_C)
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_CMP_C)
   #define BN_MP_CMP_MAG_C
#endif

#if defined(BN_MP_CMP_D_C)
#endif

#if defined(BN_MP_CMP_MAG_C)
#endif

#if defined(BN_MP_CNT_LSB_C)
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_COPY_C)
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_COUNT_BITS_C)
#endif

#if defined(BN_MP_DIV_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_COPY_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_SET_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_ABS_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CMP_C
   #define BN_MP_SUB_C
   #define BN_MP_ADD_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_INIT_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_LSHD_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_D_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DIV_2_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_DIV_2D_C)
   #define BN_MP_COPY_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_RSHD_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_DIV_3_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DIV_D_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_COPY_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_DR_IS_MODULUS_C)
#endif

#if defined(BN_MP_DR_REDUCE_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_DR_SETUP_C)
#endif

#if defined(BN_MP_EXCH_C)
#endif

#if defined(BN_MP_EXPT_D_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_SET_C
   #define BN_MP_SQR_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MUL_C
#endif

#if defined(BN_MP_EXPTMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_INVMOD_C
   #define BN_MP_CLEAR_C
   #define BN_MP_ABS_C
   #define BN_MP_CLEAR_MULTI_C
   #define BN_MP_REDUCE_IS_2K_L_C
   #define BN_S_MP_EXPTMOD_C
   #define BN_MP_DR_IS_MODULUS_C
   #define BN_MP_REDUCE_IS_2K_C
   #define BN_MP_ISODD_C
   #define BN_MP_EXPTMOD_FAST_C
#endif

#if defined(BN_MP_EXPTMOD_FAST_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MONTGOMERY_SETUP_C
   #define BN_FAST_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_DR_SETUP_C
   #define BN_MP_DR_REDUCE_C
   #define BN_MP_REDUCE_2K_SETUP_C
   #define BN_MP_REDUCE_2K_C
   #define BN_MP_MONTGOMERY_CALC_NORMALIZATION_C
   #define BN_MP_MULMOD_C
   #define BN_MP_SET_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_EXTEUCLID_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_SET_C
   #define BN_MP_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_C
   #define BN_MP_MUL_C
   #define BN_MP_SUB_C
   #define BN_MP_NEG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_FREAD_C)
   #define BN_MP_ZERO_C
   #define BN_MP_S_RMAP_C
   #define BN_MP_MUL_D_C
   #define BN_MP_ADD_D_C
   #define BN_MP_CMP_D_C
#endif

#if defined(BN_MP_FWRITE_C)
   #define BN_MP_RADIX_SIZE_C
   #define BN_MP_TORADIX_C
#endif

#if defined(BN_MP_GCD_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_ABS_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_S_MP_SUB_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_GET_INT_C)
#endif

#if defined(BN_MP_GROW_C)
#endif

#if defined(BN_MP_INIT_C)
#endif

#if defined(BN_MP_INIT_COPY_C)
   #define BN_MP_COPY_C
#endif

#if defined(BN_MP_INIT_MULTI_C)
   #define BN_MP_ERR_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_INIT_SET_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
#endif

#if defined(BN_MP_INIT_SET_INT_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_INT_C
#endif

#if defined(BN_MP_INIT_SIZE_C)
   #define BN_MP_INIT_C
#endif

#if defined(BN_MP_INVMOD_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_ISODD_C
   #define BN_FAST_MP_INVMOD_C
   #define BN_MP_INVMOD_SLOW_C
#endif

#if defined(BN_MP_INVMOD_SLOW_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_ISEVEN_C
   #define BN_MP_SET_C
   #define BN_MP_DIV_2_C
   #define BN_MP_ISODD_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_C
   #define BN_MP_CMP_D_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_IS_SQUARE_C)
   #define BN_MP_MOD_D_C
   #define BN_MP_INIT_SET_INT_C
   #define BN_MP_MOD_C
   #define BN_MP_GET_INT_C
   #define BN_MP_SQRT_C
   #define BN_MP_SQR_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_JACOBI_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MOD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_KARATSUBA_MUL_C)
   #define BN_MP_MUL_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_SUB_C
   #define BN_MP_ADD_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_KARATSUBA_SQR_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_SQR_C
   #define BN_MP_SUB_C
   #define BN_S_MP_ADD_C
   #define BN_MP_LSHD_C
   #define BN_MP_ADD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_LCM_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_GCD_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_DIV_C
   #define BN_MP_MUL_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_LSHD_C)
   #define BN_MP_GROW_C
   #define BN_MP_RSHD_C
#endif

#if defined(BN_MP_MOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_DIV_C
   #define BN_MP_CLEAR_C
   #define BN_MP_ADD_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_MP_MOD_2D_C)
   #define BN_MP_ZERO_C
   #define BN_MP_COPY_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MOD_D_C)
   #define BN_MP_DIV_D_C
#endif

#if defined(BN_MP_MONTGOMERY_CALC_NORMALIZATION_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_2EXPT_C
   #define BN_MP_SET_C
   #define BN_MP_MUL_2_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_MONTGOMERY_REDUCE_C)
   #define BN_FAST_MP_MONTGOMERY_REDUCE_C
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
   #define BN_MP_RSHD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_MONTGOMERY_SETUP_C)
#endif

#if defined(BN_MP_MUL_C)
   #define BN_MP_TOOM_MUL_C
   #define BN_MP_KARATSUBA_MUL_C
   #define BN_FAST_S_MP_MUL_DIGS_C
   #define BN_S_MP_MUL_C
   #define BN_S_MP_MUL_DIGS_C
#endif

#if defined(BN_MP_MUL_2_C)
   #define BN_MP_GROW_C
#endif

#if defined(BN_MP_MUL_2D_C)
   #define BN_MP_COPY_C
   #define BN_MP_GROW_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MUL_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_MULMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_MUL_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_N_ROOT_C)
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
   #define BN_MP_COPY_C
   #define BN_MP_EXPT_D_C
   #define BN_MP_MUL_C
   #define BN_MP_SUB_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_C
   #define BN_MP_CMP_C
   #define BN_MP_SUB_D_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_NEG_C)
   #define BN_MP_COPY_C
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_OR_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_FERMAT_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_INIT_C
   #define BN_MP_EXPTMOD_C
   #define BN_MP_CMP_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_IS_DIVISIBLE_C)
   #define BN_MP_MOD_D_C
#endif

#if defined(BN_MP_PRIME_IS_PRIME_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_PRIME_IS_DIVISIBLE_C
   #define BN_MP_INIT_C
   #define BN_MP_SET_C
   #define BN_MP_PRIME_MILLER_RABIN_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_MILLER_RABIN_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_SUB_D_C
   #define BN_MP_CNT_LSB_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_EXPTMOD_C
   #define BN_MP_CMP_C
   #define BN_MP_SQRMOD_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_NEXT_PRIME_C)
   #define BN_MP_CMP_D_C
   #define BN_MP_SET_C
   #define BN_MP_SUB_D_C
   #define BN_MP_ISEVEN_C
   #define BN_MP_MOD_D_C
   #define BN_MP_INIT_C
   #define BN_MP_ADD_D_C
   #define BN_MP_PRIME_MILLER_RABIN_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_PRIME_RABIN_MILLER_TRIALS_C)
#endif

#if defined(BN_MP_PRIME_RANDOM_EX_C)
   #define BN_MP_READ_UNSIGNED_BIN_C
   #define BN_MP_PRIME_IS_PRIME_C
   #define BN_MP_SUB_D_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_D_C
#endif

#if defined(BN_MP_RADIX_SIZE_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_RADIX_SMAP_C)
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_RAND_C)
   #define BN_MP_ZERO_C
   #define BN_MP_ADD_D_C
   #define BN_MP_LSHD_C
#endif

#if defined(BN_MP_READ_RADIX_C)
   #define BN_MP_ZERO_C
   #define BN_MP_S_RMAP_C
   #define BN_MP_RADIX_SMAP_C
   #define BN_MP_MUL_D_C
   #define BN_MP_ADD_D_C
   #define BN_MP_ISZERO_C
#endif

#if defined(BN_MP_READ_SIGNED_BIN_C)
   #define BN_MP_READ_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_READ_UNSIGNED_BIN_C)
   #define BN_MP_GROW_C
   #define BN_MP_ZERO_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_REDUCE_C)
   #define BN_MP_REDUCE_SETUP_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_C
   #define BN_S_MP_MUL_HIGH_DIGS_C
   #define BN_FAST_S_MP_MUL_HIGH_DIGS_C
   #define BN_MP_MOD_2D_C
   #define BN_S_MP_MUL_DIGS_C
   #define BN_MP_SUB_C
   #define BN_MP_CMP_D_C
   #define BN_MP_SET_C
   #define BN_MP_LSHD_C
   #define BN_MP_ADD_C
   #define BN_MP_CMP_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MUL_D_C
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_L_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_MUL_C
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_2K_SETUP_C)
   #define BN_MP_INIT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_2EXPT_C
   #define BN_MP_CLEAR_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_REDUCE_2K_SETUP_L_C)
   #define BN_MP_INIT_C
   #define BN_MP_2EXPT_C
   #define BN_MP_COUNT_BITS_C
   #define BN_S_MP_SUB_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_REDUCE_IS_2K_C)
   #define BN_MP_REDUCE_2K_C
   #define BN_MP_COUNT_BITS_C
#endif

#if defined(BN_MP_REDUCE_IS_2K_L_C)
#endif

#if defined(BN_MP_REDUCE_SETUP_C)
   #define BN_MP_2EXPT_C
   #define BN_MP_DIV_C
#endif

#if defined(BN_MP_RSHD_C)
   #define BN_MP_ZERO_C
#endif

#if defined(BN_MP_SET_C)
   #define BN_MP_ZERO_C
#endif

#if defined(BN_MP_SET_INT_C)
   #define BN_MP_ZERO_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_SHRINK_C)
#endif

#if defined(BN_MP_SIGNED_BIN_SIZE_C)
   #define BN_MP_UNSIGNED_BIN_SIZE_C
#endif

#if defined(BN_MP_SQR_C)
   #define BN_MP_TOOM_SQR_C
   #define BN_MP_KARATSUBA_SQR_C
   #define BN_FAST_S_MP_SQR_C
   #define BN_S_MP_SQR_C
#endif

#if defined(BN_MP_SQRMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_SQR_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_SQRT_C)
   #define BN_MP_N_ROOT_C
   #define BN_MP_ISZERO_C
   #define BN_MP_ZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_DIV_C
   #define BN_MP_ADD_C
   #define BN_MP_DIV_2_C
   #define BN_MP_CMP_MAG_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_SUB_C)
   #define BN_S_MP_ADD_C
   #define BN_MP_CMP_MAG_C
   #define BN_S_MP_SUB_C
#endif

#if defined(BN_MP_SUB_D_C)
   #define BN_MP_GROW_C
   #define BN_MP_ADD_D_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_MP_SUBMOD_C)
   #define BN_MP_INIT_C
   #define BN_MP_SUB_C
   #define BN_MP_CLEAR_C
   #define BN_MP_MOD_C
#endif

#if defined(BN_MP_TO_SIGNED_BIN_C)
   #define BN_MP_TO_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_TO_SIGNED_BIN_N_C)
   #define BN_MP_SIGNED_BIN_SIZE_C
   #define BN_MP_TO_SIGNED_BIN_C
#endif

#if defined(BN_MP_TO_UNSIGNED_BIN_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_ISZERO_C
   #define BN_MP_DIV_2D_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_TO_UNSIGNED_BIN_N_C)
   #define BN_MP_UNSIGNED_BIN_SIZE_C
   #define BN_MP_TO_UNSIGNED_BIN_C
#endif

#if defined(BN_MP_TOOM_MUL_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_MUL_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_TOOM_SQR_C)
   #define BN_MP_INIT_MULTI_C
   #define BN_MP_MOD_2D_C
   #define BN_MP_COPY_C
   #define BN_MP_RSHD_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_2_C
   #define BN_MP_ADD_C
   #define BN_MP_SUB_C
   #define BN_MP_DIV_2_C
   #define BN_MP_MUL_2D_C
   #define BN_MP_MUL_D_C
   #define BN_MP_DIV_3_C
   #define BN_MP_LSHD_C
   #define BN_MP_CLEAR_MULTI_C
#endif

#if defined(BN_MP_TORADIX_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_TORADIX_N_C)
   #define BN_MP_ISZERO_C
   #define BN_MP_INIT_COPY_C
   #define BN_MP_DIV_D_C
   #define BN_MP_CLEAR_C
   #define BN_MP_S_RMAP_C
#endif

#if defined(BN_MP_UNSIGNED_BIN_SIZE_C)
   #define BN_MP_COUNT_BITS_C
#endif

#if defined(BN_MP_XOR_C)
   #define BN_MP_INIT_COPY_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_MP_ZERO_C)
#endif

#if defined(BN_PRIME_TAB_C)
#endif

#if defined(BN_REVERSE_C)
#endif

#if defined(BN_S_MP_ADD_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BN_S_MP_EXPTMOD_C)
   #define BN_MP_COUNT_BITS_C
   #define BN_MP_INIT_C
   #define BN_MP_CLEAR_C
   #define BN_MP_REDUCE_SETUP_C
   #define BN_MP_REDUCE_C
   #define BN_MP_REDUCE_2K_SETUP_L_C
   #define BN_MP_REDUCE_2K_L_C
   #define BN_MP_MOD_C
   #define BN_MP_COPY_C
   #define BN_MP_SQR_C
   #define BN_MP_MUL_C
   #define BN_MP_SET_C
   #define BN_MP_EXCH_C
#endif

#if defined(BN_S_MP_MUL_DIGS_C)
   #define BN_FAST_S_MP_MUL_DIGS_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_MUL_HIGH_DIGS_C)
   #define BN_FAST_S_MP_MUL_HIGH_DIGS_C
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_SQR_C)
   #define BN_MP_INIT_SIZE_C
   #define BN_MP_CLAMP_C
   #define BN_MP_EXCH_C
   #define BN_MP_CLEAR_C
#endif

#if defined(BN_S_MP_SUB_C)
   #define BN_MP_GROW_C
   #define BN_MP_CLAMP_C
#endif

#if defined(BNCORE_C)
#endif

#ifdef LTM3
#define LTM_LAST
#endif
//#include "tommath_superclass.h"
//#include "tommath_class.h"
#else
#define LTM_LAST
#endif

/* $Source$ */
/* $Revision: 0.36 $ */
/* $Date: 2005-08-01 16:37:28 +0000 $ */

#else
#define LTM_LAST
#endif

/* $Source$ */
/* $Revision: 0.36 $ */
/* $Date: 2005-08-01 16:37:28 +0000 $ */

#ifndef MIN
   #define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
   #define MAX(x,y) ((x)>(y)?(x):(y))
#endif

#ifdef __cplusplus
/* C++ compilers don't like assigning void * to mp_digit * */
#define  OPT_CAST(x)  (x *)

#else

/* C on the other hand doesn't care */
#define  OPT_CAST(x)

#endif


/* detect 64-bit mode if possible */
//#if defined(__x86_64__) 
//   #if !(defined(MP_64BIT) && defined(MP_16BIT) && defined(MP_8BIT))
//      #define MP_64BIT
//   #endif
//#endif
#define MP_31BIT

/* some default configurations.
 *
 * A "mp_digit" must be able to hold DIGIT_BIT + 1 bits
 * A "mp_word" must be able to hold 2*DIGIT_BIT + 1 bits
 *
 * At the very least a mp_digit must be able to hold 7 bits
 * [any size beyond that is ok provided it doesn't overflow the data type]
 */
#ifdef MP_8BIT
   typedef unsigned char      mp_digit;
   typedef unsigned short     mp_word;
#elif defined(MP_16BIT)
   typedef unsigned short     mp_digit;
   typedef unsigned long      mp_word;
#elif defined(MP_64BIT)
   /* for GCC only on supported platforms */
#ifndef CRYPT
   typedef unsigned long long ulong64;
   typedef signed long long   long64;
#endif

   typedef unsigned long      mp_digit;
   typedef unsigned long      mp_word __attribute__ ((mode(TI)));

   #define DIGIT_BIT          60
#else
   /* this is the default case, 28-bit digits */
   
   /* this is to make porting into LibTomCrypt easier :-) */
#ifndef CRYPT
   #if defined(_MSC_VER) || defined(__BORLANDC__) 
      typedef unsigned __int64   ulong64;
      typedef signed __int64     long64;
   #else
      typedef unsigned long long ulong64;
      typedef signed long long   long64;
   #endif
#endif

   typedef unsigned long      mp_digit;
   typedef ulong64            mp_word;

#ifdef MP_31BIT   
   /* this is an extension that uses 31-bit digits */
   #define DIGIT_BIT          31
#else
   /* default case is 28-bit digits, defines MP_28BIT as a handy macro to test */
   #define DIGIT_BIT          28
   #define MP_28BIT
#endif   
#endif

/* define heap macros */
#ifndef CRYPT
   /* default to libc stuff */
   #ifndef XMALLOC 
       #define XMALLOC  malloc
       #define XFREE    free
       #define XREALLOC realloc
       #define XCALLOC  calloc
   #else
      /* prototypes for our heap functions */
      extern void *XMALLOC(size_t n);
      extern void *XREALLOC(void *p, size_t n);
      extern void *XCALLOC(size_t n, size_t s);
      extern void XFREE(void *p);
   #endif
#endif


/* otherwise the bits per digit is calculated automatically from the size of a mp_digit */
#ifndef DIGIT_BIT
   #define DIGIT_BIT     ((int)((CHAR_BIT * sizeof(mp_digit) - 1)))  /* bits per digit */
#endif

#define MP_DIGIT_BIT     DIGIT_BIT
#define MP_MASK          (mp_digit)((1ULL << DIGIT_BIT) - 1)
#define MP_DIGIT_MAX     MP_MASK

/* equalities */
#define MP_LT        -1   /* less than */
#define MP_EQ         0   /* equal to */
#define MP_GT         1   /* greater than */

#define MP_ZPOS       0   /* positive integer */
#define MP_NEG        1   /* negative */

#define MP_OKAY       0   /* ok result */
#define MP_MEM        -2  /* out of mem */
#define MP_VAL        -3  /* invalid input */
#define MP_RANGE      MP_VAL

#define MP_YES        1   /* yes response */
#define MP_NO         0   /* no response */

/* Primality generation flags */
#define LTM_PRIME_BBS      0x0001 /* BBS style prime */
#define LTM_PRIME_SAFE     0x0002 /* Safe prime (p-1)/2 == prime */
#define LTM_PRIME_2MSB_ON  0x0008 /* force 2nd MSB to 1 */

typedef int           mp_err;

/* you'll have to tune these... */
extern int KARATSUBA_MUL_CUTOFF,
           KARATSUBA_SQR_CUTOFF,
           TOOM_MUL_CUTOFF,
           TOOM_SQR_CUTOFF;

/* define this to use lower memory usage routines (exptmods mostly) */
/* #define MP_LOW_MEM */

/* default precision */
#ifndef MP_PREC
   #ifndef MP_LOW_MEM
      #define MP_PREC                 32     /* default digits of precision */
   #else
      #define MP_PREC                 8      /* default digits of precision */
   #endif   
#endif

/* size of comba arrays, should be at least 2 * 2**(BITS_PER_WORD - BITS_PER_DIGIT*2) */
#define MP_WARRAY               (1 << (sizeof(mp_word) * CHAR_BIT - 2 * DIGIT_BIT + 1))

/* the infamous mp_int structure */
typedef struct  {
    int used, alloc, sign;
    mp_digit *dp;
} mp_int;

/* callback for mp_prime_random, should fill dst with random bytes and return how many read [upto len] */
typedef int ltm_prime_callback(unsigned char *dst, int len, void *dat);


#define USED(m)    ((m)->used)
#define DIGIT(m,k) ((m)->dp[(k)])
#define SIGN(m)    ((m)->sign)

/* error code to char* string */
char *mp_error_to_string(int code);

/* ---> init and deinit bignum functions <--- */
/* init a bignum */
int mp_init(mp_int *a);

/* free a bignum */
void mp_clear(mp_int *a);

/* init a null terminated series of arguments */
int mp_init_multi(mp_int *mp, ...);

/* clear a null terminated series of arguments */
void mp_clear_multi(mp_int *mp, ...);

/* exchange two ints */
void mp_exch(mp_int *a, mp_int *b);

/* shrink ram required for a bignum */
int mp_shrink(mp_int *a);

/* grow an int to a given size */
int mp_grow(mp_int *a, int size);

/* init to a given number of digits */
int mp_init_size(mp_int *a, int size);

/* ---> Basic Manipulations <--- */
#define mp_iszero(a) (((a)->used == 0) ? MP_YES : MP_NO)
#define mp_iseven(a) (((a)->used > 0 && (((a)->dp[0] & 1) == 0)) ? MP_YES : MP_NO)
#define mp_isodd(a)  (((a)->used > 0 && (((a)->dp[0] & 1) == 1)) ? MP_YES : MP_NO)

/* set to zero */
void mp_zero(mp_int *a);

/* set to a digit */
void mp_set(mp_int *a, mp_digit b);

/* set a 32-bit const */
int mp_set_int(mp_int *a, unsigned long b);

/* get a 32-bit value */
unsigned long mp_get_int(mp_int * a);

/* initialize and set a digit */
int mp_init_set (mp_int * a, mp_digit b);

/* initialize and set 32-bit value */
int mp_init_set_int (mp_int * a, unsigned long b);

/* copy, b = a */
int mp_copy(mp_int *a, mp_int *b);

/* inits and copies, a = b */
int mp_init_copy(mp_int *a, mp_int *b);

/* trim unused digits */
void mp_clamp(mp_int *a);

/* ---> digit manipulation <--- */

/* right shift by "b" digits */
void mp_rshd(mp_int *a, int b);

/* left shift by "b" digits */
int mp_lshd(mp_int *a, int b);

/* c = a / 2**b */
int mp_div_2d(mp_int *a, int b, mp_int *c, mp_int *d);

/* b = a/2 */
int mp_div_2(mp_int *a, mp_int *b);

/* c = a * 2**b */
int mp_mul_2d(mp_int *a, int b, mp_int *c);

/* b = a*2 */
int mp_mul_2(mp_int *a, mp_int *b);

/* c = a mod 2**d */
int mp_mod_2d(mp_int *a, int b, mp_int *c);

/* computes a = 2**b */
int mp_2expt(mp_int *a, int b);

/* Counts the number of lsbs which are zero before the first zero bit */
int mp_cnt_lsb(mp_int *a);

/* I Love Earth! */

/* makes a pseudo-random int of a given size */
int mp_rand(mp_int *a, int digits);

/* ---> binary operations <--- */
/* c = a XOR b  */
int mp_xor(mp_int *a, mp_int *b, mp_int *c);

/* c = a OR b */
int mp_or(mp_int *a, mp_int *b, mp_int *c);

/* c = a AND b */
int mp_and(mp_int *a, mp_int *b, mp_int *c);

/* ---> Basic arithmetic <--- */

/* b = -a */
int mp_neg(mp_int *a, mp_int *b);

/* b = |a| */
int mp_abs(mp_int *a, mp_int *b);

/* compare a to b */
int mp_cmp(mp_int *a, mp_int *b);

/* compare |a| to |b| */
int mp_cmp_mag(mp_int *a, mp_int *b);

/* c = a + b */
int mp_add(mp_int *a, mp_int *b, mp_int *c);

/* c = a - b */
int mp_sub(mp_int *a, mp_int *b, mp_int *c);

/* c = a * b */
int mp_mul(mp_int *a, mp_int *b, mp_int *c);

/* b = a*a  */
int mp_sqr(mp_int *a, mp_int *b);

/* a/b => cb + d == a */
int mp_div(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a mod b, 0 <= c < b  */
int mp_mod(mp_int *a, mp_int *b, mp_int *c);

/* ---> single digit functions <--- */

/* compare against a single digit */
int mp_cmp_d(mp_int *a, mp_digit b);

/* c = a + b */
int mp_add_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a - b */
int mp_sub_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a * b */
int mp_mul_d(mp_int *a, mp_digit b, mp_int *c);

/* a/b => cb + d == a */
int mp_div_d(mp_int *a, mp_digit b, mp_int *c, mp_digit *d);

/* a/3 => 3c + d == a */
int mp_div_3(mp_int *a, mp_int *c, mp_digit *d);

/* c = a**b */
int mp_expt_d(mp_int *a, mp_digit b, mp_int *c);

/* c = a mod b, 0 <= c < b  */
int mp_mod_d(mp_int *a, mp_digit b, mp_digit *c);

/* ---> number theory <--- */

/* d = a + b (mod c) */
int mp_addmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a - b (mod c) */
int mp_submod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* d = a * b (mod c) */
int mp_mulmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* c = a * a (mod b) */
int mp_sqrmod(mp_int *a, mp_int *b, mp_int *c);

/* c = 1/a (mod b) */
int mp_invmod(mp_int *a, mp_int *b, mp_int *c);

/* c = (a, b) */
int mp_gcd(mp_int *a, mp_int *b, mp_int *c);

/* produces value such that U1*a + U2*b = U3 */
int mp_exteuclid(mp_int *a, mp_int *b, mp_int *U1, mp_int *U2, mp_int *U3);

/* c = [a, b] or (a*b)/(a, b) */
int mp_lcm(mp_int *a, mp_int *b, mp_int *c);

/* finds one of the b'th root of a, such that |c|**b <= |a|
 *
 * returns error if a < 0 and b is even
 */
int mp_n_root(mp_int *a, mp_digit b, mp_int *c);

/* special sqrt algo */
int mp_sqrt(mp_int *arg, mp_int *ret);

/* is number a square? */
int mp_is_square(mp_int *arg, int *ret);

/* computes the jacobi c = (a | n) (or Legendre if b is prime)  */
int mp_jacobi(mp_int *a, mp_int *n, int *c);

/* used to setup the Barrett reduction for a given modulus b */
int mp_reduce_setup(mp_int *a, mp_int *b);

/* Barrett Reduction, computes a (mod b) with a precomputed value c
 *
 * Assumes that 0 < a <= b*b, note if 0 > a > -(b*b) then you can merely
 * compute the reduction as -1 * mp_reduce(mp_abs(a)) [pseudo code].
 */
int mp_reduce(mp_int *a, mp_int *b, mp_int *c);

/* setups the montgomery reduction */
int mp_montgomery_setup(mp_int *a, mp_digit *mp);

/* computes a = B**n mod b without division or multiplication useful for
 * normalizing numbers in a Montgomery system.
 */
int mp_montgomery_calc_normalization(mp_int *a, mp_int *b);

/* computes x/R == x (mod N) via Montgomery Reduction */
int mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);

/* returns 1 if a is a valid DR modulus */
int mp_dr_is_modulus(mp_int *a);

/* sets the value of "d" required for mp_dr_reduce */
void mp_dr_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b using the Diminished Radix method */
int mp_dr_reduce(mp_int *a, mp_int *b, mp_digit mp);

/* returns true if a can be reduced with mp_reduce_2k */
int mp_reduce_is_2k(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup(mp_int *a, mp_digit *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k(mp_int *a, mp_int *n, mp_digit d);

/* returns true if a can be reduced with mp_reduce_2k_l */
int mp_reduce_is_2k_l(mp_int *a);

/* determines k value for 2k reduction */
int mp_reduce_2k_setup_l(mp_int *a, mp_int *d);

/* reduces a modulo b where b is of the form 2**p - k [0 <= a] */
int mp_reduce_2k_l(mp_int *a, mp_int *n, mp_int *d);

/* d = a**b (mod c) */
int mp_exptmod(mp_int *a, mp_int *b, mp_int *c, mp_int *d);

/* ---> Primes <--- */

/* number of primes */
#ifdef MP_8BIT
   #define PRIME_SIZE      31
#else
   #define PRIME_SIZE      256
#endif

/* table of first PRIME_SIZE primes */
extern const mp_digit ltm_prime_tab[];

/* result=1 if a is divisible by one of the first PRIME_SIZE primes */
int mp_prime_is_divisible(mp_int *a, int *result);

/* performs one Fermat test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_fermat(mp_int *a, mp_int *b, int *result);

/* performs one Miller-Rabin test of "a" using base "b".
 * Sets result to 0 if composite or 1 if probable prime
 */
int mp_prime_miller_rabin(mp_int *a, mp_int *b, int *result);

/* This gives [for a given bit size] the number of trials required
 * such that Miller-Rabin gives a prob of failure lower than 2^-96 
 */
int mp_prime_rabin_miller_trials(int size);

/* performs t rounds of Miller-Rabin on "a" using the first
 * t prime bases.  Also performs an initial sieve of trial
 * division.  Determines if "a" is prime with probability
 * of error no more than (1/4)**t.
 *
 * Sets result to 1 if probably prime, 0 otherwise
 */
int mp_prime_is_prime(mp_int *a, int t, int *result);

/* finds the next prime after the number "a" using "t" trials
 * of Miller-Rabin.
 *
 * bbs_style = 1 means the prime must be congruent to 3 mod 4
 */
int mp_prime_next_prime(mp_int *a, int t, int bbs_style);

/* makes a truly random prime of a given size (bytes),
 * call with bbs = 1 if you want it to be congruent to 3 mod 4 
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 * The prime generated will be larger than 2^(8*size).
 */
#define mp_prime_random(a, t, size, bbs, cb, dat) mp_prime_random_ex(a, t, ((size) * 8) + 1, (bbs==1)?LTM_PRIME_BBS:0, cb, dat)

/* makes a truly random prime of a given size (bits),
 *
 * Flags are as follows:
 * 
 *   LTM_PRIME_BBS      - make prime congruent to 3 mod 4
 *   LTM_PRIME_SAFE     - make sure (p-1)/2 is prime as well (implies LTM_PRIME_BBS)
 *   LTM_PRIME_2MSB_OFF - make the 2nd highest bit zero
 *   LTM_PRIME_2MSB_ON  - make the 2nd highest bit one
 *
 * You have to supply a callback which fills in a buffer with random bytes.  "dat" is a parameter you can
 * have passed to the callback (e.g. a state or something).  This function doesn't use "dat" itself
 * so it can be NULL
 *
 */
int mp_prime_random_ex(mp_int *a, int t, int size, int flags, ltm_prime_callback cb, void *dat);

/* ---> radix conversion <--- */
int mp_count_bits(mp_int *a);

int mp_unsigned_bin_size(mp_int *a);
int mp_read_unsigned_bin(mp_int *a, const unsigned char *b, int c);
int mp_to_unsigned_bin(mp_int *a, unsigned char *b);
int mp_to_unsigned_bin_n (mp_int * a, unsigned char *b, unsigned long *outlen);

int mp_signed_bin_size(mp_int *a);
int mp_read_signed_bin(mp_int *a, const unsigned char *b, int c);
int mp_to_signed_bin(mp_int *a,  unsigned char *b);
int mp_to_signed_bin_n (mp_int * a, unsigned char *b, unsigned long *outlen);

int mp_read_radix(mp_int *a, const char *str, int radix);
int mp_toradix(mp_int *a, char *str, int radix);
int mp_toradix_n(mp_int * a, char *str, int radix, int maxlen);
int mp_radix_size(mp_int *a, int radix, int *size);

int mp_fread(mp_int *a, int radix, FILE *stream);
int mp_fwrite(mp_int *a, int radix, FILE *stream);

#define mp_read_raw(mp, str, len) mp_read_signed_bin((mp), (str), (len))
#define mp_raw_size(mp)           mp_signed_bin_size(mp)
#define mp_toraw(mp, str)         mp_to_signed_bin((mp), (str))
#define mp_read_mag(mp, str, len) mp_read_unsigned_bin((mp), (str), (len))
#define mp_mag_size(mp)           mp_unsigned_bin_size(mp)
#define mp_tomag(mp, str)         mp_to_unsigned_bin((mp), (str))

#define mp_tobinary(M, S)  mp_toradix((M), (S), 2)
#define mp_tooctal(M, S)   mp_toradix((M), (S), 8)
#define mp_todecimal(M, S) mp_toradix((M), (S), 10)
#define mp_tohex(M, S)     mp_toradix((M), (S), 16)

/* lowlevel functions, do not call! */
int s_mp_add(mp_int *a, mp_int *b, mp_int *c);
int s_mp_sub(mp_int *a, mp_int *b, mp_int *c);
#define s_mp_mul(a, b, c) s_mp_mul_digs(a, b, c, (a)->used + (b)->used + 1)
int fast_s_mp_mul_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
int s_mp_mul_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
int fast_s_mp_mul_high_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
int s_mp_mul_high_digs(mp_int *a, mp_int *b, mp_int *c, int digs);
int fast_s_mp_sqr(mp_int *a, mp_int *b);
int s_mp_sqr(mp_int *a, mp_int *b);
int mp_karatsuba_mul(mp_int *a, mp_int *b, mp_int *c);
int mp_toom_mul(mp_int *a, mp_int *b, mp_int *c);
int mp_karatsuba_sqr(mp_int *a, mp_int *b);
int mp_toom_sqr(mp_int *a, mp_int *b);
int fast_mp_invmod(mp_int *a, mp_int *b, mp_int *c);
int mp_invmod_slow (mp_int * a, mp_int * b, mp_int * c);
int fast_mp_montgomery_reduce(mp_int *a, mp_int *m, mp_digit mp);
int mp_exptmod_fast(mp_int *G, mp_int *X, mp_int *P, mp_int *Y, int mode);
int s_mp_exptmod (mp_int * G, mp_int * X, mp_int * P, mp_int * Y, int mode);
void bn_reverse(unsigned char *s, int len);

extern const char *mp_s_rmap;

#endif


/* $Source$ */
/* $Revision: 0.39 $ */
/* $Date: 2006-04-06 19:49:59 +0000 $ */

/* LibTomFloat, multiple-precision floating-point library
 *
 * LibTomFloat is a library that provides multiple-precision
 * floating-point artihmetic as well as trigonometric functionality.
 *
 * This library requires the public domain LibTomMath to be installed.
 * 
 * This library is free for all purposes without any express
 * gurantee it works
 *
 * Tom St Denis, tomstdenis@iahu.ca, http://float.libtomcrypt.org
 */
#ifndef TF_H_
#define TF_H_

/* this is mp_float type */
typedef struct {
     mp_int mantissa;
     long   radix,       /* how many bits for mantissa */
            exp;         /* current exponent, e.g. mantissa * 2^exp == number  */
} mp_float;

/* initializers */
int  mpf_init(mp_float *a, long radix);
void mpf_clear(mp_float *a);

int  mpf_init_multi(long radix, mp_float *a, ...);
void mpf_clear_multi(mp_float *a, ...);

int  mpf_init_copy(mp_float *a, mp_float *b);

int  mpf_copy(mp_float *src, mp_float *dest);
void mpf_exch(mp_float *a, mp_float *b);

/* maintainers/misc */
int  mpf_normalize(mp_float *a);
int  mpf_normalize_to(mp_float *a, long radix);
int  mpf_iterations(mp_float *a);

/* constants */
int  mpf_const_0(mp_float *a);                  /* valid zero */
int  mpf_const_d(mp_float *a, long d);          /* valid d */
int  mpf_const_ln_d(mp_float *a, long b);       /* a = ln(b)     */
int  mpf_const_sqrt_d(mp_float *a, long b);     /* a = sqrt(b);  */

/* math constants as they appear in math.h */
int  mpf_const_e(mp_float *a);                  /* e             */
int  mpf_const_l2e(mp_float *a);                /* log_2 e       */
int  mpf_const_l10e(mp_float *a);               /* log_10 e      */
int  mpf_const_le2(mp_float *a);                /* log_e 2       */
int  mpf_const_pi(mp_float *a);                 /* Pi            */
int  mpf_const_pi2(mp_float *a);                /* Pi/2          */
int  mpf_const_pi4(mp_float *a);                /* Pi/4          */
int  mpf_const_1pi(mp_float *a);                /* 1/Pi          */
int  mpf_const_2pi(mp_float *a);                /* 2/Pi          */
int  mpf_const_2rpi(mp_float *a);               /* 2/sqrt(Pi)    */
int  mpf_const_r2(mp_float *a);                 /* sqrt(2)       */
int  mpf_const_1r2(mp_float *a);                /* 1/sqrt(2)     */

/* sign operators */
int  mpf_abs(mp_float *a, mp_float *b);         /* absolute */
int  mpf_neg(mp_float *a, mp_float *b);         /* negation */

/* basic math */
int  mpf_mul_2(mp_float *a, mp_float *b);              /* b = 2a       */
int  mpf_div_2(mp_float *a, mp_float *b);              /* b = a/2      */
int  mpf_add(mp_float *a, mp_float *b, mp_float *c);   /* c = a + b    */
int  mpf_sub(mp_float *a, mp_float *b, mp_float *c);   /* c = a - b    */
int  mpf_mul(mp_float *a, mp_float *b, mp_float *c);   /* c = a * b    */
int  mpf_div(mp_float *a, mp_float *b, mp_float *c);   /* c = a / b    */
int  mpf_sqr(mp_float *a, mp_float *b);                /* b = a^2      */

int  mpf_add_d(mp_float *a, long b, mp_float *c);      /* c = a + b    */
int  mpf_sub_d(mp_float *a, long b, mp_float *c);      /* c = a - b    */
int  mpf_mul_d(mp_float *a, long b, mp_float *c);      /* c = a * b    */
int  mpf_div_d(mp_float *a, long b, mp_float *c);      /* c = a / b    */

/* compares */
int  mpf_cmp(mp_float *a,   mp_float *b);
int  mpf_cmp_d(mp_float *a, long b, int *res);
#define mpf_iszero(a) mp_iszero(&((a)->mantissa))

/* Algebra */
int  mpf_exp(mp_float *a, mp_float *b);                /* b = e^a       */
int  mpf_pow(mp_float *a, mp_float *b, mp_float *c);   /* c = a^b       */
int  mpf_ln(mp_float *a, mp_float *b);                 /* b = ln a      */
int  mpf_invsqrt(mp_float *a, mp_float *b);            /* b = 1/sqrt(a) */
int  mpf_inv(mp_float *a, mp_float *b);                /* b = 1/a       */
int  mpf_sqrt(mp_float *a, mp_float *b);               /* b = sqrt(a)   */

/* Trig */
int  mpf_cos(mp_float *a, mp_float *b);                /* b = cos(a)    */
int  mpf_sin(mp_float *a, mp_float *b);                /* b = sin(a)    */
int  mpf_tan(mp_float *a, mp_float *b);                /* b = tan(a)    */
int  mpf_acos(mp_float *a, mp_float *b);               /* b = acos(a)   */
int  mpf_asin(mp_float *a, mp_float *b);               /* b = asin(a)   */
int  mpf_atan(mp_float *a, mp_float *b);               /* b = atan(a)   */

/* ASCII <=> mp_float conversions */
char *mpf_to_string(mp_float *a, mp_digit radix);
int mpf_from_string(mp_float *a, const char *str, mp_digit radix);

#endif


#endif  // __LIBTOM__
/* end libtom.h */