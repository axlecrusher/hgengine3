#pragma once

#define RADIANS 0.01745329251994329576923690768488612713442871888542
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
}
