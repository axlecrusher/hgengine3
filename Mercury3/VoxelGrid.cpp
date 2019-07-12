#include <VoxelGrid.h>
#include <cube.h>
#include <HgVbo.h>

#include <oglDisplay.h> //TODO: remove this

//instanced render data
static std::shared_ptr<RenderData> crd;
//static vbo_layout_vnut* voxelGridVertices = NULL;
//static uint16_t* indices = NULL;

std::shared_ptr<vbo_layout_vnut[]> voxelGridVertices;
std::shared_ptr<uint16_t[]> indices;

model_data generateVoxelVBO(uint8_t x, uint8_t y) {
	model_data data;
	uint32_t cube_count = x*y;

	const uint32_t vertexCount = 24 * cube_count;
	auto vertices = std::shared_ptr<vbo_layout_vnut[]>(new vbo_layout_vnut[vertexCount]);
	data.storeVertices(vertices, vertexCount);

	const uint32_t indexCount = 36 * cube_count;
	auto indices = std::shared_ptr<uint16_t[]>(new uint16_t[indexCount]);
	data.storeIndices(indices, indexCount);

	uint16_t vert_counter = 0;
	uint16_t idx_counter = 0;
	for (uint16_t i = 0; i < cube_count; i++) {
		auto vertArray = vertices.get();
		for (uint8_t ix = 0; ix < 24; ++ix) {
			vbo_layout_vnut vert = raw_cube_data[ix];
			vert.v.object.x(vert.v.object.x() + (i % x) );
			vert.v.object.y(vert.v.object.y() - ((i / x)%y));
//			vert.v.components.z += i / 100;
			vertArray[vert_counter++] = vert;
		}

		auto indexArray = indices.get();
		for (uint32_t ix = 0; ix < 36; ++ix) {
			indexArray[idx_counter++] = cube_indices[ix] + (24 * i);
		}
	}

	return data;
}

static void SetupRenderData() {
	model_data data;
	if (voxelGridVertices == NULL) {
		data = generateVoxelVBO(10, 10);
		voxelGridVertices = data.getVertices();
		indices = data.getIndices16();
	}

	//crd = (OGLRenderData*)RenderData::Create();
	crd = RenderData::Create();

	auto rec = HgVbo::GenerateFrom(voxelGridVertices.get(), data.getVertexCount());
	crd->VertexVboRecord(rec);

	auto iRec = HgVbo::GenerateFrom(indices.get(), data.getIndexCount());
	crd->indexVboRecord(iRec);
}

void change_to_voxelGrid(HgEntity* entity) {
	//create an instance of the render data for all triangles to share
	if (crd == nullptr) SetupRenderData();

	entity->setRenderData( crd );
}

void* voxelGrid_create(HgEntity* entity) {
//	entity->position.components.x = 0.0f;
//	entity->position.components.y = 0.0f;
//	entity->position.components.z = 0.0f;

//	entity->rotation.w = 1.0f;
	//	entity->rotation.z = 0.707f;
	entity->scale(1.0f);

	change_to_voxelGrid(entity);
	return nullptr;
}

REGISTER_LINKTIME(voxelGrid, voxelGrid_create);