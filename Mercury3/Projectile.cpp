#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

#include <HgMath.h>
#include <HgElement.h>
#include <stdlib.h>

#include <UpdatableCollection.h>

float projectileMsecSpeed = 1.0f / 50.0f;

static UpdatableCollection<Projectile>& ProjectileCollection() {
	static UpdatableCollection<Projectile> collection;
	return collection;
}

static Projectile& CreateProjectile() {
	static bool init = false;
	if (init == false) {
		Engine::collections().push_back(&ProjectileCollection());
		init = true;
	}
	return ProjectileCollection().newItem();
}

Projectile::Projectile()
{
	element.init();
}

void Projectile::update(HgTime tdelta) {
		total_time += tdelta;

		if (total_time >= HgTime::msec(3000)) {
//			printf("set destroy\n");
			element.setFlag(HGE_DESTROY);
			ProjectileCollection().remove(*this);
			return;
		}

		float tmp = tdelta.msec() * projectileMsecSpeed;
		vector3 r = direction.scale(tmp);
		element.position = element.position + r;
}

static void* generate_projectile(HgElement* element) {
	//element->setLogic(std::make_unique<Projectile>());
	Projectile& p = CreateProjectile();
	//p.setElement(element);

	//p.getElement().init();

	change_to_triangle(&p.getElement());
	return &p;
}

REGISTER_LINKTIME(basic_projectile, generate_projectile);