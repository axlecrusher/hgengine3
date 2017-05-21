#pragma once

#include <vertex.h>
#include <quaternion.h>

typedef vertex point;

typedef struct HgCamera {
	point position;
	quaternion rotation;
} HgCamera;