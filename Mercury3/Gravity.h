#pragma once

#include <HgScene.h>

typedef struct GravityField {
	uint16_t* indices;
	vector3* velocity;
	HgScene* scene;
	uint16_t count;

	point vector;
} GravityField;

void allocate_space(GravityField* gf, uint16_t count);
void gravity_update(GravityField* gf, uint32_t dTime);
