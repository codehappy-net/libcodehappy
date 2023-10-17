/***

	bits.h

	Useful low-level bit-twiddling functions of various sorts.

	Copyright (c) 2014-2022 Chris Street

***/
#ifndef _BITS_H_
#define _BITS_H_

/*** turn off the rightmost 1 bit in an integer ***/
extern u32 rightmost_1_bit_off(u32 i);
extern i32 rightmost_1_bit_off(i32 i);
extern u64 rightmost_1_bit_off(u64 i);
extern i64 rightmost_1_bit_off(i64 i);

/*** is a given integer a power of 2? ***/
extern bool ispow2(i32 i);
extern bool ispow2(u32 i);
extern bool ispow2(i64 i);
extern bool ispow2(u64 i);

/*** return the rightmost 1 bit in an integer ***/
extern u32 rightmost_1_bit(u32 i);
extern i32 rightmost_1_bit(i32 i);
extern u64 rightmost_1_bit(u64 i);
extern i64 rightmost_1_bit(i64 i);

/*** return the next integer with the same number of 1 bits ***/
extern u32 next_same_number_1_bits(u32 i);
extern u64 next_same_number_1_bits(u64 i);

/*** the sign function ***/
extern int sign_function(i32 i);
extern int sign_function(i64 i);

/*** rotate shifts ***/
extern u32 rotate_left(u32 i, u32 s);
extern u64 rotate_left(u64 i, u32 s);
extern u32 rotate_right(u32 i, u32 s);
extern u64 rotate_right_u64(u64 i, u32 s);

/*** double length addition for 64 bit integers -- adds the
	128-bit numbers represented by x1,x2 and y1,y2 to
	z1,z2. x1,y1,z1 are the most-significant dwords. ***/
extern void double_length_addition_64(i64 x1, u64 x2, i64 y1, u64 y2, i64& z1, u64& z2);

/*** double length subtraction for 64 bit integers -- subtracts the
	128-bit numbers represented by x1,x2 and y1,y2 to
	z1,z2. x1,y1,z1 are the most-significant dwords. ***/
extern void double_length_subtraction_64(i64 x1, u64 x2, i64 y1, u64 y2, i64& z1, u64& z2);

/*** Bit shifts of 128-bit numbers. In the following, x1,x2 represents
	the 128-bit number, and y1,y2 will be the shifted value. n is
	the number of bits to shift.
	x1,y1 are the most-significant dwords. ***/
extern void double_length_shift_left_64(u64 x1, u64 x2, u64& y1, u64& y2, u32 n);
extern void double_length_shift_right_64(u64 x1, u64 x2, u64& y1, u64& y2, u32 n);

/*** Difference or zero functions ***/
extern u32 difference_or_zero(u32 i, u32 j);
extern i32 difference_or_zero(i32 i, i32 j);
extern u64 difference_or_zero(u64 i, u64 j);
extern i64 difference_or_zero(i64 i, i64 j);

/*** max and min for integers ***/
extern u32 max_u32(u32 i, u32 j);
extern i32 max_32(i32 i, i32 j);
extern u64 max_u64(u64 i, u64 j);
extern i64 max_64(i64 i, i64 j);
extern u32 min_u32(u32 i, u32 j);
extern i32 min_32(i32 i, i32 j);
extern u64 min_u64(u64 i, u64 j);
extern i64 min_64(i64 i, i64 j);

/*** min and max over 3 inputs ***/
extern int min_int3(int i, int j, int k);
extern int max_int3(int i, int j, int k);

/*** ...and 4 inputs ***/
extern int min_int4(int i, int j, int k, int l);
extern int max_int4(int i, int j, int k, int l);

/*** floating point min and max ***/
extern double max_double(double e, double f);
extern double min_double(double e, double f);
extern unsigned int max_uint(unsigned int i, unsigned int j);
extern unsigned int min_uint(unsigned int i, unsigned int j);

/*** These functions alternate between two values. If
	x is equal to v1, they return v2. If i is equal
	to v2, they return v1. ***/
extern i32 alternate(i32 x, i32 v1, i32 v2);
extern i64 alternate(i64 x, i64 v1, i64 v2);
extern u32 alternate(u32 x, u32 v1, u32 v2);
extern u64 alternate(u64 x, u64 v1, u64 v2);

/*** rounding integers down or up to the nearest power of 2 ***/
extern u32 largest_pow2_less_than(u32 i);
extern u64 largest_pow2_less_than(u64 i);
extern u32 smallest_pow2_greater_than(u32 i);
extern u64 smallest_pow2_greater_than(u64 i);

/*** count the number of 1-bits in an integer ***/
extern unsigned int count_bits(u16 i);
extern unsigned int count_bits(i16 i);
extern unsigned int count_bits(u32 i);
extern unsigned int count_bits(i32 i);
extern unsigned int count_bits(u64 i);
extern unsigned int count_bits(i64 i);

/*** the minimum number of bits needed to represent a given integer ***/
extern unsigned int number_bits_to_represent(i32 i);
extern unsigned int number_bits_to_represent(u32 u);

/*** the number of trailing zeros (in binary representation) ***/
unsigned int ntz(u32 x);
unsigned int ntz(u64 x);

/*** the number of leading zeros (in binary representation) ***/
extern unsigned int nlz(u32 x);
extern unsigned int nlz(u64 x);

/*** integer square root function ***/
extern u32 isqrt(u32 x);

/*** integer cube root functions ***/
extern u32 icuberoot(u32 x);
extern u32 icuberoot(u64 x);

/*** integer exponentiation functions ***/
extern i32 iexp(u32 x, unsigned int n);
extern i64 iexp(u64 x, unsigned int n);

/*** integer log base 10 function ***/
extern unsigned int ilog10(u32 x);

/*** reflect/reverse bits in a 32-bit integer ***/
extern u32 reverse_bits(u32 x);

/*** calculate cyclic redundancy check, CRC32 ***/
extern u32 crc32(unsigned char *message);

/*** parity function ***/
extern u32 parity(u32 x);
extern u32 parity(u64 x);

/*** The 6 Hamming code check bits for 32-bit data u. ***/
extern unsigned int hamming_checkbits(u32 u);

/*** Validate Hamming code. ***/
extern int hamming_correct(unsigned int pr, u32 *ur);

/*** Returns the 6 Hamming code check bits plus the overall parity bit for u. ***/
extern unsigned int hamming_checkbits_with_parity_bit(u32 u);

/*** super-fast but approximate floating point square roots ***/
/*** These next three approximate the reciprocal of the square root ***/
/*** the first is basically an initial guess for Newton's method, the next
	is after one iteration, and the third is after two iterations. ***/
extern float rough_but_fast_rsqrt(float x0);	// very approximate, very fast
extern float approx_rsqrt(float x0);		// more accurate, a little slower
extern float fast_rsqrt(float x0);		// most accurate approximation, still pretty fast

/*** these three are as above, but calculate the square root itself ***/
extern float rough_but_fast_sqrt(float x0);
extern float approx_sqrt(float x0);
extern float fast_sqrt(float x0);

/*** super-fast approximate floating point cube roots ***/
/*** the integer in the function name is the number of Newton-Raphson iterations ***/
extern float cuberoot_0(float x0);
extern float cuberoot_1(float x0);
extern float cuberoot_2(float x0);

#endif  // _BITS_H_
