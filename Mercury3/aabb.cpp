#include <aabb.h>
#include <HgMath.h>
#include <float.h>
#include <stdlib.h>

#include "str_utils.h"

//#define max(a,b) (a > b ? a : b)
//#define min(a,b) (a < b ? a : b)

template<typename T>
inline T max(T a, T b) { return (a > b) ? a : b; }

template<typename T>
inline T min(T a, T b) { return (a < b) ? a : b; }

AABB* BoundingBoxes::allocate(uint32_t count) {
	AABB* r = (AABB*)malloc(sizeof(*r) * count);
	return r;
}

BoundingBoxes::BoundingBoxes() : bounding_boxes(nullptr)
{
	cube_count = 0;
}

void BoundingBoxes::setBoxes(const AABB* bc, uint32_t count) {
	if (bounding_boxes) SAFE_FREE(bounding_boxes);
	bounding_boxes = (AABB*)malloc(sizeof(*bounding_boxes) * count);
	cube_count = count;

	boundingVolume = bc[0];

	for (uint32_t i = 0; i < count; i++) {
		bounding_boxes[i] = bc[i];

		if (bounding_boxes[i].lb < boundingVolume.lb) boundingVolume.lb = bounding_boxes[i].lb;
		if (!(boundingVolume.rt < bounding_boxes[i].rt)) boundingVolume.rt = bounding_boxes[i].rt;
	}
}

static bool cast_ray(const vector3& dirfrac, const vector3& pos, AABB bb) {
	vector3 a = (bb.lb - pos) * dirfrac;
	vector3 b = (bb.rt - pos) * dirfrac;

	float dmin = max(max(min(a.x(), b.x()), min(a.y(), b.y())), min(a.z(), b.z()));
	float dmax = min(min(max(a.x(), b.x()), max(a.y(), b.y())), max(a.z(), b.z()));

	//intersection, but behind ray position
	if (dmax < 0) return false;

	//ray doesn't intersect AABB
	if (dmin > dmax) return false;
}

void BoundingBoxes::cast_ray(const vector3& ray, const vector3& pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) const {
	//see https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms

	static const vector3 one = { 1.0,1.0,1.0 };
	vector3 dirfrac = one / ray;
	AABB* cubes = bounding_boxes;

	vector3 a, b;
	float dmin, dmax;
	aabb_result r;

//	if (!::cast_ray(&dirfrac, pos, boundingVolume)) return; //broken.....

	for (int32_t i = 0; i < cube_count; i++) {
		a = (cubes[i].lb - pos) * dirfrac;
		b = (cubes[i].rt - pos) * dirfrac;

		dmin = max(max(min(a.x(), b.x()), min(a.y(), b.y())), min(a.z(), b.z()));
		dmax = min(min(max(a.x(), b.x()), max(a.y(), b.y())), max(a.z(), b.z()));

		//intersection, but behind ray position
		if (dmax < 0) continue;

		//ray doesn't intersect AABB
		if (dmin > dmax) continue;

		r = { dmin, i };
		intersectClbk(&r, userData);
	}
}