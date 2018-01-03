
#include <shapes.h>
#include <stdlib.h>

void gen_triangle(vertices* v) {
	static float vv[9] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		0.0f, 0.5f, 0.0f
	};

	v->points.array = (float*)malloc(9 * sizeof(float));
	memcpy(v->points.array, vv, 9 * sizeof(float));
	v->f_size = 9;
}