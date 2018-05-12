#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <HgVbo.h>
#include <HgUtils.h>

static float vv[12] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f
};

static color colors[] = {
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255,
	255, 0, 0, 255
};

static vbo_layout_vc raw_data[] = {
	{{-0.5f, -0.5f, 0.0f}, 255, 0, 0, 255},
	{{0.5f, -0.5f, 0.0f}, 0, 255, 0, 255},
	{{0.5f, 0.5f, 0.0f}, 0, 0, 255, 255},
	{{-0.5f, 0.5f, 0.0f}, 255, 0, 0, 255}
};

static uint8_t indices[] = {
	0,1,2,2,3,0
};
/*
static void updateClbk(struct HgElement* e, uint32_t tdelta) {
}
*/
static void destroy(HgElement* e) {
	e->setRenderData(NULL,false);
}

//instanced render data
static OGLRenderData* trd = NULL;


static void SetupRenderData() {
	trd = OGLRenderData::Create();
//	trd->indices.data = indices;

	//vertices points;
	//points.points.array = vv;
	//points.size = 4;

	trd->hgVbo(staticVbo);
	trd->vertex_count = NUM_ARRAY_ELEMENTS(raw_data);
	trd->index_count = NUM_ARRAY_ELEMENTS(indices);
	trd->vbo_offset = staticVbo->add_data(raw_data, trd->vertex_count);

	trd->index_offset = staticIndice8->add_data(indices, trd->index_count);
	trd->indexVbo(staticIndice8);
}

static OGLRenderData* init_render_data() {
	if (trd == NULL) SetupRenderData();
	return trd;
}

static void* change_to_square(HgElement* element) {
	//create an instance of the render data for all triangles to share
	element->setRenderData( init_render_data(), false );
	return nullptr;
}
/*
void shape_create_square(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_square(element);
}
*/

REGISTER_LINKTIME(square,change_to_square)