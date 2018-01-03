#include <VoxelGrid.h>
#include <cube.h>
#include <HgVbo.h>

//instanced render data
static OGLRenderData *crd = NULL;
static vbo_layout_vnut* voxelGridVertices = NULL;
static uint16_t* indices = NULL;

model_data generateVoxelVBO(uint8_t x, uint8_t y) {
	model_data data;
	uint32_t cube_count = x*y;

	data.vertices = (vbo_layout_vnut*)malloc(24 * cube_count * sizeof(*data.vertices));
	data.vertex_count = 24 * cube_count;

	data.indices = (uint16_t*)malloc(36 * cube_count * sizeof(*data.indices));
	data.index_count = 36 * cube_count;

	uint16_t vert_counter = 0;
	uint16_t idx_counter = 0;
	for (uint16_t i = 0; i < cube_count; i++) {
		for (uint8_t ix = 0; ix < 24; ++ix) {
			vbo_layout_vnut vert = raw_cube_data[ix];
			vert.v.components.x += i % x;
			vert.v.components.y -= (i / x)%y;
//			vert.v.components.z += i / 100;
			data.vertices[vert_counter++] = vert;
		}

		for (uint32_t ix = 0; ix < 36; ++ix) {
			data.indices[idx_counter++] = cube_indices[ix] + (24 * i);
		}
	}
//	voxelGridVertices = vertices;
	return data;
}

//Draw vertices directly. We aren't using indices here,
static void render(RenderData* rd) {
	//Special render call, uses uint16_t as indices rather than uint8_t that the rest of the engine uses
	OGLRenderData *d = (OGLRenderData*)rd;
	if (d->idx_id == 0) {
		d->idx_id = new_index_buffer16((uint16_t*)d->indices.data, d->index_count);
//		free_arbitrary(&d->indices);
	}

	setBlendMode((BlendMode)rd->blendMode);
	d->hgVbo->use();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, d->idx_id);
	glDrawElementsBaseVertex(GL_TRIANGLES, d->index_count, GL_UNSIGNED_SHORT, 0, d->vbo_offset);
}

static void SetupRenderData() {
	model_data data;
	if (voxelGridVertices == NULL) {
		data = generateVoxelVBO(10, 10);
		voxelGridVertices = data.vertices;
		indices = data.indices;
	}

	crd = (OGLRenderData*)RenderData::Create();

	crd->vertex_count = data.vertex_count;
	crd->hgVbo = &staticVboVNU;
	crd->vbo_offset = staticVboVNU.add_data(voxelGridVertices, crd->vertex_count);

	crd->indices.data = indices;
	crd->index_count = data.index_count;
//	rd->indices.owns_ptr = 1;

	crd->renderFunction = render;
}

void change_to_voxelGrid(HgElement* element) {
	//create an instance of the render data for all triangles to share
	if (crd == NULL) SetupRenderData();

	element->m_renderData = (RenderData*)crd;
}

void voxelGrid_create(HgElement* element) {
//	element->position.components.x = 0.0f;
//	element->position.components.y = 0.0f;
//	element->position.components.z = 0.0f;

//	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;
	element->scale = 1.0f;

	change_to_voxelGrid(element);
}
/*
static void destroy(HgElement* e) {
	e->m_renderData = NULL;
}

static HgElement_vtable vtable = {
	.create = voxelGrid_create,
	.destroy = destroy,
	.updateFunc = NULL
};
*/
REGISTER_LINKTIME(voxelGrid, voxelGrid_create);