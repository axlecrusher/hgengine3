#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

#include <HgMath.h>
#include <HgEntity.h>
#include <stdlib.h>
#include <HgEngine.h>

#include <UpdatableCollection.h>

float projectileMsecSpeed = 1.0f / 50.0f;

static auto ProjectileCollection()
{
	return Engine::getCollectionOf<UpdatableCollection<Projectile>>();
}

static Projectile& CreateProjectile() {
	return ProjectileCollection()->newItem();
}

Projectile::Projectile()
{
	m_entity.init();
}

void Projectile::update(HgTime tdelta) {
		total_time += tdelta;

		if (total_time >= HgTime::msec(3000)) {
//			printf("set destroy\n");
			m_entity.setDestroy(true);
			ProjectileCollection()->remove(*this);
			return;
		}

		float distance = tdelta.seconds() * 20.0;
		vector3 r = direction.normal().scale(distance);
		m_entity.position(m_entity.position() + r);
}

void Projectile::getInstanceData(Instancing::GPUTransformationMatrix* instanceData)
{
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

static void* generate_projectile(HgEntity* entity) {
	//element->setLogic(std::make_unique<Projectile>());
	Projectile& p = CreateProjectile();
	//p.setElement(element);

	//p.getElement().init();

	change_to_triangle(&p.getEntity());
	return &p;
}

REGISTER_LINKTIME(basic_projectile, generate_projectile);