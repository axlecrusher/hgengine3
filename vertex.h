#pragma once

#include <stdint.h>

typedef struct _vertex_internal {
	float x, y, z;
} _vertex_internal; //pack tight

typedef struct vertex {
	union {
		_vertex_internal components;
		float array[3];
	};
} vertex;

typedef struct vertices {
	union {
		vertex* v;
		float* array;
	} points;
	uint32_t f_size;
} vertices;

//typedef vertex* vertices;