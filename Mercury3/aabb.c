#include <aabb.h>
#include <HgMath.h>
#include <float.h>
#include <stdlib.h>

//#define max(a,b) (a > b ? a : b)
//#define min(a,b) (a < b ? a : b)

void aabb_allocate_cubes(BoundingCubes* bc, uint32_t count) {
	bc->bounding_boxes = malloc(sizeof(*bc->bounding_boxes) * count);
	bc->cube_count = count;
}

void cast_ray_to_aabb(BoundingCubes* aabb, vector3* ray, const vector3* pos, void(*intersectClbk)(aabb_result* result, void* userData), void* userData) {
	vector3 one = { 1.0,1.0,1.0 };
	vector3 dirfrac = vector3_div(&one, ray);
	aabb_result r;
	AABB* cubes = aabb->bounding_boxes;

	vector3 a, b;
	float dmin, dmax;

	for (uint32_t i = 0; i < aabb->cube_count; i++) {
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