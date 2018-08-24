#pragma once

#include <cmath>
#include <HgMath.h>

namespace HgMath {

	//This is written to take advantage of auto vectorization

	template<typename T, int fcount>
	class vector {
	public:

		vector() : xyz{ 0,0,0,0 }
		{
		}

		vector(T w, T x, T y, T z) : xyz{ w,x,y,z }
		{
		}

		vector(const T _xyz[fcount])
		{
			for (int i = 0; i < fcount; i++) {
				xyz[i] = _xyz[i];
			}
		}

		vector(const T a)
		{
			for (int i = 0; i < fcount; i++) {
				xyz[i] = a;
			}
		}

		inline T operator[](int idx) const { return xyz[idx]; }
		inline T& operator[](int idx) { return xyz[idx]; }

		inline vector scale(T n) const {
			//Writing functions exactly like this lets the compiler make a lot of good optimizations.
			vector r = *this;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] *= n;
			}
			return r;
		}

		inline vector operator+(const vector& _rhs) const {
			vector r = *this;
			const vector rhs = _rhs;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] += rhs.xyz[i];
			}
			return r;
		}

		inline vector operator+=(const vector& _rhs) {
			const vector rhs = _rhs;
			for (int i = 0; i < fcount; i++) {
				xyz[i] += rhs.xyz[i];
			}
			return *this;
		}

		inline vector operator-(const vector& _rhs) const {
			vector r = *this;
			const vector rhs = _rhs;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] -= rhs.xyz[i];
			}
			return r;
		}

		inline vector operator*(const vector& _rhs) const {
			vector r = *this;
			const vector rhs = _rhs;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] *= rhs.xyz[i];
			}
			return r;
		}

		inline vector operator/(const vector& _rhs) const {
			vector r = *this;
			const vector rhs = _rhs;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] /= rhs.xyz[i];
			}
			return r;
		}

		//inline vector operator^(const vector& _rhs) const {
		//	vector r = *this;
		//	const vector rhs = _rhs;
		//	for (int i = 0; i < fcount; i++) {
		//		r.xyz[i] = r.xyz[i] ^ rhs.xyz[i];
		//	}
		//	return r;
		//}

		inline T dot(const vector& _rhs) const {
			//the 1st loop is auto vectorized
			T sum = 0;
			T r[fcount];
			const vector a = *this;
			const vector b = *this;
			for (int i = 0; i < 4; i++)
			{
				r[i] = a[i] * b[i];
			}
			for (int i = 0; i < 4; i++)
			{
				sum += r[i];
			}
			return sum;
		}

		inline T squaredLength() const {
			//the 1st loop is auto vectorized
			T sum = 0;
			T components[fcount];
			for (int i = 0; i < 4; i++)
			{
				components[i] = HgMath::square(xyz[i]);
			}
			for (int i = 0; i < 4; i++)
			{
				sum += components[i];
			}
			return sum;
		}

		inline T magnitude() const {
			return HgMath::sqrt(squaredLength());
		}

		inline vector normal() const {
			vector r = *this;
			T length = magnitude();
			if (length > 1e-06f) {
				r = this->scale(T(1.0) / length);
			}
			return r;
		}

		inline vector negate() const {
			vector r = *this;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] *= -1.0f;
			}
			return r;
		}

		inline bool isZeroLength() const
		{
			return (squaredLength() < HgMath::square(1e-06f)); //does squaredLength need to be double here if 1e-06 is double?
		}

		inline const T* raw() const { return xyz; }
		inline T* raw() { return xyz; }

		inline T w() const { return xyz[0]; }
		inline T x() const { return xyz[1]; }
		inline T y() const { return xyz[2]; }
		inline T z() const { return xyz[3]; }

	protected:

		T xyz[fcount];
	};
/*
	template<typename T>
	class vec3 : protected vector<T,4> {
	public:
		vec3() : vector<T,4>()
		{
		}

		vec3(T x, T y, T z) : xyz{ 0,x,y,z }
		{
		}

		vec3(const T _xyz[3])
		{
			for (int i = 1; i < 4; i++) {
				xyz[i] = _xyz[i];
			}
		}

		vec3(const T a)
		{
			for (int i = 0; i < fcount; i++) {
				xyz[i] = a;
			}
		}

		inline vector operator-(const vector& _rhs) const {
			vector r = *this;
			const vector rhs = _rhs;
			for (int i = 0; i < 4; i++) {
				r.xyz[i] -= rhs.xyz[i];
			}
			return r;
		}

		inline T x() const { return vector<T, 4>::xyz[1]; }
		inline void x(T a) { vector<T, 4>::xyz[1] = a; }

		inline T y() const { return vector<T, 4>::xyz[2]; }
		inline void y(T a) { vector<T, 4>::xyz[2] = a; }

		inline T z() const { return vector<T, 4>::xyz[3]; }
		inline void z(T a) { vector<T, 4>::xyz[3] = a; }
	};*/
	
	typedef vector<double, 4> vec4d;
	typedef vector<float, 4> vec4f;

	//typedef vector<double,3> vec3d;
	//typedef vector<float,3> vec3f;
}