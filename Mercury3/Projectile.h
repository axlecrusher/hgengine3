#pragma once

#include <HgElement.h>

extern float projectileMsecSpeed;

typedef struct ProjectileData {
	vector3 direction;
	uint32_t total_time;
} ProjectileData;

/*
void projectile_create(HgElement* element);
void change_to_projectile(HgElement* element);
*/