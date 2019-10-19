#pragma once

//#include <HgEntity.h>
#include <IUpdatable.h>
#include <HgTypes.h>

class Projectile : public IUpdatable {
public:
	Projectile();
	Projectile(Projectile&&) = default;

	virtual void update(HgTime tdelta);
	vector3 direction;
private:
	HgTime total_time;
//	float projectileMsecSpeed;
};

/*
void projectile_create(HgEntity* element);
void change_to_projectile(HgEntity* element);
*/