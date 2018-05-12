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
	vertex3f object;
	float raw[3];

	vertex() : object() {}
	vertex(float x, float y, float z) : object(x,y,z) {}
};

typedef struct vertices {
	union {
		vertex* v;
		float* array;
	} points;
	uint32_t size;
} vertices;

//typedef vertex* vertices;