/***

	overflow.cpp

	Integer arithmetic with saturation (i.e., if the result is too large to 
	fit into the data type, they return INT32_MAX, INT64_MAX, etc.)

	Copyright (c) 2022 Chris Street.

***/
#include "libcodehappy.h"
#include <limits.h>

u32 add_saturate(u32 a1, u32 a2) {
	u32 ret = a1 + a2;
	/* Unsigned integer overflow is defined in the standard, so this check should work */
	if (ret < a1) {
		ret = UINT32_MAX;
	}
	return ret;
}

u32 sub_saturate(u32 s1, u32 s2) {
	if (s2 > s1)
		return 0UL;
	return s1 - s2;
}

u32 mul_saturate(u32 m1, u32 m2) {
	u64 ret = (u64)m1 * (u64)m2;
	if (ret > (u64)UINT32_MAX)
		return UINT32_MAX;
	return (u32)ret;
}

int add_saturate(int a1, int a2) {
	i64 ret = (i64)a1 + (i64)a2;
	if (ret > INT32_MAX)
		return INT32_MAX;
	if (ret < INT32_MIN)
		return INT32_MIN;
	return (int)ret;
}

int sub_saturate(int s1, int s2) {
	i64 ret = (i64)s1 - (i64)s2;
	if (ret > INT32_MAX)
		return INT32_MAX;
	if (ret < INT32_MIN)
		return INT32_MIN;
	return (int)ret;
}

int mul_saturate(int m1, int m2) {
	i64 ret = (i64)m1 * (i64)m2;
	if (ret > INT32_MAX)
		return INT32_MAX;
	if (ret < INT32_MIN)
		return INT32_MIN;
	return (int)ret;
}

u64 add_saturate(u64 a1, u64 a2) {
	u64 ret = a1 + a2;
	/* Unsigned integer overflow is defined in the standard, so this check should work */
	if (ret < a1) {
		ret = UINT64_MAX;
	}
	return ret;
}

u64 sub_saturate(u64 s1, u64 s2) {
	if (s2 > s1)
		return 0ULL;
	return s1 - s2;
}

u64 mul_saturate(u64 m1, u64 m2) {
	u128 ret = m1 * m2;
	if (ret > UINT64_MAX)
		ret = UINT64_MAX;
	return (u64)ret;
}

static void abs_i64(u128& out, i64 in) {
	if (in >= 0) {
		out = in;
		return;
	}
	if (INT64_MIN == in) {
		out = -(in + 1);
		++out;
		return;
	}
	out = -in;
	return;
}

i64 add_saturate(i64 a1, i64 a2) {
	if (a1 >= 0 && a2 < 0) {
		return a1 + a2;
	}
	if (a1 < 0 && a2 >= 0) {
		return a1 + a2;
	}
	u128 n1, n2;
	if (a1 < 0) {
		abs_i64(n1, a1);
		abs_i64(n2, a2);
		n1 += n2;
		--n1;
		if (n1 > INT64_MAX)
			return INT64_MIN;
		return a1 + a2;
	}
	n1 = a1;
	n1 += a2;
	if (n1 > INT64_MAX)
		return INT64_MAX;
	return a1 + a2;
}

i64 sub_saturate(i64 s1, i64 s2) {
	u128 n1, n2;
	if (s1 == 0) {
		if (s2 == INT64_MIN)
			return INT64_MAX;
		return -s2;
	}
	if (s1 < 0 && s2 >= 0) {
		if (s1 == INT64_MIN)
			return INT64_MIN;
		abs_i64(n1, s1);
		n1 += s2;
		--n1;
		if (n1 > INT64_MAX)
			return INT64_MIN;
		return s1 - s2;
	}
	if (s1 > 0 && s2 < 0) {
		if (s2 == INT64_MIN)
			return INT64_MAX;
		n1 = s1;
		abs_i64(n2, s2);
		n1 += s2;
		if (n1 > INT64_MAX)
			return INT64_MAX;
	}
	return s1 - s2;
}

i64 mul_saturate(i64 m1, i64 m2) {
	u128 n1, n2;
	bool sign = ((m1 < 0) || (m2 < 0)) && !(m1 < 0 && m2 < 0);
	abs_i64(n1, m1);
	abs_i64(n2, m2);
	n1 *= n2;
	if (sign)
		--n1;
	if (n1 > INT64_MAX)
		return (sign ? INT64_MIN : INT64_MAX);
	return m1 * m2;
}

u16 add_saturate(u16 a1, u16 a2) {
	u32 n1 = a1, n2 = a2;
	n1 += n2;
	if (n1 > UINT16_MAX)
		return UINT16_MAX;
	return (u16)n1;
}

u16 sub_saturate(u16 s1, u16 s2) {
	if (s2 > s1)
		return 0;
	return s1 - s2;
}

u16 mul_saturate(u16 m1, u16 m2) {
	u32 n1 = m1, n2 = m2;
	n1 *= m2;
	if (n1 > UINT16_MAX)
		return UINT16_MAX;
	return (u16)n1;
}

i16 add_saturate(i16 a1, i16 a2) {
	int n1 = a1, n2 = a2;
	n1 += n2;
	n1 = CLAMP(n1, INT16_MIN, INT16_MAX);
	return (i16)n1;
}

i16 sub_saturate(i16 s1, i16 s2) {
	int n1 = s1, n2 = s2;
	n1 -= n2;
	n1 = CLAMP(n1, INT16_MIN, INT16_MAX);
	return (i16)n1;
}

i16 mul_saturate(i16 m1, i16 m2) {
	i64 n1 = m1, n2 = m2;
	n1 *= n2;
	n1 = CLAMP(n1, INT16_MIN, INT16_MAX);
	return (i16)n1;
}

u8 add_saturate(u8 a1, u8 a2) {
	int n1 = a1, n2 = a2;
	n1 += n2;
	n1 = CLAMP(n1, 0, 255);
	return (u8)n1;
}

u8 sub_saturate(u8 s1, u8 s2) {
	int n1 = s1, n2 = s2;
	n1 -= n2;
	n1 = CLAMP(n1, 0, 255);
	return (u8)n1;
}

u8 mul_saturate(u8 m1, u8 m2) {
	int n1 = m1, n2 = m2;
	n1 *= n2;
	n1 = CLAMP(n1, 0, 255);
	return (u8)n1;
}

i8 add_saturate(i8 a1, i8 a2) {
	int n1 = a1, n2 = a2;
	n1 += n2;
	n1 = CLAMP(n1, -128, 127);
	return (i8)n1;
}

i8 sub_saturate(i8 s1, i8 s2) {
	int n1 = s1, n2 = s2;
	n1 -= n2;
	n1 = CLAMP(n1, -128, 127);
	return (i8)n1;
}

i8 mul_saturate(i8 m1, i8 m2) {
	int n1 = m1, n2 = m2;
	n1 *= n2;
	n1 = CLAMP(n1, -128, 127);
	return (i8)n1;
}

/* Overloaded operators with int types on the left-hand side */

const SaturatedI64 operator+(i64 lhs, const SaturatedI64& rhs) {
	return SaturatedI64(lhs) + rhs;
}

const SaturatedI64 operator-(i64 lhs, const SaturatedI64& rhs) {
	return SaturatedI64(lhs) - rhs;
}

const SaturatedI64 operator*(i64 lhs, const SaturatedI64& rhs) {
	return SaturatedI64(lhs) * rhs;
}

const SaturatedI64 operator/(i64 lhs, const SaturatedI64& rhs) {
	return SaturatedI64(lhs) / rhs;
}
const SaturatedI32 operator+(int lhs, const SaturatedI32& rhs) {
	return SaturatedI32((i32)lhs) + rhs;
}

const SaturatedI32 operator-(int lhs, const SaturatedI32& rhs) {
	return SaturatedI32((i32)lhs) - rhs;
}

const SaturatedI32 operator*(int lhs, const SaturatedI32& rhs) {
	return SaturatedI32((i32)lhs) * rhs;
}

const SaturatedI32 operator/(int lhs, const SaturatedI32& rhs) {
	return SaturatedI32((i32)lhs) / rhs;
}

const SaturatedI16 operator+(int lhs, const SaturatedI16& rhs) {
	return SaturatedI16((i16)lhs) + rhs;
}

const SaturatedI16 operator-(int lhs, const SaturatedI16& rhs) {
	return SaturatedI16((i16)lhs) - rhs;
}

const SaturatedI16 operator*(int lhs, const SaturatedI16& rhs) {
	return SaturatedI16((i16)lhs) * rhs;
}

const SaturatedI16 operator/(int lhs, const SaturatedI16& rhs) {
	return SaturatedI16((i16)lhs) / rhs;
}

const SaturatedI8 operator+(int lhs, const SaturatedI8& rhs) {
	return SaturatedI8((i8)lhs) + rhs;
}

const SaturatedI8 operator-(int lhs, const SaturatedI8& rhs) {
	return SaturatedI8((i8)lhs) - rhs;
}

const SaturatedI8 operator*(int lhs, const SaturatedI8& rhs) {
	return SaturatedI8((i8)lhs) * rhs;
}

const SaturatedI8 operator/(int lhs, const SaturatedI8& rhs) {
	return SaturatedI8((i8)lhs) / rhs;
}

const SaturatedU64 operator+(u64 lhs, const SaturatedU64& rhs) {
	return SaturatedU64(lhs) + rhs;
}

const SaturatedU64 operator-(u64 lhs, const SaturatedU64& rhs) {
	return SaturatedU64(lhs) - rhs;
}

const SaturatedU64 operator*(u64 lhs, const SaturatedU64& rhs) {
	return SaturatedU64(lhs) * rhs;
}

const SaturatedU64 operator/(u64 lhs, const SaturatedU64& rhs) {
	return SaturatedU64(lhs) / rhs;
}

const SaturatedU32 operator+(unsigned int lhs, const SaturatedU32& rhs) {
	return SaturatedU32((u32)lhs) + rhs;
}

const SaturatedU32 operator-(unsigned int lhs, const SaturatedU32& rhs) {
	return SaturatedU32((u32)lhs) - rhs;
}

const SaturatedU32 operator*(unsigned int lhs, const SaturatedU32& rhs) {
	return SaturatedU32((u32)lhs) * rhs;
}

const SaturatedU32 operator/(unsigned int lhs, const SaturatedU32& rhs) {
	return SaturatedU32((u32)lhs) / rhs;
}

const SaturatedU16 operator+(unsigned int lhs, const SaturatedU16& rhs) {
	return SaturatedU16((u16)lhs) + rhs;
}

const SaturatedU16 operator-(unsigned int lhs, const SaturatedU16& rhs) {
	return SaturatedU16((u16)lhs) - rhs;
}

const SaturatedU16 operator*(unsigned int lhs, const SaturatedU16& rhs) {
	return SaturatedU16((u16)lhs) * rhs;
}

const SaturatedU16 operator/(unsigned int lhs, const SaturatedU16& rhs) {
	return SaturatedU16((u16)lhs) / rhs;
}

const SaturatedU8 operator+(unsigned int lhs, const SaturatedU8& rhs) {
	return SaturatedU8((u8)lhs) + rhs;
}

const SaturatedU8 operator-(unsigned int lhs, const SaturatedU8& rhs) {
	return SaturatedU8((u8)lhs) - rhs;
}

const SaturatedU8 operator*(unsigned int lhs, const SaturatedU8& rhs) {
	return SaturatedU8((u8)lhs) * rhs;
}

const SaturatedU8 operator/(unsigned int lhs, const SaturatedU8& rhs) {
	return SaturatedU8((u8)lhs) / rhs;
}

/*** end overflow.cpp ***/