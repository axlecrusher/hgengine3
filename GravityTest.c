#include <Gravity.h>
#include <stdlib.h>

#include <HgMath.h>
#define G_ACCEL 9.807

void allocate_space(GravityField* gf, uint16_t count) {
	gf->indices = calloc(count, sizeof(*gf->indices));
	gf->velocity = calloc(count, sizeof(*gf->velocity));
	gf->count = count;
}

void gravity_update(GravityField* gf, uint32_t dTime) {
	uint32_t i;
	float x = (dTime / 1000.0)*G_ACCEL;
	vector3 v1 = vector3_scale(&gf->vector, x);

	for (i = 0; i < gf->count; ++i) {
		gf->velocity[i] = vector3_add(gf->velocity+i, &v1);
//		gf->velocity[i] += x;  // velocity per second
		vector3 d = vector3_scale(gf->velocity+i, (dTime / 1000.0));
//		float d = (dTime / 1000.0) * gf->velocity[i];
//		printf("gravity update %f %f\n", gf->velocity[i],d);

		gf->scene->elements[gf->indices[i]].position = vector3_add((vector3*)&gf->scene->elements[gf->indices[i]].position, &d);


//		gf->scene->elements[gf->indices[i]].position.components.y -= d;
//		if (gf->scene->elements[gf->indices[i]].position.components.y < 0)
//			gf->scene->elements[gf->indices[i]].position.components.y = 0;
	}
}