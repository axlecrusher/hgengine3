#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

static vtable_index VTABLE_INDEX;

static float vv[9] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f
};

static color colors[] = {
	255, 0, 0, 255,
	0, 255, 0, 255,
	0, 0, 255, 255
};

static uint8_t indices[] = {
	0,1,2
};

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
	//	printf("cube\n");
}

static void destroy(HgElement* e) {
	e->m_renderData = NULL;
}

//instanced render data
static OGLRenderData* trd = NULL;

static void SetupRenderData() {
	trd = new_RenderData();

	vertices points;
	points.points.array = vv;
	points.size = 3;

	trd->hgVbo = &staticVbo;
	trd->vertex_count = points.size;
	trd->index_count = 3;
	trd->indices.data = indices;
	trd->vbo_offset = hgvbo_add_data_vc(&staticVbo, points.points.v, colors, trd->vertex_count);

	trd->baseRender.destroy = NULL; //render data is shared by all triangles so we don't really want to do anything
}

OGLRenderData* triangle_init_render_data() {
	if (trd == NULL) SetupRenderData();
	return trd;
}

void change_to_triangle(HgElement* element) {
	//create an instance of the render data for all triangles to share
	element->m_renderData = (RenderData*)triangle_init_render_data();
}

void shape_create_triangle(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_triangle(element);
}


static HgElement_vtable vtable = {
	.create = shape_create_triangle,
	.destroy = destroy,
	.updateFunc = NULL
};

REGISTER_LINKTIME(triangle)