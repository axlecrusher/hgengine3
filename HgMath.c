#include <stdint.h>
#include <HgMath.h>
#include <stdio.h>

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