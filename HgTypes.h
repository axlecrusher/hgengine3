#pragma once

#include <vertex.h>
#include <quaternion.h>

typedef vertex point;

typedef struct HgCamera {
	point position;
	quaternion rotation;
} HgCamera;

typedef struct viewport {
	uint16_t x, y, width, height;
} viewport;