#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

//FIXME: must automate this number generation
#define VTABLE_INDEX 3

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
		printf("projectile\n");
}

static void destroy(struct HgElement* e) {

	//	printf("cube\n");
}

static HgElement_vtable vtable = {
	.destroy = destroy,
	.updateFunc = updateClbk
};

void change_to_projectile(HgElement* element) {
	HGELEMT_VTABLES[VTABLE_INDEX] = vtable; //how to only do this once?

	//	element->vptr = &vtable;
	element->vptr_idx = 0;
	//create an instance of the render data for all triangles to share

	//create an instance of the render data for all triangles to share
	element->m_renderData = (RenderData*)triangle_init_render_data();
}

void projectile_create(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_projectile(element);
}
