#include <aabb.h>
#include <HgMath.h>
#include <float.h>
#include <stdlib.h>

#include "str_utils.h"

template<typename T>
inline T max(T a, T b) { return (a > b) ? a : b; }

template<typename T>
inline T min(T a, T b) { return (a < b) ? a : b; }

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

static bool cast_ray(const vector3& dirfrac, const vector3& pos, AABB bb) {
	vector3 a = (bb.lb - pos) * dirfrac;
	vector3 b = (bb.rt - pos) * dirfrac;

	float dmin = max(max(min(a.x(), b.x()), min(a.y(), b.y())), min(a.z(), b.z()));
	float dmax = min(min(max(a.x(), b.x()), max(a.y(), b.y())), max(a.z(), b.z()));

	if ((dmax < 0) ||	//intersection, but behind ray position
		(dmin > dmax))	//ray doesn't intersect AABB
	{
		return false;
	}

	return true;
}

void BoundingBoxes::cast_ray(const vector3& ray, const vector3& pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) const {
	//see https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms

	static const vector3 one = { 1.0,1.0,1.0 };
	vector3 dirfrac = one / ray;
	AABB* cubes = bounding_boxes.get();

	vector3 a, b;
	float dmin, dmax;
	aabb_result r;

	if (!::cast_ray(dirfrac, pos, boundingVolume)) return;

	for (int32_t i = 0; i < cube_count; i++) {
		a = (cubes[i].lb - pos) * dirfrac;
		b = (cubes[i].rt - pos) * dirfrac;

		//super scalar?
		float min1 = min(a.x(), b.x());
		float min2 = min(a.y(), b.y());
		float min3 = min(a.z(), b.z());

		float max1 = max(a.x(), b.x());
		float max2 = max(a.y(), b.y());
		float max3 = max(a.z(), b.z());

		dmin = max(max(min1, min2), min3);
		dmax = min(min(max1, max2), max3);

		if ((dmax < 0) ||	//intersection, but behind ray position
			(dmin > dmax))	//ray doesn't intersect AABB
		{
			continue;
		}

		r = { dmin, i };
		intersectClbk(&r, userData);
	}
}