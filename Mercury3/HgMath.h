#pragma once

#define RADIANS 0.01745329251994329576923690768488612713442871888542
#define RAD_360 6.28318530717958647692528676655900576839433879875021

#include <HgTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

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

vector3 vector3_scale(const vector3* lhs, float scale);
vector3 vector3_add(const vector3* lhs, const vector3* rhs);
vector3 vector3_sub(const vector3* lhs, const vector3* rhs);
vector3 vector3_mul(const vector3* lhs, const vector3* rhs);
vector3 vector3_div(const vector3* lhs, const vector3* rhs);
float vector3_dot(const vector3* lhs, const vector3* rhs);

quaternion vector3_to_quat(const vector3* a);

float vector3_length(const vector3* v);
vector3 vector3_normalize(const vector3* v);

extern vector3 vector3_zero;
//inline vector3 vector3_zero() { vector3 a = { 0,0,0 }; return a; }

vector3 vector3_cross(const vector3* v1, const vector3* v3);
vector3 vector3_quat_rotate(const vector3* v, const quaternion* q);

#ifdef __cplusplus
}
#endif