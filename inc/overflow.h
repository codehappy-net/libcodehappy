/***

	overflow.h

	Integer arithmetic with saturation (i.e., if the result is too large to 
	fit into the data type, they return INT32_MAX, INT64_MAX, etc.)

	Copyright (c) 2022 Chris Street.

***/
#ifndef __OVERFLOW_H__
#define __OVERFLOW_H__

extern u8 add_saturate(u8 a1, u8 a2);
extern u8 sub_saturate(u8 s1, u8 s2);
extern u8 mul_saturate(u8 m1, u8 m2);
extern i8 add_saturate(i8 a1, i8 a2);
extern i8 sub_saturate(i8 s1, i8 s2);
extern i8 mul_saturate(i8 m1, i8 m2);
extern u16 add_saturate(u16 a1, u16 a2);
extern u16 sub_saturate(u16 s1, u16 s2);
extern u16 mul_saturate(u16 m1, u16 m2);
extern i16 add_saturate(i16 a1, i16 a2);
extern i16 sub_saturate(i16 s1, i16 s2);
extern i16 mul_saturate(i16 m1, i16 m2);
extern u32 add_saturate(u32 a1, u32 a2);
extern u32 sub_saturate(u32 s1, u32 s2);
extern u32 mul_saturate(u32 m1, u32 m2);
extern int add_saturate(int a1, int a2);
extern int sub_saturate(int s1, int s2);
extern int mul_saturate(int m1, int m2);
extern u64 add_saturate(u64 a1, u64 a2);
extern u64 sub_saturate(u64 s1, u64 s2);
extern u64 mul_saturate(u64 m1, u64 m2);
extern i64 add_saturate(i64 a1, i64 a2);
extern i64 sub_saturate(i64 s1, i64 s2);
extern i64 mul_saturate(i64 m1, i64 m2);

/*** Classes for saturated arithmetic types. A set of operators is defined for each. ***/

class SaturatedU8 {
public:
	SaturatedU8(u8 val) { v = val; }

	const SaturatedU8 operator+(const SaturatedU8& rhs) const {
		return SaturatedU8(add_saturate(v, rhs.v));
	}

	const SaturatedU8 operator+(u8 rhs) const {
		return SaturatedU8(add_saturate(v, rhs));
	}

	const SaturatedU8 operator*(const SaturatedU8& rhs) const {
		return SaturatedU8(mul_saturate(v, rhs.v));
	}

	const SaturatedU8 operator*(u8 rhs) const {
		return SaturatedU8(mul_saturate(v, rhs));
	}

	const SaturatedU8 operator-(const SaturatedU8& rhs) const {
		return SaturatedU8(sub_saturate(v, rhs.v));
	}

	const SaturatedU8 operator-(u8 rhs) const {
		return SaturatedU8(sub_saturate(v, rhs));
	}

	const SaturatedU8 operator/(const SaturatedU8& rhs) const {
		return SaturatedU8(v / rhs.v);
	}

	const SaturatedU8 operator/(u8 rhs) const {
		return SaturatedU8(v / rhs);
	}

	const SaturatedU8 operator=(const SaturatedU8& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedU8 operator=(u8 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedU8 operator+=(const SaturatedU8& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU8 operator+=(u8 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU8 operator*=(const SaturatedU8& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU8 operator*=(u8 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU8 operator-=(const SaturatedU8& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU8 operator-=(u8 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU8 operator/=(const SaturatedU8& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedU8 operator/=(u8 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator u8()           const { return v; }
	operator unsigned int() const { return (unsigned int)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	u8 v;
};

extern const SaturatedU8 operator+(unsigned int lhs, const SaturatedU8& rhs);
extern const SaturatedU8 operator-(unsigned int lhs, const SaturatedU8& rhs);
extern const SaturatedU8 operator*(unsigned int lhs, const SaturatedU8& rhs);
extern const SaturatedU8 operator/(unsigned int lhs, const SaturatedU8& rhs);

class SaturatedU16 {
public:
	SaturatedU16(u16 val) { v = val; }

	const SaturatedU16 operator+(const SaturatedU16& rhs) const {
		return SaturatedU16(add_saturate(v, rhs.v));
	}

	const SaturatedU16 operator+(u16 rhs) const {
		return SaturatedU16(add_saturate(v, rhs));
	}

	const SaturatedU16 operator*(const SaturatedU16& rhs) const {
		return SaturatedU16(mul_saturate(v, rhs.v));
	}

	const SaturatedU16 operator*(u16 rhs) const {
		return SaturatedU16(mul_saturate(v, rhs));
	}

	const SaturatedU16 operator-(const SaturatedU16& rhs) const {
		return SaturatedU16(sub_saturate(v, rhs.v));
	}

	const SaturatedU16 operator-(u16 rhs) const {
		return SaturatedU16(sub_saturate(v, rhs));
	}

	const SaturatedU16 operator/(const SaturatedU16& rhs) const {
		return SaturatedU16(v / rhs.v);
	}

	const SaturatedU16 operator/(u16 rhs) const {
		return SaturatedU16(v / rhs);
	}

	const SaturatedU16 operator=(const SaturatedU16& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedU16 operator=(u16 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedU16 operator+=(const SaturatedU16& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU16 operator+=(u16 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU16 operator*=(const SaturatedU16& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU16 operator*=(u16 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU16 operator-=(const SaturatedU16& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU16 operator-=(u16 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU16 operator/=(const SaturatedU16& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedU16 operator/=(u16 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator u16()          const { return v; }
	operator unsigned int() const { return (unsigned int)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	u16 v;
};

const SaturatedU16 operator+(unsigned int lhs, const SaturatedU16& rhs);
const SaturatedU16 operator-(unsigned int lhs, const SaturatedU16& rhs);
const SaturatedU16 operator*(unsigned int lhs, const SaturatedU16& rhs);
const SaturatedU16 operator/(unsigned int lhs, const SaturatedU16& rhs);

class SaturatedU32 {
public:
	SaturatedU32(u32 val) { v = val; }

	const SaturatedU32 operator+(const SaturatedU32& rhs) const {
		return SaturatedU32(add_saturate(v, rhs.v));
	}

	const SaturatedU32 operator+(u32 rhs) const {
		return SaturatedU32(add_saturate(v, rhs));
	}

	const SaturatedU32 operator*(const SaturatedU32& rhs) const {
		return SaturatedU32(mul_saturate(v, rhs.v));
	}

	const SaturatedU32 operator*(u32 rhs) const {
		return SaturatedU32(mul_saturate(v, rhs));
	}

	const SaturatedU32 operator-(const SaturatedU32& rhs) const {
		return SaturatedU32(sub_saturate(v, rhs.v));
	}

	const SaturatedU32 operator-(u32 rhs) const {
		return SaturatedU32(sub_saturate(v, rhs));
	}

	const SaturatedU32 operator/(const SaturatedU32& rhs) const {
		return SaturatedU32(v / rhs.v);
	}

	const SaturatedU32 operator/(u32 rhs) const {
		return SaturatedU32(v / rhs);
	}

	const SaturatedU32 operator=(const SaturatedU32& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedU32 operator=(u32 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedU32 operator+=(const SaturatedU32& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU32 operator+=(u32 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU32 operator*=(const SaturatedU32& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU32 operator*=(u32 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU32 operator-=(const SaturatedU32& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU32 operator-=(u32 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU32 operator/=(const SaturatedU32& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedU32 operator/=(u32 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator u32()          const { return v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	u32 v;
};

extern const SaturatedU32 operator+(unsigned int lhs, const SaturatedU32& rhs);
extern const SaturatedU32 operator-(unsigned int lhs, const SaturatedU32& rhs);
extern const SaturatedU32 operator*(unsigned int lhs, const SaturatedU32& rhs);
extern const SaturatedU32 operator/(unsigned int lhs, const SaturatedU32& rhs);

class SaturatedU64 {
public:
	SaturatedU64(u64 val) { v = val; }

	const SaturatedU64 operator+(const SaturatedU64& rhs) const {
		return SaturatedU64(add_saturate(v, rhs.v));
	}

	const SaturatedU64 operator+(u64 rhs) const {
		return SaturatedU64(add_saturate(v, rhs));
	}

	const SaturatedU64 operator*(const SaturatedU64& rhs) const {
		return SaturatedU64(mul_saturate(v, rhs.v));
	}

	const SaturatedU64 operator*(u64 rhs) const {
		return SaturatedU64(mul_saturate(v, rhs));
	}

	const SaturatedU64 operator-(const SaturatedU64& rhs) const {
		return SaturatedU64(sub_saturate(v, rhs.v));
	}

	const SaturatedU64 operator-(u64 rhs) const {
		return SaturatedU64(sub_saturate(v, rhs));
	}

	const SaturatedU64 operator/(const SaturatedU64& rhs) const {
		return SaturatedU64(v / rhs.v);
	}

	const SaturatedU64 operator/(u64 rhs) const {
		return SaturatedU64(v / rhs);
	}

	const SaturatedU64 operator=(const SaturatedU64& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedU64 operator=(u64 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedU64 operator+=(const SaturatedU64& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU64 operator+=(u64 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedU64 operator*=(const SaturatedU64& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU64 operator*=(u64 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedU64 operator-=(const SaturatedU64& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU64 operator-=(u64 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedU64 operator/=(const SaturatedU64& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedU64 operator/=(u64 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator u64()          const { return v; }
	operator i64()          const { return (i64)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	u64 v;
};

extern const SaturatedU64 operator+(u64 lhs, const SaturatedU64& rhs);
extern const SaturatedU64 operator-(u64 lhs, const SaturatedU64& rhs);
extern const SaturatedU64 operator*(u64 lhs, const SaturatedU64& rhs);
extern const SaturatedU64 operator/(u64 lhs, const SaturatedU64& rhs);

class SaturatedI8 {
public:
	SaturatedI8(i8 val) { v = val; }

	const SaturatedI8 operator+(const SaturatedI8& rhs) const {
		return SaturatedI8(add_saturate(v, rhs.v));
	}

	const SaturatedI8 operator+(i8 rhs) const {
		return SaturatedI8(add_saturate(v, rhs));
	}

	const SaturatedI8 operator*(const SaturatedI8& rhs) const {
		return SaturatedI8(mul_saturate(v, rhs.v));
	}

	const SaturatedI8 operator*(i8 rhs) const {
		return SaturatedI8(mul_saturate(v, rhs));
	}

	const SaturatedI8 operator-(const SaturatedI8& rhs) const {
		return SaturatedI8(sub_saturate(v, rhs.v));
	}

	const SaturatedI8 operator-(i8 rhs) const {
		return SaturatedI8(sub_saturate(v, rhs));
	}

	const SaturatedI8 operator/(const SaturatedI8& rhs) const {
		return SaturatedI8(v / rhs.v);
	}

	const SaturatedI8 operator/(i8 rhs) const {
		return SaturatedI8(v / rhs);
	}

	const SaturatedI8 operator=(const SaturatedI8& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedI8 operator=(i8 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedI8 operator+=(const SaturatedI8& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI8 operator+=(i8 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI8 operator*=(const SaturatedI8& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI8 operator*=(i8 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI8 operator-=(const SaturatedI8& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI8 operator-=(i8 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI8 operator/=(const SaturatedI8& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedI8 operator/=(i8 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator i8()           const { return v; }
	operator unsigned int() const { return (unsigned int)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	i8 v;
};

extern const SaturatedI8 operator+(int lhs, const SaturatedI8& rhs);
extern const SaturatedI8 operator-(int lhs, const SaturatedI8& rhs);
extern const SaturatedI8 operator*(int lhs, const SaturatedI8& rhs);
extern const SaturatedI8 operator/(int lhs, const SaturatedI8& rhs);

class SaturatedI16 {
public:
	SaturatedI16(i16 val) { v = val; }

	const SaturatedI16 operator+(const SaturatedI16& rhs) const {
		return SaturatedI16(add_saturate(v, rhs.v));
	}

	const SaturatedI16 operator+(i16 rhs) const {
		return SaturatedI16(add_saturate(v, rhs));
	}

	const SaturatedI16 operator*(const SaturatedI16& rhs) const {
		return SaturatedI16(mul_saturate(v, rhs.v));
	}

	const SaturatedI16 operator*(i16 rhs) const {
		return SaturatedI16(mul_saturate(v, rhs));
	}

	const SaturatedI16 operator-(const SaturatedI16& rhs) const {
		return SaturatedI16(sub_saturate(v, rhs.v));
	}

	const SaturatedI16 operator-(i16 rhs) const {
		return SaturatedI16(sub_saturate(v, rhs));
	}

	const SaturatedI16 operator/(const SaturatedI16& rhs) const {
		return SaturatedI16(v / rhs.v);
	}

	const SaturatedI16 operator/(i16 rhs) const {
		return SaturatedI16(v / rhs);
	}

	const SaturatedI16 operator=(const SaturatedI16& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedI16 operator=(i16 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedI16 operator+=(const SaturatedI16& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI16 operator+=(i16 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI16 operator*=(const SaturatedI16& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI16 operator*=(i16 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI16 operator-=(const SaturatedI16& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI16 operator-=(i16 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI16 operator/=(const SaturatedI16& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedI16 operator/=(i16 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator i16()           const { return v; }
	operator unsigned int() const { return (unsigned int)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	i16 v;
};

extern const SaturatedI16 operator+(int lhs, const SaturatedI16& rhs);
extern const SaturatedI16 operator-(int lhs, const SaturatedI16& rhs);
extern const SaturatedI16 operator*(int lhs, const SaturatedI16& rhs);
extern const SaturatedI16 operator/(int lhs, const SaturatedI16& rhs);

class SaturatedI32 {
public:
	SaturatedI32(i32 val) { v = val; }

	const SaturatedI32 operator+(const SaturatedI32& rhs) const {
		return SaturatedI32(add_saturate(v, rhs.v));
	}

	const SaturatedI32 operator+(i32 rhs) const {
		return SaturatedI32(add_saturate(v, rhs));
	}

	const SaturatedI32 operator*(const SaturatedI32& rhs) const {
		return SaturatedI32(mul_saturate(v, rhs.v));
	}

	const SaturatedI32 operator*(i32 rhs) const {
		return SaturatedI32(mul_saturate(v, rhs));
	}

	const SaturatedI32 operator-(const SaturatedI32& rhs) const {
		return SaturatedI32(sub_saturate(v, rhs.v));
	}

	const SaturatedI32 operator-(i32 rhs) const {
		return SaturatedI32(sub_saturate(v, rhs));
	}

	const SaturatedI32 operator/(const SaturatedI32& rhs) const {
		return SaturatedI32(v / rhs.v);
	}

	const SaturatedI32 operator/(i32 rhs) const {
		return SaturatedI32(v / rhs);
	}

	const SaturatedI32 operator=(const SaturatedI32& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedI32 operator=(i32 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedI32 operator+=(const SaturatedI32& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI32 operator+=(i32 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI32 operator*=(const SaturatedI32& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI32 operator*=(i32 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI32 operator-=(const SaturatedI32& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI32 operator-=(i32 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI32 operator/=(const SaturatedI32& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedI32 operator/=(i32 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator unsigned int() const { return (unsigned int)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	i32 v;
};

extern const SaturatedI32 operator+(int lhs, const SaturatedI32& rhs);
extern const SaturatedI32 operator-(int lhs, const SaturatedI32& rhs);
extern const SaturatedI32 operator*(int lhs, const SaturatedI32& rhs);
extern const SaturatedI32 operator/(int lhs, const SaturatedI32& rhs);

class SaturatedI64 {
public:
	SaturatedI64(i64 val) { v = val; }

	const SaturatedI64 operator+(const SaturatedI64& rhs) const {
		return SaturatedI64(add_saturate(v, rhs.v));
	}

	const SaturatedI64 operator+(i64 rhs) const {
		return SaturatedI64(add_saturate(v, rhs));
	}

	const SaturatedI64 operator*(const SaturatedI64& rhs) const {
		return SaturatedI64(mul_saturate(v, rhs.v));
	}

	const SaturatedI64 operator*(i64 rhs) const {
		return SaturatedI64(mul_saturate(v, rhs));
	}

	const SaturatedI64 operator-(const SaturatedI64& rhs) const {
		return SaturatedI64(sub_saturate(v, rhs.v));
	}

	const SaturatedI64 operator-(i64 rhs) const {
		return SaturatedI64(sub_saturate(v, rhs));
	}

	const SaturatedI64 operator/(const SaturatedI64& rhs) const {
		return SaturatedI64(v / rhs.v);
	}

	const SaturatedI64 operator/(i64 rhs) const {
		return SaturatedI64(v / rhs);
	}

	const SaturatedI64 operator=(const SaturatedI64& rhs) {
		v = rhs.v;
		return *this;
	}

	const SaturatedI64 operator=(i64 rhs) {
		v = rhs;
		return *this;
	}

	const SaturatedI64 operator+=(const SaturatedI64& rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI64 operator+=(i64 rhs) {
		*this = (*this + rhs);
		return *this;
	}

	const SaturatedI64 operator*=(const SaturatedI64& rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI64 operator*=(i64 rhs) {
		*this = (*this * rhs);
		return *this;
	}

	const SaturatedI64 operator-=(const SaturatedI64& rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI64 operator-=(i64 rhs) {
		*this = (*this - rhs);
		return *this;
	}

	const SaturatedI64 operator/=(const SaturatedI64& rhs) {
		*this = (*this / rhs);
		return *this;
	}

	const SaturatedI64 operator/=(i64 rhs) {
		*this = (*this / rhs);
		return *this;
	}

	operator i64()          const { return v; }
	operator u64()          const { return (u64)v; }
	operator int()          const { return (int)v; }
	operator bool()         const { return (bool)v; }

private:
	i64 v;
};

extern const SaturatedI64 operator+(i64 lhs, const SaturatedI64& rhs);
extern const SaturatedI64 operator-(i64 lhs, const SaturatedI64& rhs);
extern const SaturatedI64 operator*(i64 lhs, const SaturatedI64& rhs);
extern const SaturatedI64 operator/(i64 lhs, const SaturatedI64& rhs);

#endif  // __OVERLOW_H__
/* end overflow.h */