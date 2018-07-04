#pragma once

#include <memory>
#include <HgTypes.h>

class aabb_result {
public:
	aabb_result() : dist(FLT_MAX), index(-1)
	{}

	aabb_result(float d, int32_t i) : dist(d), index(i)
	{}

	inline bool operator<(const aabb_result& rhs) const { return dist < rhs.dist; }
	inline bool hit() const { return dist != FLT_MAX; }

	float dist;
	int32_t index;
};

typedef struct AABB {
	//left bottom
	vertex3f lb;

	//right top
	vertex3f rt;
} AABB;

class BoundingBoxes {
public:
	BoundingBoxes();

	void setBoxes(const AABB* bc, uint32_t count);
	void setBoxes(std::unique_ptr<AABB[]>& bc, uint32_t count);
	void cast_ray(const vector3& ray, const vector3& pos, void(*intersectClbk)(const aabb_result& result, void* userData), void* userData) const;

private:
	aabb_result cast_ray(const vector3f& dirfrac, const vector3f& pos, const AABB& bb) const ;

	AABB boundingVolume;
	std::unique_ptr<AABB[]> bounding_boxes;
	uint32_t cube_count;
};
