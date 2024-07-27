#include <stdint.h>
#include <HgMath.h>
#include <stdio.h>

#include <math.h>
#include <memory.h>
#include <stdlib.h>

#include <vertex3d.h>
#include <math/MatrixMath.h>

//const vertex vertex_zero = { 0,0,0 };
//const vector3 vector3_zero = { 0,0,0 };
//

template<>
const HgMath::angle HgMath::angle::ZERO;

const vertex3f vertex3f::UNIT_X( 1.0,0,0 );
const vertex3f vertex3f::UNIT_Y( 0,1.0,0 );
const vertex3f vertex3f::UNIT_Z( 0,0,1.0 );

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

//right handed
void Perspective(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	double top = tan(fov*0.5 * DEG_RAD) * znear;
	double bottom = -top;
	double right = aspect*top;
	double left = -right;

	//column major
	M[0] = (float)((2 * znear) / (right - left));
	M[5] = (float)((2 * znear) / (top - bottom));
	M[8] = (float)((right + left) / (right - left));
	M[9] = (float)((top + bottom) / (top - bottom));
	M[10] = (float)((zfar + znear) / (zfar - znear));
	M[11] = -1.0f;
	M[14] = (float)((2 * zfar*znear) / (zfar - znear));
}

//right handed
void Perspective2(
	double fov,
	const double aspect,
	const double znear,
	const double zfar, float* M)
{
	fov *= DEG_RAD;

	const double f = 1.0 / tan(fov*0.5);
	memset(M, 0, 16 * sizeof* M);

	//column major
	M[0] = (float)(f / aspect);
	M[5] = (float)f;
	M[10] = (float)((zfar + znear) / (znear - zfar));
	M[11] = -1.0f;
	M[14] = (float)((2 * zfar*znear) / (znear - zfar));
}

/*
*	Similar to glFrustum, but with a few changes.
	Construct right handed projection matrix with infinate zFar
	and reversed depth. Reversing the depth gives better zbuffer bitdepth
	distribution. Clipping must be set from 0 to 1.
*/
void Projection_RH_InfZ_RevDepth(float fLeft, float fRight, float fTop, float fBottom, float zNear, float* M)
{
	float idx = 1.0f / (fRight - fLeft);
	float idy = 1.0f / (fBottom - fTop);
	//float idz = 1.0f / (zFar - zNear);
	float sx = fRight + fLeft;
	float sy = fBottom + fTop;

	memset(M, 0, 16 * sizeof * M);

	M[0] = 2 * idx;
	M[5] = 2 * idy;

	M[8] = sx * idx;
	M[9] = sy * idy;
	M[10] = 0;
	M[11] = -1.0f;
	M[14] = zNear;
}


/*
	Construct right handed projection matrix with infinate z-plane
	and reversed z depth
*/
void Projection_RH_InfZ_RevDepth(
	double fov_deg,
	const double aspect,
	const double znear,
	float* M)
{
	fov_deg *= DEG_RAD;

	const double f = 1.0 / tan(fov_deg * 0.5);
	memset(M, 0, 16 * sizeof * M);

	//column major
	M[0] = (float)(f / aspect);
	M[5] = (float)f;
	M[10] = 0.0f;
	M[11] = -1.0f;
	M[14] = (float)znear;
}

void Ortho(	double left, double right,
			double bottom, double top,
			double near, double far,
			float* M)
{
	memset(M, 0, 16 * sizeof* M);

	M[3] = -(right + left)/(right-left);
	M[7] = -(top+bottom)/(top-bottom);
	M[11] = -(far+near)/(far-near);


	//column major
	M[0] = 2.0 / (right - left);
	M[5] = 2.0 / (top - bottom);
	M[10] = -2.0 / (far - near);
	M[15] = 1.0;
}