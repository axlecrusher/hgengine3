#include <HgModel.h>
#include <stdio.h>
#include <stdlib.h>
#include <HgVbo.h>



static uint8_t VTABLE_INDEX;

typedef struct header {
	uint32_t vertex_count, index_count;
} header;


static model_data LoadModel(const char* filename) {
	header head;
	model_data r;
	r.vertices = NULL;
	r.indices = NULL;

	vbo_layout_vnu* buffer1 = NULL;
	uint16_t* buffer2 = NULL;

	FILE* f = fopen(filename, "rb");
	if (f == NULL) {
		fprintf(stderr, "Unable to open file \"%s\"\n", filename);
		return r;
	}

	int32_t read = fread(&head, sizeof(head), 1, f);
	if (read != 1) {
		fprintf(stderr, "Unable to read file header for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	if (head.vertex_count > 65535) {
		fprintf(stderr, "Too many vertices for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	if (head.index_count > 1000000) {
		fprintf(stderr, "Too many indices for \"%s\"\n", filename);
		fclose(f);
		return r;
	}

	buffer1 = malloc(sizeof(*buffer1)*head.vertex_count);
	buffer2 = malloc(sizeof(*buffer2)*head.index_count);

	read = fread(buffer1, sizeof(*buffer1), head.vertex_count, f);
	if (read != head.vertex_count) {
		fprintf(stderr, "Error, %d vertices expected, read %d", head.vertex_count, read);
		free(buffer1);
		free(buffer2);
		fclose(f);
		return r;
	}

	read = fread(buffer2, sizeof(*buffer2), head.index_count, f);
	if (read != head.index_count) {
		fprintf(stderr, "Error, %d indices expected, read %d", head.index_count, read);
		free(buffer1);
		free(buffer2);
		fclose(f);
		return r;
	}

	fclose(f);

	r.vertices = buffer1;
	r.indices = buffer2;
	r.vertex_count = head.vertex_count;
	r.index_count = head.index_count;

	return r;
}

typedef struct model_render_data {
	OGLRenderData ogl_render_data;
	uint16_t* indices;
} model_render_data;

static void model_render(RenderData* rd) {
	//This can almost be generic, except for setup_ogl function call
	model_render_data* mrd = (model_render_data*)rd;
	OGLRenderData *d = &mrd->ogl_render_data;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer16(mrd->indices, d->index_count);
		free(mrd->indices);
	}

	hgvbo_use(d->hgVbo);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_SHORT, 0, d->vbo_offset);
}

static RenderData* init_render_data() {
	model_render_data* rd = calloc(1,sizeof* rd);
	rd->ogl_render_data.baseRender.renderFunc = model_render;
	return (void*)rd;
}

static void updateClbk(struct HgElement* e, uint32_t tdelta) {
	//	printf("cube\n");
}

static void destroy(struct HgElement* e) {
	free(e->m_renderData);
	e->m_renderData = NULL;
	//	printf("cube\n");
}

static HgElement_vtable vtable = {
	.destroy = destroy
//	.updateFunc = updateClbk
};

static void change_to_model(HgElement* element) {
	HGELEMT_VTABLES[VTABLE_INDEX] = vtable; //how to only do this once?
	element->vptr_idx = VTABLE_INDEX;

	//create an instance of the render data for all triangles to share
	element->m_renderData = init_render_data();
}

int8_t model_load(HgElement* element, const char* filename) {
	change_to_model(element);

	model_render_data* mrd = (model_render_data*)element->m_renderData;
	OGLRenderData* rd = &mrd->ogl_render_data;

	SET_FLAG(element, HGE_DESTROY); //clear when we make it to the end

	model_data mdl = LoadModel(filename);
	if (mdl.vertices == NULL || mdl.indices == NULL) return -1;

	rd->hgVbo = &staticVboVNU;
	rd->vertex_count = mdl.vertex_count;
	rd->index_count = mdl.index_count;
	rd->vbo_offset = hgvbo_add_data_raw(rd->hgVbo, mdl.vertices, rd->vertex_count);
	free(mdl.vertices);

//	mrd->index_count = mdl.index_count;
	mrd->indices = mdl.indices;

	CLEAR_FLAG(element, HGE_DESTROY);

	return 0;
}

/*
void model_create(HgElement* element) {
	element->position.components.x = 0.0f;
	element->position.components.y = 0.0f;
	element->position.components.z = 0.0f;

	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;

	element->scale = 1;

	change_to_model(element);
}
*/

REGISTER_LINKTIME(hgmodel);