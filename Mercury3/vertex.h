#pragma once

#include <stdint.h>
#include <vertex3d.h>

//typedef struct _vertex_internal {
//	float x, y, z;
//} _vertex_internal; //pack tight
//
//typedef struct vertex {
//	union {
//		_vertex_internal components;
//		float array[3];
//	};
//} vertex;

//typedef vertex3f vertex;

union vertex {
	float raw[3]; //first so list initilization works
	vertex3f object;
};

typedef struct vertices {
	union {
		vertex* v;
		float* array;
	} points;
	uint32_t size;
} vertices;

//typedef vertex* vertices;