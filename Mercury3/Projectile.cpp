#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

#include <HgMath.h>
#include <HgEntity.h>
#include <stdlib.h>
#include <HgEngine.h>

#include <UpdatableCollection.h>

namespace Projectile
{

	using namespace Projectile;
	float projectileMsecSpeed = 1.0f / 50.0f;

	ProjectileTableType ProjectileTable;

	//static auto ProjectileCollection()
	//{
	//	return Engine::getCollectionOf<UpdatableCollection<Projectile>>();
	//}
	//
	//static Projectile* CreateProjectile() {
	//	return ProjectileCollection()->newItem();
	//}

	//Projectile::Projectile()
	//{
	//	//m_entity.init();
	//}

	//void Projectile::init()
	//{
	//	//m_entity.init();
	//	//change_to_triangle(&getEntity());
	//}

	void Projectile::update(HgTime tdelta) {
		total_time += tdelta;

		if (total_time >= HgTime::msec(3000)) {
			EntityIdTable::Singleton().destroy(entityId);
			//EntityTable::Singleton().destroy(entityId);
			return;
		}

		float distance = tdelta.seconds() * 20.0;
		vector3 r = direction.normal().scale(distance);

		HgEntity* entity = EntityTable::Singleton().getPtr(entityId);
		entity->position(entity->position() + r);
	}

	//void Projectile::getInstanceData(Instancing::GPUTransformationMatrix* instanceData)
	//{
	//	HgEntity* entity = EntityTable::Singleton().getPtr(entityId);
	//	const auto mat = entity.computeWorldSpaceMatrix();
	//	mat.store(instanceData->matrix);
	//}


	Projectile* CreateProjectileTriangle(Engine::HgScene* scene)
	{
		auto id = EntityHelpers::createSingle();
		change_to_triangle(id);
		scene->addEntityID(id);

		Projectile p;
		p.entityId = id;

		ProjectileTable.push_back(p);

		return &ProjectileTable.back();
	}

	void updateProjectiles(HgTime dt)
	{
		for (auto& p : ProjectileTable)
		{
			p.update(dt);
		}
	}


} //namespace Projectile

//static void* generate_projectile(HgEntity* entity) {
//	//element->setLogic(std::make_unique<Projectile>());
//	Projectile* p = CreateProjectile();
//	//p.setElement(element);
//
//	//p.getElement().init();
//
//	change_to_triangle(&p->getEntity());
//	return p;
//}
//
//REGISTER_LINKTIME(basic_projectile, generate_projectile);