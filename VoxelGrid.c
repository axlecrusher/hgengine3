#include <VoxelGrid.h>
#include <cube.h>
#include <HgVbo.h>

static uint8_t VTABLE_INDEX;

//instanced render data
static OGLRenderData *crd = NULL;
static vbo_layout_vnu* voxelGridVertices = NULL;

static void generateVoxelVBO() {
	vbo_layout_vnu* cube_verts = (vbo_layout_vnu*)raw_cube_data;
	vbo_layout_vnu* vertices = (vbo_layout_vnu*)malloc(36 * 100 * sizeof(*vertices));
	uint16_t vert_counter = 0;
	for (uint16_t i = 0; i < 100; i++) {
		for (uint8_t x = 0; x < 36; ++x) {
			vbo_layout_vnu vert = cube_verts[x];
			vert.v.components.x += i % 10;
			vert.v.components.y += (i / 10)%10;
//			vert.v.components.z += i / 100;
			vertices[vert_counter++] = vert;
		}
	}
	voxelGridVertices = vertices;
}

//Draw vertices directly. We aren't using indices here,
static void render(RenderData* rd) {
	OGLRenderData *d = (OGLRenderData*)rd;

	setBlendMode(rd->blendMode);
	hgvbo_use(d->hgVbo);

	glDrawArrays(GL_TRIANGLES, d->vbo_offset, d->vertex_count);
}

static void SetupRenderData() {
	if (voxelGridVertices == NULL) generateVoxelVBO();
	crd = new_RenderData();

	crd->vertex_count = 36*100;
	crd->hgVbo = &staticVboVNU;
	crd->vbo_offset = hgvbo_add_data_vnu_raw(crd->hgVbo, voxelGridVertices, crd->vertex_count);

	crd->baseRender.renderFunc = render;
}

void change_to_voxelGrid(HgElement* element) {
	element->vptr_idx = VTABLE_INDEX;
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

static void destroy(HgElement* e) {
	e->m_renderData = NULL;
}

static HgElement_vtable vtable = {
	.destroy = destroy,
	.updateFunc = NULL
};

REGISTER_LINKTIME(voxelGrid);