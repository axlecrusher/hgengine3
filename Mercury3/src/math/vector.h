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

			//loop vectorized
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] *= n;
			}
			return r;
		}

		inline vector operator+(vector rhs) const {
			vector r = *this;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] += rhs.xyz[i];
			}
			return r;
		}

		inline vector operator+=(vector rhs) {
			for (int i = 0; i < fcount; i++) {
				xyz[i] += rhs.xyz[i];
			}
			return *this;
		}

		inline vector operator-(vector rhs) const {
			vector r = *this;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] -= rhs.xyz[i];
			}
			return r;
		}

		inline vector operator*(vector rhs) const {
			vector r = *this;
			for (int i = 0; i < fcount; i++) {
				r.xyz[i] *= rhs.xyz[i];
			}
			return r;
		}

		inline vector operator/(vector rhs) const {
			vector r = *this;
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

		inline T dot(vector rhs) const {
			T components[fcount];

			const vector a = *this;

			//the loop is auto vectorized
			for (int i = 0; i < fcount; i++)
			{
				components[i] = a[i] * rhs[i];
			}

			const auto sum1 = components[0] + components[1];
			const auto sum2 = components[2] + components[3];

			return sum1 + sum2;
		}

		inline T squaredMagnitude() const {
			return this->dot(*this);
		}

		inline T magnitude() const {
			return HgMath::sqrt(squaredMagnitude());
		}

		inline vector normal() const {
			vector r;
			double sqmag = squaredMagnitude();
			//https://stackoverflow.com/questions/11667783/quaternion-and-normalization/12934750#12934750
			if (std::abs(1.0 - sqmag) < 2.107342e-08)
			{
				r = this->scale(T(2.0 / (1.0 + sqmag)));
			}
			else
			{
				r = *this / T(::sqrt(sqmag));
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
			return (squaredMagnitude() < tolerance<T>());
		}

		inline const T* raw() const { return xyz; }
		inline T* raw() { return xyz; }

		inline T w() const { return xyz[0]; }
		inline T x() const { return xyz[1]; }
		inline T y() const { return xyz[2]; }
		inline T z() const { return xyz[3]; }

	protected:

		T xyz[fcount];
	private:
		template<typename X>
		X tolerance() const { static_assert(false, "Not implemented"); }

		template<>
		double tolerance<double>() const { return 1e-15; }

		template<>
		float tolerance<float>() const { return 1e-06f; }

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