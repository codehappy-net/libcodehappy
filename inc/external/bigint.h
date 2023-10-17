/*
    ===========================================================================
    BigInt
    ===========================================================================
    Definition for the BigInt class.
*/

#ifndef BIG_INT_HPP
#define BIG_INT_HPP

#include <iostream>
#include <tuple>

class BigInt {
    std::string value;
    char sign;

    public:
        // Constructors:
        BigInt();
        BigInt(const BigInt&);
        BigInt(const long long&);
        BigInt(const std::string&);

        // Assignment operators:
        BigInt& operator=(const BigInt&);
        BigInt& operator=(const long long&);
        BigInt& operator=(const std::string&);

        // Unary arithmetic operators:
        BigInt operator+() const;   // unary +
        BigInt operator-() const;   // unary -

        // Binary arithmetic operators:
        BigInt operator+(const BigInt&) const;
        BigInt operator-(const BigInt&) const;
        BigInt operator*(const BigInt&) const;
        BigInt operator/(const BigInt&) const;
        BigInt operator%(const BigInt&) const;
        BigInt operator+(const long long&) const;
        BigInt operator-(const long long&) const;
        BigInt operator*(const long long&) const;
        BigInt operator/(const long long&) const;
        BigInt operator%(const long long&) const;
        BigInt operator+(const std::string&) const;
        BigInt operator-(const std::string&) const;
        BigInt operator*(const std::string&) const;
        BigInt operator/(const std::string&) const;
        BigInt operator%(const std::string&) const;

        // Arithmetic-assignment operators:
        BigInt& operator+=(const BigInt&);
        BigInt& operator-=(const BigInt&);
        BigInt& operator*=(const BigInt&);
        BigInt& operator/=(const BigInt&);
        BigInt& operator%=(const BigInt&);
        BigInt& operator+=(const long long&);
        BigInt& operator-=(const long long&);
        BigInt& operator*=(const long long&);
        BigInt& operator/=(const long long&);
        BigInt& operator%=(const long long&);
        BigInt& operator+=(const std::string&);
        BigInt& operator-=(const std::string&);
        BigInt& operator*=(const std::string&);
        BigInt& operator/=(const std::string&);
        BigInt& operator%=(const std::string&);

        // Increment and decrement operators:
        BigInt& operator++();       // pre-increment
        BigInt& operator--();       // pre-decrement
        BigInt operator++(int);     // post-increment
        BigInt operator--(int);     // post-decrement

        // Relational operators:
        bool operator<(const BigInt&) const;
        bool operator>(const BigInt&) const;
        bool operator<=(const BigInt&) const;
        bool operator>=(const BigInt&) const;
        bool operator==(const BigInt&) const;
        bool operator!=(const BigInt&) const;
        bool operator<(const long long&) const;
        bool operator>(const long long&) const;
        bool operator<=(const long long&) const;
        bool operator>=(const long long&) const;
        bool operator==(const long long&) const;
        bool operator!=(const long long&) const;
        bool operator<(const std::string&) const;
        bool operator>(const std::string&) const;
        bool operator<=(const std::string&) const;
        bool operator>=(const std::string&) const;
        bool operator==(const std::string&) const;
        bool operator!=(const std::string&) const;

        // I/O stream operators:
        friend std::istream& operator>>(std::istream&, BigInt&);
        friend std::ostream& operator<<(std::ostream&, const BigInt&);

        // Conversion functions:
        std::string to_string() const;
        int to_int() const;
        long to_long() const;
        long long to_long_long() const;

        // Random number generating functions:
        friend BigInt big_random(size_t);
};

extern BigInt abs(const BigInt& num);
extern BigInt big_pow10(size_t exp);
extern BigInt pow(const BigInt& base, int exp);
extern BigInt pow(const long long& base, int exp);
extern BigInt pow(const std::string& base, int exp);
extern BigInt sqrt(const BigInt& num);
extern BigInt gcd(const BigInt &num1, const BigInt &num2);
extern BigInt gcd(const BigInt& num1, const long long& num2);
extern BigInt gcd(const BigInt& num1, const std::string& num2);
extern BigInt gcd(const long long& num1, const BigInt& num2);
extern BigInt gcd(const std::string& num1, const BigInt& num2);
extern BigInt lcm(const BigInt& num1, const BigInt& num2);
extern BigInt lcm(const BigInt& num1, const long long& num2);
extern BigInt lcm(const BigInt& num1, const std::string& num2);
extern BigInt lcm(const long long& num1, const BigInt& num2);
extern BigInt lcm(const std::string& num1, const BigInt& num2);
extern BigInt big_random(size_t num_digits = 0);
extern bool is_valid_number(const std::string& num);
extern void strip_leading_zeroes(std::string& num);
extern void add_leading_zeroes(std::string& num, size_t num_zeroes);
extern void add_trailing_zeroes(std::string& num, size_t num_zeroes);
extern std::tuple<std::string, std::string> get_larger_and_smaller(const std::string& num1,
        const std::string& num2);
extern bool is_power_of_10(const std::string& num);
extern BigInt operator+(const long long& lhs, const BigInt& rhs);
extern BigInt operator-(const long long& lhs, const BigInt& rhs);
extern BigInt operator*(const long long& lhs, const BigInt& rhs);
extern BigInt operator/(const long long& lhs, const BigInt& rhs);
extern BigInt operator%(const long long& lhs, const BigInt& rhs);
extern BigInt operator+(const std::string& lhs, const BigInt& rhs);
extern BigInt operator-(const std::string& lhs, const BigInt& rhs);
extern BigInt operator*(const std::string& lhs, const BigInt& rhs);
extern BigInt operator/(const std::string& lhs, const BigInt& rhs);
extern BigInt operator%(const std::string& lhs, const BigInt& rhs);
extern std::istream& operator>>(std::istream& in, BigInt& num);
extern std::ostream& operator<<(std::ostream& out, const BigInt& num);
extern bool operator==(const long long& lhs, const BigInt& rhs);
extern bool operator!=(const long long& lhs, const BigInt& rhs);
extern bool operator<(const long long& lhs, const BigInt& rhs);
extern bool operator>(const long long& lhs, const BigInt& rhs);
extern bool operator<=(const long long& lhs, const BigInt& rhs);
extern bool operator>=(const long long& lhs, const BigInt& rhs);
extern bool operator==(const std::string& lhs, const BigInt& rhs);
extern bool operator!=(const std::string& lhs, const BigInt& rhs);
extern bool operator<(const std::string& lhs, const BigInt& rhs);
extern bool operator>(const std::string& lhs, const BigInt& rhs);
extern bool operator<=(const std::string& lhs, const BigInt& rhs);
extern bool operator>=(const std::string& lhs, const BigInt& rhs);

#endif  // BIG_INT_HPP
