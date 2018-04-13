#include <Gravity.h>
#include <stdlib.h>
#include <HgMath.h>
#include <stdint.h>

#define G_ACCEL 9.807f

void allocate_space(GravityField* gf, uint16_t count) {
	gf->indices = (uint16_t*)calloc(count, sizeof(*gf->indices));
	gf->velocity = (vector3*)calloc(count, sizeof(*gf->velocity));
	gf->count = count;
}

void gravity_update(GravityField* gf, uint32_t dTime) {
	uint32_t i;
	float x = (dTime / 1000.0f)*G_ACCEL;
	vector3 v1 = gf->vector.scale(x);

	for (i = 0; i < gf->count; ++i) {
		gf->velocity[i] = gf->velocity[i] + v1;
//		gf->velocity[i] += x;  // velocity per second
		vector3 d = gf->velocity[i].scale(dTime / 1000.0f);
//		float d = (dTime / 1000.0) * gf->velocity[i];
//		printf("gravity update %f %f\n", gf->velocity[i],d);

		HgElement* e = gf->scene->get_element(gf->indices[i]);
		e->position = e->position + d;


//		gf->scene->elements[gf->indices[i]].position.components.y -= d;
//		if (gf->scene->elements[gf->indices[i]].position.components.y < 0)
//			gf->scene->elements[gf->indices[i]].position.components.y = 0;
	}
}