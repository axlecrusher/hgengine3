#include <stdint.h>
#include <HgMath.h>
#include <stdio.h>

#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include <quaternion.h>

void MatrixMultiply4f(const float* in1, const float* in2, float* outa)
{
	float* r = outa;

	r[0] = in1[0] * in2[0] + in1[1] * in2[4] +
		in1[2] * in2[8] + in1[3] * in2[12];
	r[1] = in1[0] * in2[1] + in1[1] * in2[5] +
		in1[2] * in2[9] + in1[3] * in2[13];
	r[2] = in1[0] * in2[2] + in1[1] * in2[6] +
		in1[2] * in2[10] + in1[3] * in2[14];
	r[3] = in1[0] * in2[3] + in1[1] * in2[7] +
		in1[2] * in2[11] + in1[3] * in2[15];

	r[4] = in1[4] * in2[0] + in1[5] * in2[4] +
		in1[6] * in2[8] + in1[7] * in2[12];
	r[5] = in1[4] * in2[1] + in1[5] * in2[5] +
		in1[6] * in2[9] + in1[7] * in2[13];
	r[6] = in1[4] * in2[2] + in1[5] * in2[6] +
		in1[6] * in2[10] + in1[7] * in2[14];
	r[7] = in1[4] * in2[3] + in1[5] * in2[7] +
		in1[6] * in2[11] + in1[7] * in2[15];

	r[8] = in1[8] * in2[0] + in1[9] * in2[4] +
		in1[10] * in2[8] + in1[11] * in2[12];
	r[9] = in1[8] * in2[1] + in1[9] * in2[5] +
		in1[10] * in2[9] + in1[11] * in2[13];
	r[10] = in1[8] * in2[2] + in1[9] * in2[6] +
		in1[10] * in2[10] + in1[11] * in2[14];
	r[11] = in1[8] * in2[3] + in1[9] * in2[7] +
		in1[10] * in2[11] + in1[11] * in2[15];

	r[12] = in1[12] * in2[0] + in1[13] * in2[4] +
		in1[14] * in2[8] + in1[15] * in2[12];
	r[13] = in1[12] * in2[1] + in1[13] * in2[5] +
		in1[14] * in2[9] + in1[15] * in2[13];
	r[14] = in1[12] * in2[2] + in1[13] * in2[6] +
		in1[14] * in2[10] + in1[15] * in2[14];
	r[15] = in1[12] * in2[3] + in1[13] * in2[7] +
		in1[14] * in2[11] + in1[15] * in2[15];
}

void print_matrix(const float* m) {
	uint8_t i;
	for (i = 0; i < 4; ++i) {
		printf("%f %f %f %f\n", m[i*4], m[i*4 + 1], m[i*4 + 2], m[i*4 + 3]);
	}
}


void Perspective(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	double top = tan(fov*0.5 * RADIANS) * znear;
	double bottom = -top;
	double right = aspect*top;
	double left = -right;

	M[0] = (float)((2 * znear) / (right - left));
	M[2] = (float)((right + left) / (right - left));
	M[6] = (float)((top + bottom) / (top - bottom));
	M[5] = (float)((2 * znear) / (top - bottom));
	M[10] = (float)((zfar + znear) / (zfar - znear));
	M[11] = (float)((2 * zfar*znear) / (zfar - znear));
	M[14] = -1.0f;
}

void Perspective2(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	fov *= RADIANS;

	double f = 1.0 / tan(fov*0.5);
	memset(M, 0, 16 * sizeof* M);


	M[0] = (float)(f / aspect);
	M[5] = (float)f;
	M[10] = (float)((zfar + znear) / (znear - zfar));
	M[11] = (float)((2 * zfar*znear) / (znear - zfar));
	M[14] = -1.0f;
}

vector3 vector3_scale(const vector3* v, float scale) {
	vector3 r = *v;
	r.array[0] *= scale;
	r.array[1] *= scale;
	r.array[2] *= scale;
	return r;
}

vector3 vector3_mul(const vector3* v, const vector3* v2) {
	vector3 r = *v;
	r.array[0] *= v2->array[0];
	r.array[1] *= v2->array[1];
	r.array[2] *= v2->array[2];
	return r;
}

vector3 vector3_div(const vector3* lhs, const vector3* rhs) {
	vector3 r = *lhs;
	r.array[0] /= rhs->array[0];
	r.array[1] /= rhs->array[1];
	r.array[2] /= rhs->array[2];
	return r;
}

vector3 vector3_add(const vector3* lhs, const vector3* rhs) {
	vector3 r = *lhs;
	r.array[0] += rhs->array[0];
	r.array[1] += rhs->array[1];
	r.array[2] += rhs->array[2];
	return r;
}

vector3 vector3_sub(const vector3* lhs, const vector3* rhs) {
	/*
	float result[4];

	__m128 rx = _mm_loadu_ps(v->array);
	__m128 xx = _mm_loadu_ps(v2->array);
	rx = _mm_sub_ps(rx, xx);
	_mm_store_ps(result, rx);

	return *(vector3*)result;
	*/
	vector3 r = *lhs;
	r.array[0] -= rhs->array[0];
	r.array[1] -= rhs->array[1];
	r.array[2] -= rhs->array[2];
	return r;
}

#define SQUARE(x) (x*x)

float vector3_length(const vector3* v) {
	float x = SQUARE(v->array[0])
			+ SQUARE(v->array[1])
			+ SQUARE(v->array[2]);

	return (float)sqrt(x);
}

vector3 vector3_normalize(const vector3* v) {
	float length = vector3_length(v);
//	if (fabs(length) < 0.000000000001f) return *v;
	if (length == 0.0) return *v;

	vector3 r = *v;
	r.array[0] /= length;
	r.array[1] /= length;
	r.array[2] /= length;

	return r;
}

vector3 vector3_cross(const vector3* v1, const vector3* v2) {
	const float* x = v1->array;
	const float* y = v2->array;
	vector3 r;
	r.array[0] = (x[1] * y[2]) - (x[2] * y[1]);
	r.array[1] = (x[2] * y[0]) - (x[0] * y[2]);
	r.array[2] = (x[0] * y[1]) - (x[1] * y[0]);
	return r;
}

vector3 vector3_quat_rotate(const vector3* v, const quaternion* q) {
	vector3 r1 = vector3_scale(v, q->w);
	vector3 r2 = vector3_cross((vector3*)&q->x, v);
	r1 = vector3_add(&r1, &r2);
	r2 = vector3_cross((vector3*)&q->x, &r1);
	r1 = vector3_scale(&r2, 2.0);
	r2 = vector3_add(v, &r1);
	return r2;
}

/*
// set the OpenGL perspective projection matrix
void glFrustum(
const float &bottom, const float &top, const float &left, const float &right,
const float &zNear, const float &zFar,
float *M)
{
//Should go in projection matrix
float near2 = 2 * zNear;
float rml = right - left;
float tmb = top - bottom;
float fmn = zFar - zNear;

float A = (right + left) / rml;
float B = (top + bottom) / tmb;
float C = -(zFar + zNear) / fmn;
float D = -(near2*zFar*zNear) / fmn;

memset(M, 0, 16 * sizeof* M);

//row major
M[0] = near2 / rml;
M[2] = A;
M[5] = near2 / tmb;
M[6] = B;
M[10] = C;
M[11] = D;
M[14] = -1;
}
*/