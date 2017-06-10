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

#define VTABLE_INDEX 2

static float cube_verts[] = {
	0.5f, 0.5, 0.5,		//0
	0.5f, -0.5, 0.5,	//1
	-0.5f, -0.5, 0.5,	//2
	-0.5f, 0.5, 0.5,	//3
	0.5f, 0.5, -0.5,	//4
	0.5f, -0.5, -0.5,	//5
	-0.5f, -0.5, -0.5,	//6
	-0.5f, 0.5, -0.5	//7
};

//rgba
static color colors[] = {
	255, 0, 0, 255, //0
	0, 255, 0, 255, //1
	0, 0, 255, 255, //2
	255, 0, 0, 255, //3
	0, 255, 0, 255, //4
	0, 0, 255, 255, //5
	255, 0, 0, 255, //6
	0, 255, 0, 255 //7
};


static uint8_t indices[] = {
	2,1,0,0,3,2, //front
	5,6,7,7,4,5, //back
	6,5,1,1,2,6, //bottom
	3,0,4,4,7,3, //top
	6,2,3,3,7,6, //R side
	1,5,4,4,0,1  //L side
};

//instanced render data
static OGLRenderData *crd = NULL;

static void cube_render(RenderData* rd) {
	//This can almost be generic, except for setup_ogl function call
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer8(indices, 36);
	}

	hgvbo_use(&staticVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_BYTE, 0, d->vbo_offset);
}

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
//	printf("cube\n");
}

static void destroy(struct HgElement* e) {

	//	printf("cube\n");
}

static HgElement_vtable vtable = {
	.destroy = destroy,
	.updateFunc = updateClbk
};

static void SetupRenderData() {
	crd = calloc(1, sizeof(*crd));
	crd->baseRender.renderFunc = cube_render;
	crd->baseRender.shader = HGShader_acquire("test_vertex.glsl", "test_frag.glsl");

	vertices points;
	points.points.array = cube_verts;
	points.size = 8;

	crd->index_count = 36;
	crd->hgVbo = &staticVbo;
	crd->vbo_offset = hgvbo_add_data_vc(&staticVbo, points.points.v, colors, 8);
}

void change_to_cube(HgElement* element) {
	HGELEMT_VTABLES[VTABLE_INDEX] = vtable; //how to only do this once?

//	element->vptr = &vtable;
	element->vptr_idx = VTABLE_INDEX;
	//create an instance of the render data for all triangles to share
	if (crd == NULL) SetupRenderData();

	element->m_renderData = (RenderData*)crd;
}

void shape_create_cube(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;
	element->scale = 1.0f;

	change_to_cube(element);
}
