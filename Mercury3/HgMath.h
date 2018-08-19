#pragma once

#include <math.h>

#undef max
#undef min

#define DEG_RAD 0.01745329251994329576923690768488612713442871888542
#define RAD_DEG 57.29577951308232087679815481410517033240547246656432

#define RAD_360 6.28318530717958647692528676655900576839433879875021
#define M_PI 3.14159265358979323846264338327950288419716939937511

void MatrixMultiply4f(const float* in1, const float* in2, float* outa);
void print_matrix(const float* m);

void Perspective(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M);

void Perspective2(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M);

namespace HgMath {
	template<typename T>
	inline T square(T x) { return x*x; }

	template<typename T>
	inline T max(T a, T b) { return (a > b) ? a : b; }

	template<typename T>
	inline T min(T a, T b) { return (a < b) ? a : b; }

	inline float sqrt(float x) { return ::sqrtf(x); }
	inline double sqrt(double x) { return ::sqrt(x); }

	template<typename T>
	class angle_template {
	public:
		angle_template<T>() : m_radians(0) {}
		static const angle_template<T> ZERO;

		static inline angle_template<T> deg(T x) { angle_template<T> tmp; tmp.m_radians = (T)x*DEG_RAD; return tmp; }
		//inline angle_template<T>& deg(T x) { m_radians = x*DEG_RAD; return *this; }
		inline T deg() const { return (T)m_radians*RAD_DEG; }

		inline T rad() const { return m_radians; }
		static inline angle_template<T> rad(T x) { angle_template<T> tmp; tmp.m_radians = x; return tmp; }
	private:
		T m_radians;
	};

	typedef angle_template<double> angle;
}
