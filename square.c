#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

static uint8_t VTABLE_INDEX;

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

static uint8_t indices[] = {
	0,1,2,2,3,0
};

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
}

static void destroy(struct HgElement* e) {
}

static HgElement_vtable vtable = {
	.destroy = NULL,
	.updateFunc = NULL
};

//instanced render data
static OGLRenderData* trd = NULL;

static void render(RenderData* rd) {
	//This can almost be generic, except for setup_ogl function call
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer8(indices, trd->index_count);
	}

	hgvbo_use(&staticVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_BYTE, 0, d->vbo_offset);
}

static void SetupRenderData() {
	trd = calloc(1, sizeof(*trd));
	trd->baseRender.renderFunc = render;
	trd->baseRender.shader = HGShader_acquire("test_vertex.glsl", "test_frag.glsl");

	vertices points;
	points.points.array = vv;
	points.size = 4;

	trd->hgVbo = &staticVbo;
	trd->vertex_count = points.size;
	trd->index_count = 6;
	trd->vbo_offset = hgvbo_add_data_vc(&staticVbo, points.points.v, colors, trd->vertex_count);
}

static OGLRenderData* init_render_data() {
	if (trd == NULL) SetupRenderData();
	return trd;
}

static void change_to_square(HgElement* element) {
	//create an instance of the render data for all triangles to share
	element->m_renderData = (RenderData*)init_render_data();
}

void shape_create_square(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_square(element);
}

REGISTER_LINKTIME(square)