#pragma once

#include <HgScene2.h>
#include <HgTypes.h>
#include <InstancedCollection.h>

#include <RefCounterTable.h>

namespace Projectile
{

	//class Projectile : public IUpdatableInstance<Instancing::GPUTransformationMatrix> {
	class Projectile
	{
	public:
		//using InstanceCollection = InstancedCollection<Projectile, Instancing::GPUTransformationMatrix, 1>;
		//using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		Projectile() = default;
		//Projectile(Projectile&&) = default;

		//virtual void init();

		void update(HgTime tdelta);
		//virtual void getInstanceData(Instancing::GPUTransformationMatrix* instanceData);

		vector3 direction;
		EntityIdType entityId;

	private:
		HgTime total_time;
		//	float projectileMsecSpeed;
	};

	//using ProjectileTableType = RefCountedTable<Projectile>;
	using ProjectileTableType = std::vector<Projectile>;

	Projectile* CreateProjectileTriangle(Engine::HgScene* scene);

	void updateProjectiles(HgTime dt);

	extern ProjectileTableType ProjectileTable;

}
/*
void projectile_create(HgEntity* element);
void change_to_projectile(HgEntity* element);
*/