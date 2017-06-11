#include <HgModel.h>
#include <stdio.h>
#include <stdlib.h>
#include <HgVbo.h>

typedef struct header {
	uint32_t vertex_count, index_count;
} header;


static model_data LoadModel(const char* filename) {
	header head;
	vbo_layout_vnu* buffer1 = NULL;
	uint16_t* buffer2 = NULL;

	FILE* f = fopen(filename, "rb");
	fread(&head, sizeof(head), 1, f);

	buffer1 = malloc(sizeof(*buffer1)*head.vertex_count);
	buffer2 = malloc(sizeof(*buffer2)*head.index_count);

	int32_t r1 = fread(buffer1, sizeof(*buffer1), head.vertex_count, f);
	int32_t r2 = fread(buffer2, sizeof(*buffer2), head.index_count, f);

	fclose(f);

	printf("vertices:%d\nindices:%d\n", head.vertex_count, head.index_count);

	model_data r;
	r.vertices = buffer1;
	r.indices = buffer2;
	r.vertex_count = head.vertex_count;
	r.index_count = head.index_count;

	return r;
}

typedef struct model_render_data {
	OGLRenderData ogl_render_data;
	uint16_t* indices;
	uint32_t index_count;
} model_render_data;

static void model_render(RenderData* rd) {
	//This can almost be generic, except for setup_ogl function call
	model_render_data* mrd = (model_render_data*)rd;
	OGLRenderData *d = &mrd->ogl_render_data;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer16(mrd->indices, mrd->index_count);
	}

	hgvbo_use(d->hgVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_SHORT, 0, d->vbo_offset);
//	glDrawElementsBaseVertex(GL_TRIANGLE_FAN, d->index_count, GL_UNSIGNED_SHORT, 0, d->vbo_offset);
//	glDrawElementsBaseVertex(GL_QUADS, d->index_count, GL_UNSIGNED_SHORT, 0, d->vbo_offset);
	//	glDrawElementsBaseVertex(GL_TRIANGLE_STRIP, d->index_count, GL_UNSIGNED_BYTE, 0, d->vbo_offset);
}

static RenderData* init_render_data() {
	model_render_data* rd = calloc(1,sizeof* rd);

	rd->ogl_render_data.baseRender.renderFunc = model_render;
	rd->ogl_render_data.baseRender.shader = HGShader_acquire("test_vertex2.glsl", "test_frag2.glsl");

	return (void*)rd;
}

void model_load(HgElement* element, const char* filename) {
	model_render_data* mrd = (model_render_data*)element->m_renderData;
	OGLRenderData* rd = &mrd->ogl_render_data;

	model_data mdl = LoadModel(filename);

	rd->hgVbo = &staticVboVNU;
	rd->vertex_count = mdl.vertex_count;
	rd->index_count = mdl.index_count;
	rd->vbo_offset = hgvbo_add_data_vnu_raw(rd->hgVbo, mdl.vertices, rd->vertex_count);
	mrd->index_count = mdl.index_count;
	mrd->indices = mdl.indices;
}

void change_to_model(HgElement* element) {
	//	element->vptr = &vtable;
	element->vptr_idx = 0;

	//create an instance of the render data for all triangles to share
	element->m_renderData = init_render_data();
}

void model_create(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_model(element);
}
