#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

#include <HgMath.h>
#include <HgElement.h>
#include <stdlib.h>

float projectileMsecSpeed = 1.0f / 50.0f;

Projectile::Projectile()
	:total_time(0)
{
	direction = vector3_zero;
}

void Projectile::update(uint32_t tdelta) {
		total_time += tdelta;

		if (total_time >= 3000) {
			printf("set destroy\n");
			SET_FLAG(element, HGE_DESTROY);
		}

		float tmp = tdelta * projectileMsecSpeed;
		vector3 r = vector3_scale(direction, tmp);
		element->position = vector3_add(&element->position, &r);
}

static void change_to_projectile(HgElement* element) {
	element->setLogic(std::make_unique<Projectile>());
	change_to_triangle(element);
}
/*
void projectile_create(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_projectile(element);
}
*/

REGISTER_LINKTIME(basic_projectile, change_to_projectile);