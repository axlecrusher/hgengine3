#pragma once

#define RADIANS 0.01745329251994329576923690768488612713442871888542
#define RAD_360 6.28318530717958647692528676655900576839433879875021

#include <HgTypes.h>

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

vector3 vector3_scale(const vector3* v, float scale);
vector3 vector3_add(const vector3* v, const vector3* v2);
float vector3_length(const vector3* v);
vector3 vector3_normalize(const vector3* v);
inline void vector3_zero(vector3* v) { v->array[0] = v->array[1] = v->array[2] = 0.0f; }

