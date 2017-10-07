#include <stdint.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <assert.h>

#include <memory.h>

#include <HgVbo.h>
#include <HgElement.h>

//TestRegistration("test");

//static vtable_index VTABLE_INDEX;

//dump from a hgmdl model, VNU format.
vbo_layout_vnu raw_cube_data[] = {
	0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 32767, 32767,
	-0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 32767, 32767,	//left bottom 1
	0.5, -0.5, -0.5, 0.0, -1.0, 0.0, 32767, 32767,
	-0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 32767, 32767,
	0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 32767, 32767,		//top right 4
	0.5, 0.5, -0.5, 0.0, 1.0, 0.0, 32767, 32767,
	0.5, 0.5, -0.5, 1.0, -0.0, 0.0, 32767, 32767,
	0.5, -0.5, 0.5, 1.0, -0.0, 0.0, 32767, 32767,
	0.5, -0.5, -0.5, 1.0, -0.0, 0.0, 32767, 32767,
	0.5, 0.5, 0.5, 0.0, -0.0, 1.0, 32767, 32767,
	-0.5, -0.5, 0.5, 0.0, -0.0, 1.0, 32767, 32767,
	0.5, -0.5, 0.5, 0.0, -0.0, 1.0, 32767, 32767,
	-0.5, -0.5, 0.5, -1.0, -0.0, -0.0, 32767, 32767,
	-0.5, 0.5, -0.5, -1.0, -0.0, -0.0, 32767, 32767,
	-0.5, -0.5, -0.5, -1.0, -0.0, -0.0, 32767, 32767,
	0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 32767, 32767,
	-0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 32767, 32767,
	0.5, 0.5, -0.5, 0.0, 0.0, -1.0, 32767, 32767,
	-0.5, -0.5, 0.5, 0.0, -1.0, 0.0, 32767, 32767,
	-0.5, 0.5, 0.5, 0.0, 1.0, 0.0, 32767, 32767,
	0.5, 0.5, 0.5, 1.0, -0.0, 0.0, 32767, 32767,
	-0.5, 0.5, 0.5, 0.0, -0.0, 1.0, 32767, 32767,
	-0.5, 0.5, 0.5, -1.0, -0.0, -0.0, 32767, 32767,
	-0.5, -0.5, -0.5, 0.0, 0.0, -1.0, 32767, 32767
};

uint8_t cube_indices[] = {
	0, 1, 2,		3, 4, 5,
	0, 18, 1,		3, 19, 4,

	6, 7, 8,		9, 10, 11,	
	6, 20, 7,		9, 21, 10,

	12, 13, 14,		15, 16, 17,	
	12, 22, 13,		15, 23, 16
};

//instanced render data
static OGLRenderData *crd = NULL;
/*
static void updateClbk(struct HgElement* e, uint32_t tdelta) {
//	printf("cube\n");
}
*/
static void destroy(HgElement* e) {
//	if (e->m_renderData && e->m_renderData->destroy) e->m_renderData->destroy(e->m_renderData);
//	free(e->m_renderData);
	e->m_renderData = NULL;
}

//Draw vertices directly. We aren't using indices here,
static void cube_render(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;

	setBlendMode((BlendMode)rd->blendMode);
	d->hgVbo->use();

	glDrawArrays(GL_TRIANGLES, d->vbo_offset, d->vertex_count);
}

static void SetupRenderData() {
	crd = OGLRenderData::Create();

	crd->vertex_count = 24;
	crd->hgVbo = &staticVboVNU;
	crd->vbo_offset = staticVboVNU.add_data(raw_cube_data, crd->vertex_count);
	crd->index_count = 36;
	crd->indices.data = cube_indices;

//	crd->baseRender.renderFunc = cube_render;

//	trd->baseRender.destroy = NULL; //render data is shared by all triangles so we don't really want to do anything
}

void change_to_cube(HgElement* element) {
//	element->vptr_idx = VTABLE_INDEX;
	//create an instance of the render data for all triangles to share
	if (crd == NULL) SetupRenderData();

	element->m_renderData = (RenderData*)crd;
}
/*
void shape_create_cube(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;
	element->scale = 1.0f;

	change_to_cube(element);
}
*/

REGISTER_LINKTIME(cube, change_to_cube)