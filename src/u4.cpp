/***

	u4.cpp

	Unsigned 4-bit integer, in both normal and saturating form.

	Copyright (c) 2022 Chris Street.

***/

u4 u4::operator=(int rhs) {
	v = (u8)(rhs & 0x0f);
	return *this;
}

u4 u4::operator=(unsigned int rhs) {
	v = (u8)(rhs & 0x0f);
	return *this;
}

u4 u4::operator+(const u4 rhs) const {
	unsigned int nv = (unsigned int)v + (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator-(const u4 rhs) const {
	unsigned int nv = (unsigned int)v - (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator*(const u4 rhs) const {
	unsigned int nv = (unsigned int)v * (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator/(const u4 rhs) const {
	unsigned int nv = (unsigned int)v / (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator|(const u4 rhs) const {
	unsigned int nv = (unsigned int)v | (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator&(const u4 rhs) const {
	unsigned int nv = (unsigned int)v & (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator^(const u4 rhs) const {
	unsigned int nv = (unsigned int)v ^ (unsigned int)rhs.v;
	return u4(nv);
}

u4 u4::operator+(int rhs) const {
	int nv = (int)v + rhs;
	return u4(nv);
}

u4 u4::operator-(int rhs) const {
	int nv = (int)v - rhs;
	return u4(nv);
}

u4 u4::operator*(int rhs) const {
	int nv = (int)v * rhs;
	return u4(nv);
}

u4 u4::operator/(int rhs) const {
	int nv = (int)v / rhs;
	return u4(nv);
}

u4 u4::operator|(int rhs) const {
	int nv = (int)v | rhs;
	return u4(nv);
}

u4 u4::operator&(int rhs) const {
	int nv = (int)v & rhs;
	return u4(nv);
}

u4 u4::operator^(int rhs) const {
	int nv = (int)v ^ rhs;
	return u4(nv);
}

u4 u4::operator+(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v + rhs;
	return u4(nv);
}

u4 u4::operator-(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v - rhs;
	return u4(nv);
}

u4 u4::operator*(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v * rhs;
	return u4(nv);
}

u4 u4::operator/(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v / rhs;
	return u4(nv);
}

u4 u4::operator|(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v | rhs;
	return u4(nv);
}

u4 u4::operator&(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v & rhs;
	return u4(nv);
}

u4 u4::operator^(unsigned int rhs) const {
	unsigned int nv = (unsigned int)v ^ rhs;
	return u4(nv);
}

u4 u4::operator+=(const u4 rhs) {
	v += rhs.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator-=(const u4 rhs) {
	v -= rhs.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator*=(const u4 rhs) {
	v *= rhs.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator/=(const u4 rhs) {
	v /= rhs.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator&=(const u4 rhs) {
	v &= rhs.v;
	return *this;
}

u4 u4::operator|=(const u4 rhs) {
	v |= rhs.v;
	return *this;
}

u4 u4::operator^=(const u4 rhs) {
	v ^= rhs.v;
	return *this;
}

u4 u4::operator+=(int rhs) {
	u4 r(rhs);
	v += r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator-=(int rhs) {
	u4 r(rhs);
	v -= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator*=(int rhs) {
	u4 r(rhs);
	v *= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator/=(int rhs) {
	u4 r(rhs);
	v /= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator&=(int rhs) {
	v &= rhs;
	return *this;
}

u4 u4::operator|=(int rhs) {
	v |= rhs;
	v &= 0x0f;
	return *this;
}

u4 u4::operator^=(int rhs) {
	v ^= rhs;
	v &= 0x0f;
	return *this;
}

u4 u4::operator+=(unsigned int rhs) {
	u4 r(rhs);
	v += r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator-=(unsigned int rhs) {
	u4 r(rhs);
	v -= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator*=(unsigned int rhs) {
	u4 r(rhs);
	v *= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator/=(unsigned int rhs) {
	u4 r(rhs);
	v /= r.v;
	v &= 0x0f;
	return *this;
}

u4 u4::operator&=(unsigned int rhs) {
	v &= rhs;
	return *this;
}

u4 u4::operator|=(unsigned int rhs) {
	v |= rhs;
	v &= 0x0f;
	return *this;
}

u4 u4::operator^=(unsigned int rhs) {
	v ^= rhs;
	v &= 0x0f;
	return *this;
}

/* Print u4s to an ostream */
std::ostream& operator<<(std::ostream& lhs, u4 rhs) {
	int val = (int)rhs.v;
	return (lhs << val);
}

/* Do u4 arithmetic with signed or unsigned integer types on the left-hand side. */
int operator+(int lhs, u4 rhs) {
	return lhs + int(rhs.v);
}

int operator-(int lhs, u4 rhs) {
	return lhs - int(rhs.v);
}

int operator*(int lhs, u4 rhs) {
	return lhs * int(rhs.v);
}

int operator/(int lhs, u4 rhs) {
	return lhs / int(rhs.v);
}

int& operator=(int& lhs, u4 rhs) {
	lhs = int(rhs.v);
	return lhs;
}

int& operator+=(int& lhs, u4 rhs) {
	lhs += int(rhs.v);
	return lhs;
}

int& operator-=(int& lhs, u4 rhs) {
	lhs -= int(rhs.v);
	return lhs;
}

int& operator*=(int& lhs, u4 rhs) {
	lhs *= int(rhs.v);
	return lhs;
}

int& operator/=(int& lhs, u4 rhs) {
	lhs /= int(rhs.v);
	return lhs;
}

unsigned int operator+(unsigned int lhs, u4 rhs) {
	return lhs + (unsigned int)(rhs.v);
}

unsigned int operator-(unsigned int lhs, u4 rhs) {
	return lhs - (unsigned int)(rhs.v);
}

unsigned int operator*(unsigned int lhs, u4 rhs) {
	return lhs * (unsigned int)(rhs.v);
}

unsigned int operator/(unsigned int lhs, u4 rhs) {
	return lhs / (unsigned int)(rhs.v);
}

unsigned int& operator=(unsigned int& lhs, u4 rhs) {
	lhs = (unsigned int)(rhs.v);
	return lhs;
}

unsigned int& operator+=(unsigned int& lhs, u4 rhs) {
	lhs += (unsigned int)(rhs.v);
	return lhs;
}

unsigned int& operator-=(unsigned int& lhs, u4 rhs) {
	lhs -= (unsigned int)(rhs.v);
	return lhs;
}

unsigned int& operator*=(unsigned int& lhs, u4 rhs) {
	lhs *= (unsigned int)(rhs.v);
	return lhs;
}

unsigned int& operator/=(unsigned int& lhs, u4 rhs) {
	lhs /= (unsigned int)(rhs.v);
	return lhs;
}

/*** end u4.cpp ***/