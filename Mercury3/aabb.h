#pragma once

#include <HgTypes.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct AABB {
	//left bottom
	vector3 lb;

	//right top
	vector3 rt;
} AABB;

typedef struct aabb_result {
	float dist;
	uint32_t index;
} aabb_result;

typedef struct BoundingCubes {
	AABB* bounding_boxes;
	uint32_t cube_count;
} BoundingCubes;

void aabb_allocate_cubes(BoundingCubes* bc, uint32_t count);
void cast_ray_to_aabb(BoundingCubes* aabb, vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData);

#ifdef __cplusplus
}
#endif