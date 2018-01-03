#include <aabb.h>
#include <HgMath.h>
#include <float.h>
#include <stdlib.h>

#include "str_utils.h"

//#define max(a,b) (a > b ? a : b)
//#define min(a,b) (a < b ? a : b)

inline float max(float a, float b) { return (a > b) ? a : b; }
inline float min(float a, float b) { return (a < b) ? a : b; }

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

static bool cast_ray(const vector3* dirfrac, const vector3* pos, AABB bb) {
	vector3 a = vector3_sub(&bb.lb, pos);
	a = vector3_mul(&a, dirfrac);

	vector3 b = vector3_sub(&bb.rt, pos);
	b = vector3_mul(&b, dirfrac);

	float dmin = max(max(min(a.components.x, b.components.x), min(a.components.y, b.components.y)), min(a.components.z, b.components.z));
	float dmax = min(min(max(a.components.x, b.components.x), max(a.components.y, b.components.y)), max(a.components.z, b.components.z));

	//intersection, but behind ray position
	if (dmax < 0) return false;

	//ray doesn't intersect AABB
	if (dmin > dmax) return false;
}

void BoundingBoxes::cast_ray(const vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) const {
	//see https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms

	vector3 one = { 1.0,1.0,1.0 };
	vector3 dirfrac = vector3_div(&one, ray);
	aabb_result r;
	AABB* cubes = bounding_boxes;

	vector3 a, b;
	float dmin, dmax;

//	if (!::cast_ray(&dirfrac, pos, boundingVolume)) return; //broken.....

	for (uint32_t i = 0; i < cube_count; i++) {
		a = vector3_sub(&cubes[i].lb, pos);
		a = vector3_mul(&a, &dirfrac);

		b = vector3_sub(&cubes[i].rt, pos);
		b = vector3_mul(&b, &dirfrac);

		dmin = max(max(min(a.components.x, b.components.x), min(a.components.y, b.components.y)), min(a.components.z, b.components.z));
		dmax = min(min(max(a.components.x, b.components.x), max(a.components.y, b.components.y)), max(a.components.z, b.components.z));

		//intersection, but behind ray position
		if (dmax < 0) continue;

		//ray doesn't intersect AABB
		if (dmin > dmax) continue;

		r.dist = dmin;
		r.index = i;

		intersectClbk(&r, userData);
	}
}