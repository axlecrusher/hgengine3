#include <Projectile.h>

#include <triangle.h>
#include <oglDisplay.h>
#include <oglShaders.h>

#include <HgMath.h>

#include <stdlib.h>

static vtable_index VTABLE_INDEX;

static void updateClbk(struct HgElement* e, uint32_t tdelta) {

		ProjectileData* pd = e->extraData;
		pd->total_time += tdelta;

		if (pd->total_time >= 3000) {
//			printf("set destroy\n");
			SET_FLAG(e, HGE_DESTROY);
		}

		vector3 r = vector3_scale(&pd->direction, (tdelta / 50.0f));
		e->position = vector3_add(&e->position, &r);

//		printf("projectile %d\n", pd->total_time);

}

static void destroy(struct HgElement* e) {
	if (e->extraData) free(e->extraData);
	e->extraData = NULL;
	e->m_renderData = NULL;
//	HgElement_destroy(e);
	//	printf("destroy projectile\n");
	//	printf("cube\n");
}

void change_to_projectile(HgElement* element) {
	element->vptr_idx = VTABLE_INDEX;
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

	element->extraData = calloc(1,sizeof(ProjectileData));
	change_to_projectile(element);
}

static HgElement_vtable vtable = {
	.create = projectile_create,
	.destroy = destroy,
	.updateFunc = updateClbk
};

REGISTER_LINKTIME(basic_projectile);