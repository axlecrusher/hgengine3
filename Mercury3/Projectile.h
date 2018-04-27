#pragma once

//#include <HgElement.h>
#include <IUpdatable.h>
#include <HgTypes.h>

class Projectile : public IUpdatable {
public:
	Projectile();
	virtual void update(uint32_t tdelta);
	vector3 direction;
	uint32_t total_time;
//	float projectileMsecSpeed;
};

/*
void projectile_create(HgElement* element);
void change_to_projectile(HgElement* element);
*/