#include <aabb.h>
#include <HgMath.h>
#include <float.h>
#include <stdlib.h>

#include "str_utils.h"

using namespace HgMath;

BoundingBoxes::BoundingBoxes() : bounding_boxes(nullptr), cube_count(0)
{
}

void BoundingBoxes::setBoxes(const AABB* bc, uint32_t count) {
	bounding_boxes = std::unique_ptr<AABB[]>(new AABB[sizeof(AABB) * count]);
	cube_count = count;

	boundingVolume = bc[0];

	for (uint32_t i = 0; i < count; i++) {
		bounding_boxes[i] = bc[i];

		boundingVolume.lb.x(min(boundingVolume.lb.x(), bc[i].lb.x()));
		boundingVolume.lb.y(min(boundingVolume.lb.y(), bc[i].lb.y()));
		boundingVolume.lb.z(min(boundingVolume.lb.z(), bc[i].lb.z()));

		boundingVolume.rt.x(max(boundingVolume.rt.x(), bc[i].rt.x()));
		boundingVolume.rt.y(max(boundingVolume.rt.y(), bc[i].rt.y()));
		boundingVolume.rt.z(max(boundingVolume.rt.z(), bc[i].rt.z()));
	}
}

void BoundingBoxes::setBoxes(std::unique_ptr<AABB[]>& bc, uint32_t count) {
	for (uint32_t i = 0; i < count; i++) {
		boundingVolume.lb.x(min(boundingVolume.lb.x(), bc[i].lb.x()));
		boundingVolume.lb.y(min(boundingVolume.lb.y(), bc[i].lb.y()));
		boundingVolume.lb.z(min(boundingVolume.lb.z(), bc[i].lb.z()));

		boundingVolume.rt.x(max(boundingVolume.rt.x(), bc[i].rt.x()));
		boundingVolume.rt.y(max(boundingVolume.rt.y(), bc[i].rt.y()));
		boundingVolume.rt.z(max(boundingVolume.rt.z(), bc[i].rt.z()));
	}

	bounding_boxes = std::move(bc);
	cube_count = count;
}

aabb_result BoundingBoxes::cast_ray(const vector3f& dirfrac, const vector3f& pos, const AABB& bb) const {
	aabb_result result;

	const vector3f a = (bb.lb - pos) * dirfrac;
	const vector3f b = (bb.rt - pos) * dirfrac;

	const float dmin = max(max(min(a.x(), b.x()), min(a.y(), b.y())), min(a.z(), b.z()));
	const float dmax = min(min(max(a.x(), b.x()), max(a.y(), b.y())), max(a.z(), b.z()));

	if (!((dmax < 0) ||	//intersection, but behind ray position
		(dmin > dmax)))	//ray doesn't intersect AABB
	{
		result.dist = dmin;
	}

	return result;
}

void BoundingBoxes::cast_ray(const vector3& ray, const vector3& pos, void(*intersectClbk)(const aabb_result& result, void* userData), void* userData) const {
	//see https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms

	static const vector3 one = { 1.0,1.0,1.0 };
	const vector3 dirfrac( one / ray );
	const AABB* cubes = bounding_boxes.get();
	const int32_t count = cube_count;

	if (!cast_ray(dirfrac, pos, boundingVolume).hit()) return;

	for (int i = 0; i < count; i++) {
		aabb_result r = cast_ray(dirfrac, pos, cubes[i]); //could be done in parallel if needed
		if (r.hit()) {
			r.index = i;
			intersectClbk(r, userData);
		}
	}
}