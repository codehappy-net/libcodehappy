/***

	lalgebra.h

	Linear algebra algorithms, including vectors, matrices, quaternions, etc.

	Copyright (c) 2022 Chris Street.

***/
#ifndef __LALGEBRA_H__
#define __LALGEBRA_H__

template <typename _T_>
class Vector2 {
public:
	Vector2() {
		v[0] = 0;
		v[1] = 0;
	}

	Vector2(_T_ v1, _T_ v2) {
		v[0] = v1;
		v[1] = v2;
	}

	_T_ mag_sq(void) const {
		return v[0] * v[0] + v[1] * v[1];
	}

	_T_ mag(void) const {
		return (_T_)sqrt(mag_sq());
	}

	bool is_unit(void) const {
		return mag_sq() == 1;
	}

	void normalize(void) {
		_T_ m = mag();
		if (0 == m)
			return;
		v[0] /= m;
		v[1] /= m;
	}

	void unit(double ang_radians) {
		v[0] = cos(ang_radians);
		v[1] = sin(ang_radians);
	}

	_T_ dot_product(const Vector2& rhs) const {
		return v[0] * rhs.v[0] + v[1] * rhs.v[1];
	}

	Vector2 operator+(const Vector2& rhs) const {
		Vector2 ret(v[0] + rhs.v[0], v[1] + rhs.v[1]);
		return ret;
	}

	Vector2 operator-(const Vector2& rhs) const {
		Vector2 ret(v[0] - rhs.v[0], v[1] - rhs.v[1]);
		return ret;
	}

	Vector2 operator*(const _T_& rhs) const {
		Vector2 ret(v[0] * rhs, v[1] * rhs);
		return ret;
	}

	Vector2 operator/(const _T_& rhs) const {
		Vector2 ret(v[0] / rhs, v[1] / rhs);
		return ret;
	}

	const Vector2& operator*=(_T_ rhs) {
		v[0] *= rhs;
		v[1] *= rhs;
		return *this;
	}

	const Vector2& operator/=(_T_ rhs) {
		v[0] /= rhs;
		v[1] /= rhs;
		return *this;
	}

	const Vector2& operator+=(const Vector2& rhs) {
		v[0] += rhs.v[0];
		v[1] += rhs.v[1];
		return *this;
	}

	const Vector2& operator-=(const Vector2& rhs) {
		v[0] -= rhs.v[0];
		v[1] -= rhs.v[1];
		return *this;
	}

	_T_ v[2];
};


template <typename _T_>
class Vector3 {
public:
	Vector3() {
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
	}

	Vector3(_T_ v1, _T_ v2, _T_ v3) {
		v[0] = v1;
		v[1] = v2;
		v[2] = v3;
	}

	_T_ mag_sq(void) const {
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	}

	_T_ mag(void) const {
		return (_T_)sqrt(mag_sq());
	}

	bool is_unit(void) const {
		return mag_sq() == 1;
	}

	void normalize(void) {
		_T_ m = mag();
		if (0 == m)
			return;
		v[0] /= m;
		v[1] /= m;
		v[2] /= m;
	}

	void unit(double theta_radians, double phi_radians) {
		v[0] = cos(phi_radians) * sin(theta_radians);
		v[1] = sin(phi_radians) * sin(theta_radians);
		v[2] = cos(theta_radians);
	}

	_T_ dot_product(const Vector3& rhs) const {
		return v[0] * rhs.v[0] + v[1] * rhs.v[1] + v[2] * rhs.v[2];
	}

	Vector3 operator+(const Vector3& rhs) const {
		Vector3 ret(v[0] + rhs.v[0], v[1] + rhs.v[1], v[2] + rhs.v[2]);
		return ret;
	}

	Vector3 operator-(const Vector3& rhs) const {
		Vector3 ret(v[0] - rhs.v[0], v[1] - rhs.v[1], v[2] - rhs.v[2]);
		return ret;
	}

	Vector3 operator*(const _T_& rhs) const {
		Vector3 ret(v[0] * rhs, v[1] * rhs, v[2] * rhs);
		return ret;
	}

	Vector3 operator/(const _T_& rhs) const {
		Vector3 ret(v[0] / rhs, v[1] / rhs, v[2] / rhs);
		return ret;
	}

	const Vector3& operator*=(_T_ rhs) {
		v[0] *= rhs;
		v[1] *= rhs;
		v[2] *= rhs;
		return *this;
	}

	const Vector3& operator/=(_T_ rhs) {
		v[0] /= rhs;
		v[1] /= rhs;
		v[2] /= rhs;
		return *this;
	}

	const Vector3& operator+=(const Vector3& rhs) {
		v[0] += rhs.v[0];
		v[1] += rhs.v[1];
		v[2] += rhs.v[2];
		return *this;
	}

	const Vector3& operator-=(const Vector3& rhs) {
		v[0] -= rhs.v[0];
		v[1] -= rhs.v[1];
		v[2] -= rhs.v[2];
		return *this;
	}

	_T_ v[3];
};


template <typename _T_>
class Vector4 {
public:
	Vector4() {
		v[0] = 0;
		v[1] = 0;
		v[2] = 0;
		v[3] = 0;
	}

	Vector4(_T_ v1, _T_ v2, _T_ v3, _T_ v4) {
		v[0] = v1;
		v[1] = v2;
		v[2] = v3;
		v[3] = v4;
	}

	_T_ mag_sq(void) const {
		return v[0] * v[0] + v[1] * v[1] + v[2] * v[2] + v[3] * v[3];
	}

	_T_ mag(void) const {
		return (_T_)sqrt(mag_sq());
	}

	bool is_unit(void) const {
		return mag_sq() == 1;
	}

	void normalize(void) {
		_T_ m = mag();
		if (0 == m)
			return;
		v[0] /= m;
		v[1] /= m;
		v[2] /= m;
		v[3] /= m;
	}

	_T_ dot_product(const Vector3& rhs) const {
		return v[0] * rhs.v[0] + v[1] * rhs.v[1] + v[2] * rhs.v[2] + v[3] * rhs.v[3];
	}

	Vector4 operator+(const Vector4& rhs) const {
		Vector4 ret(v[0] + rhs.v[0], v[1] + rhs.v[1], v[2] + rhs.v[2], v[3] + rhs.v[3]);
		return ret;
	}

	Vector4 operator-(const Vector4& rhs) const {
		Vector4 ret(v[0] - rhs.v[0], v[1] - rhs.v[1], v[2] - rhs.v[2], v[3] - rhs.v[4]);
		return ret;
	}

	Vector4 operator*(const _T_& rhs) const {
		Vector4 ret(v[0] * rhs, v[1] * rhs, v[2] * rhs, v[3] * rhs);
		return ret;
	}

	Vector4 operator/(const _T_& rhs) const {
		Vector4 ret(v[0] / rhs, v[1] / rhs, v[2] / rhs, v[3] / rhs);
		return ret;
	}

	const Vector4& operator*=(_T_ rhs) {
		v[0] *= rhs;
		v[1] *= rhs;
		v[2] *= rhs;
		v[3] *= rhs; 
		return *this;
	}

	const Vector4& operator/=(_T_ rhs) {
		v[0] /= rhs;
		v[1] /= rhs;
		v[2] /= rhs;
		v[3] /= rhs;
		return *this;
	}

	const Vector4& operator+=(const Vector4& rhs) {
		v[0] += rhs.v[0];
		v[1] += rhs.v[1];
		v[2] += rhs.v[2];
		v[3] += rhs.v[3];
		return *this;
	}

	const Vector4& operator-=(const Vector4& rhs) {
		v[0] -= rhs.v[0];
		v[1] -= rhs.v[1];
		v[2] -= rhs.v[2];
		v[3] -= rhs.v[3];
		return *this;
	}

	_T_ v[3];
};


#endif  // __LALGEBRA_H__