#pragma once

#include <HgTypes.h>

class vertex3D : public vertex {
public:
	vertex3D() {}
	vertex3D(vertex v) { components = v.components; }
	bool operator<(const vertex& rhs) const {
		return ((components.x < rhs.components.x)
			&& (components.y < rhs.components.y)
			&& (components.z < rhs.components.z));
	}
};

typedef struct AABB {
	//left bottom
	vertex3D lb;

	//right top
	vertex3D rt;
} AABB;

typedef struct aabb_result {
	float dist;
	uint32_t index;
} aabb_result;

class BoundingBoxes {
public:
	BoundingBoxes();

	void setBoxes(const AABB* bc, uint32_t count);
	void cast_ray(const vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) const;

	static AABB* allocate(uint32_t count);

private:
	AABB boundingVolume;
	AABB* bounding_boxes;
	uint32_t cube_count;
};

//void aabb_allocate_cubes(BoundingBoxes* bc, uint32_t count);
//void cast_ray_to_aabb(const BoundingBoxes* aabb, const vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData);
