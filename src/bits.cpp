/***

	bits.cpp

	Useful low-level bit-twiddling functions of various sorts. Special thanks to Henry S. Warren Jr.,
	the authors of HACKMEM, and D. E. Knuth. Some of their code is reproduced here with permission.

	For the native (i.e. non-WASM) build of libcodehappy, some useful x64/x86 ASM functions are included 
	as well.

	Copyright (c) 2014-2022 Chris Street

***/

#include "libcodehappy.h"

u32 rightmost_1_bit_off(u32 i)
{
	return (i & (i - 1));
}

i32 rightmost_1_bit_off(i32 i)
{
	return (i & (i - 1));
}

u64 rightmost_1_bit_off(u64 i)
{
	return (i & (i - 1));
}

i64 rightmost_1_bit_off(i64 i)
{
	return (i & (i - 1));
}

bool ispow2(i32 i)
{
	if (i <= 0)
		return(0);
	return (i & (i - 1)) == 0;
}

bool ispow2(u32 i)
{
	if (iszero(i))
		return(0);
	return (i & (i - 1)) == 0;
}

bool ispow2(i64 i)
{
	if (i <= 0)
		return(0);
	return (i & (i - 1)) == 0;
}

bool ispow2(u64 i)
{
	if (unlikely(i == 0))
		return(0);

	return (i & (i - 1)) == 0;
}

u32 rightmost_1_bit(u32 i)
{
	return (u32)((i32)i & (-(i32)i));
}

i32 rightmost_1_bit(i32 i)
{
	return (i & (-i));
}

u64 rightmost_1_bit(u64 i)
{
	return (u64)((i64)i & (-(i64)i));
}

i64 rightmost_1_bit(i64 i)
{
	return (i & (-i));
}

// these two are useful in n-choose-k algorithms, as in permute.c
u32 next_same_number_1_bits(u32 i)
{
	u32 smallest, ripple, ones;

	smallest = i & (-i);
	ripple = i + smallest;
	ones = i ^ ripple;
	ones = (ones >> 2) / smallest;

	return (ripple | ones);
}

u64 next_same_number_1_bits(u64 i)
{
	u64 smallest, ripple, ones;

	smallest = i & (-i);
	ripple = i + smallest;
	ones = i ^ ripple;
	ones = (ones >> 2) / smallest;

	return (ripple | ones);
}

int sign_function(i32 i)
{
	if (i > 0)
		return(1);
	if (i < 0)
		return(-1);
	return(0);
}

int sign_function(i64 i)
{
	if (i > 0)
		return(1);
	if (i < 0)
		return(-1);
	return(0);
}

u32 rotate_left(u32 i, u32 s)
{
	return (i << s) | (i >> (32 - s));
}

u64 rotate_left(u64 i, u32 s)
{
	return (i << s) | (i >> (64 - s));
}

u32 rotate_right(u32 i, u32 s)
{
	return (i >> s) | (i << (32 - s));
}

u64 rotate_right_u64(u64 i, u32 s)
{
	return (i >> s) | (i << (32 - s));
}

void double_length_addition_64(
	i64 x1, u64 x2,
	i64 y1, u64 y2,
	i64& z1, u64& z2)
{
	u64 c;

	z2 = x2 + y2;

	c = (x2 & y2) | ((x2 | y2) & (~(z2)));
	c >>= 63;
	
	z1 = x1 + y1 + c;
}

void double_length_subtraction_64(
	i64 x1, u64 x2,
	i64 y1, u64 y2,
	i64& z1, u64& z2)
{
	u64 b;

	z2 = x2 - y2;

	b = ((~x2) & y2) | ((x2 == y2) & (z2));
	b >>= 63;

	z1 = x1 - y1 - b;
}

static u64 __shl_ex(u64 i, u32 p)
{
	if (p < 0 || p > 63)
		return(0LL);
	return (i << p);
}

static u64 __shr_ex(u64 i, u32 p)
{
	if (p < 0 || p > 63)
		return(0LL);
	return (i >> p);
}

void double_length_shift_left_64(
	u64 x1, u64 x2,
	u64& y1, u64& y2,
	u32 n
	)
{
	y1 =	(__shl_ex(x1, n)) | 
		(__shr_ex(x2, (64 - n))) | 
		(__shl_ex(x2, (n - 64)));

	y2 = __shl_ex(x2, n);
}

void double_length_shift_right_64(
	u64 x1, u64 x2,
	u64& y1, u64& y2,
	u32 n
	)
{
	y2 = 	(__shr_ex(x2, n)) |
		(__shl_ex(x1, 64 - n)) |
		(__shr_ex(x1, n - 64));

	y1 = 	__shr_ex(x1, n); 
}

u32 difference_or_zero(u32 i, u32 j)
{
	if (i >= j)
		return(i - j);
	return(0);
}

i32 difference_or_zero(i32 i, i32 j)
{
	if (i >= j)
		return(i - j);
	return(0);
}

u64 difference_or_zero(u64 i, u64 j)
{
	if (i >= j)
		return(i - j);
	return(0);
}

i64 difference_or_zero(i64 i, i64 j)
{
	if (i >= j)
		return(i - j);
	return(0);
}

u32 max_u32(u32 i, u32 j)
{
	return ((i > j) ? i : j);
}

i32 max_32(i32 i, i32 j)
{
	return ((i > j) ? i : j);
}

u64 max_u64(u64 i, u64 j)
{
	return ((i > j) ? i : j);
}

i64 max_64(i64 i, i64 j)
{
	return ((i > j) ? i : j);
}

u32 min_u32(u32 i, u32 j)
{
	return ((i < j) ? i : j);
}

i32 min_32(i32 i, i32 j)
{
	return ((i < j) ? i : j);
}

u64 min_u64(u64 i, u64 j)
{
	return ((i < j) ? i : j);
}

i64 min_64(i64 i, i64 j)
{
	return ((i < j) ? i : j);
}

int min_int3(int i, int j, int k)
{
	return min_int(min_int(i, j), k);
}

int max_int3(int i, int j, int k)
{
	return max_int(max_int(i, j), k);
}

int min_int4(int i, int j, int k, int l)
{
	return min_int(min_int3(i, j, k), l);
}

int max_int4(int i, int j, int k, int l)
{
	return max_int(max_int3(i, j, k), l);
}

double max_double(double e, double f)
{
	return ((e < f) ? f : e);
}

double min_double(double e, double f)
{
	return ((e < f) ? e : f);
}

unsigned int max_uint(unsigned int i, unsigned int j)
{
	return ((i > j) ? i : j);
}

unsigned int min_uint(unsigned int i, unsigned int j)
{
	return ((i < j) ? i : j);
}

i32 alternate(i32 x, i32 v1, i32 v2)
{
	return (x ^ v1 ^ v2);
}

i64 alternate(i64 x, i64 v1, i64 v2)
{
	return (v1 ^ v2 ^ x);
}

u32 alternate(u32 x, u32 v1, u32 v2)
{
	return (x ^ v1 ^ v2);
}

u64 alternate(u64 x, u64 v1, u64 v2)
{
	return (v1 ^ v2 ^ x);
}

u32 largest_pow2_less_than(u32 i)
{
	i = i | (i >> 1);
	i = i | (i >> 2);
	i = i | (i >> 4);
	i = i | (i >> 8);
	i = i | (i >> 16);

	return (i - (i >> 1));
}

u64 largest_pow2_less_than(u64 i)
{
	i = i | (i >> 1);
	i = i | (i >> 2);
	i = i | (i >> 4);
	i = i | (i >> 8);
	i = i | (i >> 16);
	i = i | (i >> 32);

	return (i - (i >> 1));
}

u32 smallest_pow2_greater_than(u32 i)
{
	i = i | (i >> 1);
	i = i | (i >> 2);
	i = i | (i >> 4);
	i = i | (i >> 8);
	i = i | (i >> 16);

	return (++i);
}

u64 smallest_pow2_greater_than(u64 i)
{
	i = i | (i >> 1);
	i = i | (i >> 2);
	i = i | (i >> 4);
	i = i | (i >> 8);
	i = i | (i >> 16);
	i = i | (i >> 32);

	return (++i);
}

static const unsigned char __countbits[] =
	{
	0, 1, 1, 2, 1, 2, 2, 3,		// 0 - 7
	1, 2, 2, 3, 2, 3, 3, 4,		// 8 - 15
	1, 2, 2, 3, 2, 3, 3, 4,		// 16 - 31
	2, 3, 3, 4, 3, 4, 4, 5,		 
	1, 2, 2, 3, 2, 3, 3, 4,		// 32 - 63
	2, 3, 3, 4, 3, 4, 4, 5,		 
	2, 3, 3, 4, 3, 4, 4, 5,		 
	3, 4, 4, 5, 4, 5, 5, 6,		
	1, 2, 2, 3, 2, 3, 3, 4,		// 64 - 127
	2, 3, 3, 4, 3, 4, 4, 5,		
	2, 3, 3, 4, 3, 4, 4, 5,		
	3, 4, 4, 5, 4, 5, 5, 6,		 
	2, 3, 3, 4, 3, 4, 4, 5,		
	3, 4, 4, 5, 4, 5, 5, 6,		 
	3, 4, 4, 5, 4, 5, 5, 6,		 
	4, 5, 5, 6, 5, 6, 6, 7,		
	1, 2, 2, 3, 2, 3, 3, 4,		// 128 - 255
	2, 3, 3, 4, 3, 4, 4, 5,		
	2, 3, 3, 4, 3, 4, 4, 5,		
	3, 4, 4, 5, 4, 5, 5, 6,		 
	2, 3, 3, 4, 3, 4, 4, 5,		
	3, 4, 4, 5, 4, 5, 5, 6,		 
	3, 4, 4, 5, 4, 5, 5, 6,		 
	4, 5, 5, 6, 5, 6, 6, 7,		
	2, 3, 3, 4, 3, 4, 4, 5,		
	3, 4, 4, 5, 4, 5, 5, 6,		
	3, 4, 4, 5, 4, 5, 5, 6,		
	4, 5, 5, 6, 5, 6, 6, 7,		 
	3, 4, 4, 5, 4, 5, 5, 6,		
	4, 5, 5, 6, 5, 6, 6, 7,		 
	4, 5, 5, 6, 5, 6, 6, 7,		 
	5, 6, 6, 7, 6, 7, 7, 8,		
	};

unsigned int count_bits(u16 i)
{
	unsigned char* p;
	p = (unsigned char*)(&i);
	return __countbits[p[0]] + __countbits[p[1]];
}

unsigned int count_bits(i16 i)
{
	unsigned char* p;
	p = (unsigned char*)(&i);
	return __countbits[p[0]] + __countbits[p[1]];
}

unsigned int count_bits(u32 i)
{
#if 1
	unsigned char* p;
	p = (unsigned char*)(&i);
	return	__countbits[p[0]] +
		__countbits[p[1]] +
		__countbits[p[2]] +
		__countbits[p[3]];
#else
   i = i - ((i >> 1) & 0x55555555);
   i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
   i = (i + (i >> 4)) & 0x0F0F0F0F;
   i = i + (i << 8);
   i = i + (i << 16);
   return i >> 24;
#endif
}

unsigned int count_bits(i32 i)
{
	u32 *p;
	p = (u32 *)(&i);
	return count_bits(*p);
}

unsigned int count_bits(u64 i)
{
	u32 *p;
	p = (u32 *)(&i);

	return count_bits(p[0]) + count_bits(p[1]);
}

unsigned int count_bits(i64 i)
{
	u64 *p;
	p = (u64 *)(&i);
	return count_bits(*p);
}

#define MXB(x)	(1U << ((x) - 1))

unsigned int number_bits_to_represent(i32 i)
{
	u32 u;
	u = *((u32 *)&i);
	// binary search
	if (u >= 0x10000)
		{
		// 17-32
		if (u >= MXB(25))
			{
			// 25-32
			if (u >= MXB(29))
				{
				// 29-32
				if (u >= MXB(31))
					{
					// 31-32
					if (u >= MXB(32))
						return(32);
					return(31);
					}
				else
					{
					// 29-30
					if (u >= MXB(30))
						return(30);
					return(29);
					}
				}
			else
				{
				// 25-28
				if (u >= MXB(27))
					{
					// 27-28
					if (u >= MXB(28))
						return(28);
					return(27);
					}
				else
					{
					// 25-26
					if (u >= MXB(26))
						return(26);
					return(25);
					}
				}
			}
		else
			{
			// 17-24
			if (u >= MXB(21))
				{
				// 21-24
				if (u >= MXB(23))
					{
					// 23-24
					if (u >= MXB(24))
						return(24);
					return(23);
					}
				else
					{
					// 21-22
					if (u >= MXB(22))
						return(22);
					return(21);
					}
				}
			else
				{
				// 17-20
				if (u >= MXB(19))
					{
					// 19-20
					if (u >= MXB(20))
						return(20);
					return(19);
					}
				else
					{
					// 17-18
					if (u >= MXB(18))
						return(18);
					return(17);
					}
				}
			}
		}
	else
		{
		// 1-16
		if (u >= MXB(9))
			{
			// 9-16
			if (u >= MXB(13))
				{
				// 13-16
				if (u >= MXB(15))
					{
					// 15-16
					if (u >= MXB(16))
						return(16);
					return(15);
					}
				else
					{
					// 13-14
					if (u >= MXB(14))
						return(14);
					return(13);
					}
				}
			else
				{
				// 9-12
				if (u >= MXB(11))
					{
					// 11-12
					if (u >= MXB(12))
						return(12);
					return(11);
					}
				else
					{
					// 9-10
					if (u >= MXB(10))
						return(10);
					return(9);
					}
				}
			}
		else
			{
			// 1-8
			if (u >= MXB(5))
				{
				// 5-8
				if (u >= MXB(7))
					{
					// 7-8
					if (u >= MXB(8))
						return(8);
					return(7);
					}
				else
					{
					// 5-6
					if (u >= MXB(6))
						return(6);
					return(5);
					}
				}
			else
				{
				// 1-4
				if (u >= MXB(3))
					{
					// 3-4
					if (u >= MXB(4))
						return(4);
					return(3);
					}
				else
					{
					// 1-2
					if (u >= MXB(2))
						return(2);
					return(1);
					}
				}
			}
		}

	return(0);
}

unsigned int number_bits_to_represent(u32 u)
{
	// binary search
	if (u >= MXB(17))
		{
		// 17-32
		if (u >= MXB(25))
			{
			// 25-32
			if (u >= MXB(29))
				{
				// 29-32
				if (u >= MXB(31))
					{
					// 31-32
					if (u >= MXB(32))
						return(32);
					return(31);
					}
				else
					{
					// 29-30
					if (u >= MXB(30))
						return(30);
					return(29);
					}
				}
			else
				{
				// 25-28
				if (u >= MXB(27))
					{
					// 27-28
					if (u >= MXB(28))
						return(28);
					return(27);
					}
				else
					{
					// 25-26
					if (u >= MXB(26))
						return(26);
					return(25);
					}
				}
			}
		else
			{
			// 17-24
			if (u >= MXB(21))
				{
				// 21-24
				if (u >= MXB(23))
					{
					// 23-24
					if (u >= MXB(24))
						return(24);
					return(23);
					}
				else
					{
					// 21-22
					if (u >= MXB(22))
						return(22);
					return(21);
					}
				}
			else
				{
				// 17-20
				if (u >= MXB(19))
					{
					// 19-20
					if (u >= MXB(20))
						return(20);
					return(19);
					}
				else
					{
					// 17-18
					if (u >= MXB(18))
						return(18);
					return(17);
					}
				}
			}
		}
	else
		{
		// 1-16
		if (u >= MXB(9))
			{
			// 9-16
			if (u >= MXB(13))
				{
				// 13-16
				if (u >= MXB(15))
					{
					// 15-16
					if (u >= MXB(16))
						return(16);
					return(15);
					}
				else
					{
					// 13-14
					if (u >= MXB(14))
						return(14);
					return(13);
					}
				}
			else
				{
				// 9-12
				if (u >= MXB(11))
					{
					// 11-12
					if (u >= MXB(12))
						return(12);
					return(11);
					}
				else
					{
					// 9-10
					if (u >= MXB(10))
						return(10);
					return(9);
					}
				}
			}
		else
			{
			// 1-8
			if (u >= MXB(5))
				{
				// 5-8
				if (u >= MXB(7))
					{
					// 7-8
					if (u >= MXB(8))
						return(8);
					return(7);
					}
				else
					{
					// 5-6
					if (u >= MXB(6))
						return(6);
					return(5);
					}
				}
			else
				{
				// 1-4
				if (u >= MXB(3))
					{
					// 3-4
					if (u >= MXB(4))
						return(4);
					return(3);
					}
				else
					{
					// 1-2
					if (u >= MXB(2))
						return(2);
					return(1);
					}
				}
			}
		}

	return(0);
}

#undef	MXB

// Number of trailing zeros.
unsigned int ntz(u32 x)
{
   unsigned int n;

   if (x == 0) return(32);
   n = 1;
   if ((x & 0x0000FFFF) == 0) {n = n +16; x = x >>16;}
   if ((x & 0x000000FF) == 0) {n = n + 8; x = x >> 8;}
   if ((x & 0x0000000F) == 0) {n = n + 4; x = x >> 4;}
   if ((x & 0x00000003) == 0) {n = n + 2; x = x >> 2;}
   return n - (x & 1);
}

unsigned int ntz(u64 x)
{
   unsigned int n;

   if (x == 0) return(64);
   n = 1;
   if ((x & 0xFFFFFFFFULL) == 0) {n = n +32; x = x >>32;}
   if ((x & 0x0000FFFFULL) == 0) {n = n +16; x = x >>16;}
   if ((x & 0x000000FFULL) == 0) {n = n + 8; x = x >> 8;}
   if ((x & 0x0000000FULL) == 0) {n = n + 4; x = x >> 4;}
   if ((x & 0x00000003ULL) == 0) {n = n + 2; x = x >> 2;}
   return n - (x & 1);
}

unsigned int nlz(u32 x)
{
   unsigned int n;

   if (x == 0) return(32);
   n = 0;
   if (x <= 0x0000FFFF) {n = n +16; x = x <<16;}
   if (x <= 0x00FFFFFF) {n = n + 8; x = x << 8;}
   if (x <= 0x0FFFFFFF) {n = n + 4; x = x << 4;}
   if (x <= 0x3FFFFFFF) {n = n + 2; x = x << 2;}
   if (x <= 0x7FFFFFFF) {n = n + 1;}
   return n;
}

unsigned int nlz(u64 x)
{
   unsigned int n;

   if (x == 0) return(64);
   n = 0;
   if (x <= 0x00000000FFFFFFFFULL) {n = n +32; x = x <<32;}
   if (x <= 0x0000FFFFFFFFFFFFULL) {n = n +16; x = x <<16;}
   if (x <= 0x00FFFFFFFFFFFFFFULL) {n = n + 8; x = x << 8;}
   if (x <= 0x0FFFFFFFFFFFFFFFULL) {n = n + 4; x = x << 4;}
   if (x <= 0x3FFFFFFFFFFFFFFFULL) {n = n + 2; x = x << 2;}
   if (x <= 0x7FFFFFFFFFFFFFFFULL) {n = n + 1;}
   return n;
}

u32 isqrt(u32 x)
{
   int s, g0, g1;

   if (x <= 4224)
      if (x <= 24)
         if (x <= 3) return (x + 3) >> 2;
         else if (x <= 8) return 2;
         else return (x >> 4) + 3;
      else if (x <= 288)
         if (x <= 80) s = 3; else s = 4;
      else if (x <= 1088) s = 5; else s = 6;
   else if (x <= 1025*1025 - 1)
      if (x <= 257*257 - 1)
         if (x <= 129*129 - 1) s = 7; else s = 8;
      else if (x <= 513*513 - 1) s = 9; else s = 10;
   else if (x <= 4097*4097 - 1)
      if (x <= 2049*2049 - 1) s = 11; else s = 12;
   else if (x <= 16385*16385 - 1)
      if (x <= 8193*8193 - 1) s = 13; else s = 14;
   else if (x <= 32769*32769 - 1) s = 15; else s = 16;
   g0 = 1 << s;                // g0 = 2**s.

   g1 = (g0 + (x >> s)) >> 1;  // g1 = (g0 + x/g0)/2.

   while (g1 < g0) {           // Do while approximations
      g0 = g1;                 // strictly decrease.
      g1 = (g0 + (x/g0)) >> 1;
   }
   return g0;
}

u32 icuberoot(u32 x)
{
   int s;
   u32 y, b, y2;

   y2 = 0;
   y = 0;
   for (s = 30; s >= 0; s -= 3) {
      y2 = 4*y2;
      y = 2*y;
      b = (3*(y2 + y) + 1) << s;
      if (x >= b) {
         x = x - b;
         y2 = y2 + 2*y + 1;
         y = y + 1;
      }
   }
   return y;
}

u32 icuberoot(u64 x)
{
   int s;
   u64 y, b, bs, y2;

   y2 = 0;
   y = 0;
   for (s = 63; s >= 0; s = s - 3) {
      y2 = 4*y2;
      y = 2*y;
      b = 3*(y2 + y) + 1;
      bs = b << s;
      if (x >= bs && b == (bs >> s)) {
         x = x - bs;
         y2 = y2 + 2*y + 1;
         y = y + 1;
      }
   }
   return (u32)y;
}

i32 iexp(u32 x, unsigned int n)
{
   i32 p, y;

   y = 1;                     // Initialize result
   p = x;                     // and p.
   while(1) {
      if (n & 1) y = p*y;     // If n is odd, mult by p.
      n = n >> 1;             // Position next bit of n.
      if (n == 0) return y;   // If no more bits in n.
      p = p*p;                // Power for next bit of n.
   }
   return y;
}

i64 iexp(u64 x, unsigned int n)
{
   i64 p, y;

   y = 1;                     // Initialize result
   p = x;                     // and p.
   while(1) {
      if (n & 1) y = p*y;     // If n is odd, mult by p.
      n = n >> 1;             // Position next bit of n.
      if (n == 0) return y;   // If no more bits in n.
      p = p*p;                // Power for next bit of n.
   }
   return y;
}

unsigned int ilog10(u32 x)
{
   int y;
   static unsigned char table1[33] = {10, 9, 9, 8, 8, 8,
      7, 7, 7, 6, 6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 3, 3, 3,
      2, 2, 2, 1, 1, 1, 0, 0, 0, 0};
   static unsigned table2[11] = {1, 10, 100, 1000, 10000,
      100000, 1000000, 10000000, 100000000, 1000000000,
      0};

   y = table1[nlz(x)];
   y = y - ((x - table2[y]) >> 31);
   return y;
}

u32 reverse_bits(u32 x)
{
   x = ((x & 0x55555555) <<  1) | ((x >>  1) & 0x55555555);
   x = ((x & 0x33333333) <<  2) | ((x >>  2) & 0x33333333);
   x = ((x & 0x0F0F0F0F) <<  4) | ((x >>  4) & 0x0F0F0F0F);
   x = (x << 24) | ((x & 0xFF00) << 8) |
       ((x >> 8) & 0xFF00) | (x >> 24);
   return x;
}

u32 crc32(unsigned char *message)
{
   int i, j;
   unsigned int byte, crc, mask;
   static unsigned int table[256] = {0};

   /* Set up the table, if necessary. */

   if (table[1] == 0) {
      for (byte = 0; byte <= 255; byte++) {
         crc = byte;
         for (j = 7; j >= 0; j--) {    // Do eight times.
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
         }
         table[byte] = crc;
      }
   }

   /* Through with table setup, now calculate the CRC. */

   i = 0;
   crc = 0xFFFFFFFF;
   while ((byte = message[i]) != 0) {
      crc = (crc >> 8) ^ table[(crc ^ byte) & 0xFF];
      i = i + 1;
   }
   return ~crc;
}

u32 parity(u32 x)
{
   x = x ^ (x >> 1);
   x = x ^ (x >> 2);
   x = x ^ (x >> 4);
   x = x ^ (x >> 8);
   x = x ^ (x >> 16);
   return x & 1;
}

u32 parity(u64 x)
{
   x = x ^ (x >> 1);
   x = x ^ (x >> 2);
   x = x ^ (x >> 4);
   x = x ^ (x >> 8);
   x = x ^ (x >> 16);
   x = x ^ (x >> 32);
   return x & 1;
}

unsigned int hamming_checkbits(u32 u)
{

   /* Computes the six parity check bits for the
   "information" bits given in the 32-bit word u. The
   check bits are p[5:0]. On sending, an overall parity
   bit will be prepended to p (by another process).

   Bit   Checks these bits of u
   p[0]  0, 1, 3, 5, ..., 31 (0 and the odd positions).
   p[1]  0, 2-3, 6-7, ..., 30-31 (0 and positions xxx1x).
   p[2]  0, 4-7, 12-15, 20-23, 28-31 (0 and posns xx1xx).
   p[3]  0, 8-15, 24-31 (0 and positions x1xxx).
   p[4]  0, 16-31 (0 and positions 1xxxx).
   p[5]  1-31 */

   unsigned int p0, p1, p2, p3, p4, p5, p6, p;
   unsigned int t1, t2, t3;

   // First calculate p[5:0] ignoring u[0].
   p0 = u ^ (u >> 2);
   p0 = p0 ^ (p0 >> 4);
   p0 = p0 ^ (p0 >> 8);
   p0 = p0 ^ (p0 >> 16);        // p0 is in posn 1.

   t1 = u ^ (u >> 1);
   p1 = t1 ^ (t1 >> 4);
   p1 = p1 ^ (p1 >> 8);
   p1 = p1 ^ (p1 >> 16);        // p1 is in posn 2.

   t2 = t1 ^ (t1 >> 2);
   p2 = t2 ^ (t2 >> 8);
   p2 = p2 ^ (p2 >> 16);        // p2 is in posn 4.

   t3 = t2 ^ (t2 >> 4);
   p3 = t3 ^ (t3 >> 16);        // p3 is in posn 8.

   p4 = t3 ^ (t3 >> 8);         // p4 is in posn 16.

   p5 = p4 ^ (p4 >> 16);        // p5 is in posn 0.

   p = ((p0>>1) & 1) | ((p1>>1) & 2) | ((p2>>2) & 4) |
       ((p3>>5) & 8) | ((p4>>12) & 16) | ((p5 & 1) << 5);

   p = p ^ (-(u & 1) & 0x3F);   // Now account for u[0].
   return p;
}

int hamming_correct(unsigned int pr, u32 *ur)
{

   /* This function looks at the received seven check
   bits and 32 information bits (pr and ur) and
   determines how many errors occurred (under the
   presumption that it must be 0, 1, or 2). It returns
   with 0, 1, or 2, meaning that no errors, one error, or
   two errors occurred. It corrects the information word
   received (ur) if there was one error in it. */

   unsigned int po, p, syn, b;

   po = parity(pr ^ *ur);       // Compute overall parity
                                // of the received data.
   p = hamming_checkbits(*ur);          // Calculate check bits
                                // for the received info.
   syn = p ^ (pr & 0x3F);       // Syndrome (exclusive of
                                // overall parity bit).
   if (po == 0) {
      if (syn == 0) return 0;   // If no errors, return 0.
      else return 2;            // Two errors,- return 2.
   }
                                // One error occurred.
   if (((syn - 1) & syn) == 0)  // If syn has zero or one
      return 1;                 // bits set, then the
                                // error is in the check
                                // bits or the overall
                                // parity bit (no
                                // correction required).

   // One error, and syn bits 5:0 tell where it is in ur.

   b = syn - 31 - (syn >> 5); // Map syn to range 0 to 31.
// if (syn == 0x1f) b = 0;    // (These two lines equiv.
// else b = syn & 0x1f;       // to the one line above.)
   *ur = *ur ^ (1 << b);      // Correct the bit.
   return 1;
}

unsigned int hamming_checkbits_with_parity_bit(u32 u)
{
	unsigned int p;

	p = hamming_checkbits(u);
	p = p | (parity(u ^ p) << 6);

	return(p);
}

typedef union {int ix; float x;} __u;

float rough_but_fast_rsqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as an int.
   __a.ix = 0x5f37642f - (__a.ix >> 1); // Initial guess.
   return __a.x;
}

float approx_rsqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   float xhalf = 0.5f*__a.x;
   __a.ix = 0x5f375a82 - (__a.ix >> 1); // Initial guess.
   __a.x = __a.x*(1.5f - xhalf*__a.x*__a.x);    // Newton step.
   return __a.x;
}

float fast_rsqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as an int.
   float xhalf = 0.5f*__a.x;
   __a.ix = 0x5f37599e - (__a.ix >> 1); // Initial guess.
   __a.x = __a.x*(1.5f - xhalf*__a.x*__a.x);    // Newton step.
   __a.x = __a.x*(1.5f - xhalf*__a.x*__a.x);    // Newton step again.
   return __a.x;
}

float rough_but_fast_sqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = 0x1fbb4f2e + (__a.ix >> 1); // Initial guess.
   return __a.x;
}

float approx_sqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = 0x1fbb67a8 + (__a.ix >> 1); // Initial guess.
   __a.x = 0.5f*(__a.x + x0/__a.x);         // Newton step.
   return __a.x;
}

float fast_sqrt(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = 0x1fbb3f80 + (__a.ix >> 1); // Initial guess.
   __a.x = 0.5f*(__a.x + x0/__a.x);         // Newton step.
   __a.x = 0.5f*(__a.x + x0/__a.x);         // Newton step again.
   return __a.x;
}

float cuberoot_0(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = __a.ix/4 + __a.ix/16;           // Approximate divide by 3.
   __a.ix = __a.ix + __a.ix/16;
   __a.ix = 0x2a6497f8 + __a.ix;        // Initial guess.
   return __a.x;
}

float cuberoot_1(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = __a.ix/4 + __a.ix/16;           // Approximate divide by 3.
   __a.ix = __a.ix + __a.ix/16;
   __a.ix = __a.ix + __a.ix/256;
   __a.ix = 0x2a5137a0 + __a.ix;        // Initial guess.
   __a.x = 0.33333333f*(2.0f*__a.x + x0/(__a.x*__a.x));  // Newton step.
   return __a.x;
}

float cuberoot_2(float x0)
{
   __u __a;

   __a.x = x0;                      // x can be viewed as int.
   __a.ix = __a.ix/4 + __a.ix/16;           // Approximate divide by 3.
   __a.ix = __a.ix + __a.ix/16;
   __a.ix = __a.ix + __a.ix/256;
   __a.ix = 0x2a5137a0 + __a.ix;        // Initial guess.
   __a.x = 0.33333333f*(2.0f*__a.x + x0/(__a.x*__a.x));  // Newton step.
   __a.x = 0.33333333f*(2.0f*__a.x + x0/(__a.x*__a.x));  // Newton step again.
   return __a.x;
}

/* end bits.cpp */