#pragma once

#include <HgTypes.h>
#include <InstancedCollection.h>

class Projectile : public IUpdatableInstance<Instancing::GPUTransformationMatrix> {
public:
	using InstanceCollection = InstancedCollection<Projectile, Instancing::GPUTransformationMatrix, 1>;
	using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

	Projectile();
	Projectile(Projectile&&) = default;

	virtual void update(HgTime tdelta);
	virtual void getInstanceData(Instancing::GPUTransformationMatrix* instanceData);

	vector3 direction;
private:
	HgTime total_time;
//	float projectileMsecSpeed;
};

/*
void projectile_create(HgEntity* element);
void change_to_projectile(HgEntity* element);
*/