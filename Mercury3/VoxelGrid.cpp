#include <VoxelGrid.h>
#include <cube.h>
#include <HgVbo.h>

#include <oglDisplay.h> //TODO: remove this

//instanced render data
static std::shared_ptr<RenderData> crd;
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
			vert.v.object.x(vert.v.object.x() + (i % x) );
			vert.v.object.y(vert.v.object.y() - ((i / x)%y));
//			vert.v.components.z += i / 100;
			data.vertices[vert_counter++] = vert;
		}

		for (uint32_t ix = 0; ix < 36; ++ix) {
			data.indices[idx_counter++] = cube_indices[ix] + (24 * i);
		}
	}
//	voxelGridVertices = vertices;
	return std::move(data);
}

//Draw vertices directly. We aren't using indices here,
static void render(RenderData* rd) {
	//Special render call, uses uint16_t as indices rather than uint8_t that the rest of the engine uses
	//OGLRenderData *d = (OGLRenderData*)rd;

	setRenderAttributes(rd->blendMode, rd->renderFlags);

	rd->hgVbo()->use();
	rd->indexVbo()->use();
	rd->indexVbo()->draw(rd);
}

static void SetupRenderData() {
	model_data data;
	if (voxelGridVertices == NULL) {
		data = generateVoxelVBO(10, 10);
		voxelGridVertices = data.vertices;
		indices = data.indices;
	}

	//crd = (OGLRenderData*)RenderData::Create();
	crd = RenderData::Create();

	crd->vertex_count = data.vertex_count;
	crd->hgVbo(staticVboVNUT);
	crd->vbo_offset = staticVboVNUT->add_data(voxelGridVertices, crd->vertex_count);

//	crd->indices.data = indices;
	crd->index_count = data.index_count;
//	rd->indices.owns_ptr = 1;
	crd->indexVbo( HgVbo::Create<uint16_t>() );
	crd->indexVbo()->add_data(indices, data.index_count);

//	crd->renderFunction = render;
}

void change_to_voxelGrid(HgElement* element) {
	//create an instance of the render data for all triangles to share
	if (crd == nullptr) SetupRenderData();

	element->setRenderData( crd );
}

void* voxelGrid_create(HgElement* element) {
//	element->position.components.x = 0.0f;
//	element->position.components.y = 0.0f;
//	element->position.components.z = 0.0f;

//	element->rotation.w = 1.0f;
	//	element->rotation.z = 0.707f;
	element->scale(1.0f);

	change_to_voxelGrid(element);
	return nullptr;
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