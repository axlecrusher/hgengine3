#pragma once

#include <HgTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HgCamera {
	point position;
	quaternion rotation;

	vector3 direction;
	float speedMsec;
} HgCamera;

vector3 ray_from_camera(HgCamera* c);

#ifdef __cplusplus
}
#endif