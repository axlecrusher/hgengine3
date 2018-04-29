#pragma once

#include <HgTypes.h>


typedef struct AABB {
	//left bottom
	vertex3d lb;

	//right top
	vertex3d rt;
} AABB;

typedef struct aabb_result {
	float dist;
	int32_t index;
} aabb_result;

class BoundingBoxes {
public:
	BoundingBoxes();

	void setBoxes(const AABB* bc, uint32_t count);
	void cast_ray(const vector3& ray, const vector3& pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) const;

private:
	AABB boundingVolume;
	std::unique_ptr<AABB[]> bounding_boxes;
	uint32_t cube_count;
};

//void aabb_allocate_cubes(BoundingBoxes* bc, uint32_t count);
//void cast_ray_to_aabb(const BoundingBoxes* aabb, const vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData);
