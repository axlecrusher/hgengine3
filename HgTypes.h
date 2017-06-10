#pragma once

#include <vertex.h>
#include <quaternion.h>

typedef vertex point;
typedef vertex vector3;

typedef struct HgCamera {
	point position;
	quaternion rotation;
} HgCamera;

typedef struct viewport {
	uint16_t x, y, width, height;
} viewport;

typedef struct color {
	uint8_t r, g, b, a;
} color;

typedef struct uv_coord {
	uint16_t u,v;
} uv_coord;

typedef struct normal {
	float x, y, z;
} normal;