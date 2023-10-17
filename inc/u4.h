/***

	u4.h

	Unsigned 4-bit integer, in both normal and saturating form.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __U4_H__
#define __U4_H__

class u4 {
public:
	/* Constructors. */
	u4()                { v = 0; }
	u4(unsigned int v_) { v = (u8)(v_ & 0x0f); }
	u4(int v_)          { v = (u8)(v_ & 0x0f); }

	/* Assignment operators. */
	u4 operator=(int rhs);
	u4 operator=(unsigned int rhs);

	/* Arithmetic and bitwise operators. */
	u4 operator+(const u4 rhs) const;
	u4 operator-(const u4 rhs) const;
	u4 operator*(const u4 rhs) const;
	u4 operator/(const u4 rhs) const;
	u4 operator|(const u4 rhs) const;
	u4 operator&(const u4 rhs) const;
	u4 operator^(const u4 rhs) const;
	u4 operator+(int rhs) const;
	u4 operator-(int rhs) const;
	u4 operator*(int rhs) const;
	u4 operator/(int rhs) const;
	u4 operator|(int rhs) const;
	u4 operator&(int rhs) const;
	u4 operator^(int rhs) const;
	u4 operator+(unsigned int rhs) const;
	u4 operator-(unsigned int rhs) const;
	u4 operator*(unsigned int rhs) const;
	u4 operator/(unsigned int rhs) const;
	u4 operator|(unsigned int rhs) const;
	u4 operator&(unsigned int rhs) const;
	u4 operator^(unsigned int rhs) const;

	/* Arithmetic/bitwise assignment operators. */
	u4 operator+=(const u4 rhs);
	u4 operator-=(const u4 rhs);
	u4 operator*=(const u4 rhs);
	u4 operator/=(const u4 rhs);
	u4 operator&=(const u4 rhs);
	u4 operator|=(const u4 rhs);
	u4 operator^=(const u4 rhs);
	u4 operator+=(int rhs);
	u4 operator-=(int rhs);
	u4 operator*=(int rhs);
	u4 operator/=(int rhs);
	u4 operator&=(int rhs);
	u4 operator|=(int rhs);
	u4 operator^=(int rhs);
	u4 operator+=(unsigned int rhs);
	u4 operator-=(unsigned int rhs);
	u4 operator*=(unsigned int rhs);
	u4 operator/=(unsigned int rhs);
	u4 operator&=(unsigned int rhs);
	u4 operator|=(unsigned int rhs);
	u4 operator^=(unsigned int rhs);

	/* rather than defining a passel of friends, this can be public */
	u8 v;
};

/* Print u4s to an ostream */
extern std::ostream& operator<<(std::ostream& lhs, u4 rhs);

/* Do u4 arithmetic with signed or unsigned integer types on the left-hand side. */
extern int operator+(int lhs, u4 rhs);
extern int operator-(int lhs, u4 rhs);
extern int operator*(int lhs, u4 rhs);
extern int operator/(int lhs, u4 rhs);
//extern int& operator=(int& lhs, u4 rhs);
extern int& operator+=(int& lhs, u4 rhs);
extern int& operator-=(int& lhs, u4 rhs);
extern int& operator*=(int& lhs, u4 rhs);
extern int& operator/=(int& lhs, u4 rhs);
extern unsigned int operator+(unsigned int lhs, u4 rhs);
extern unsigned int operator-(unsigned int lhs, u4 rhs);
extern unsigned int operator*(unsigned int lhs, u4 rhs);
extern unsigned int operator/(unsigned int lhs, u4 rhs);
//extern unsigned int& operator=(unsigned int& lhs, u4 rhs);
extern unsigned int& operator+=(unsigned int& lhs, u4 rhs);
extern unsigned int& operator-=(unsigned int& lhs, u4 rhs);
extern unsigned int& operator*=(unsigned int& lhs, u4 rhs);
extern unsigned int& operator/=(unsigned int& lhs, u4 rhs);

#endif  // __U4_H__
/* end u4.h */