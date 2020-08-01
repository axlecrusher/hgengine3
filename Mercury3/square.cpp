#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
//#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <HgVbo.h>
#include <HgUtils.h>

#include <square.h>

static float vv[12] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
	-0.5f, 0.5f, 0.0f
};

static color8 colors[] = {
	{255, 0, 0, 255},
	{0, 255, 0, 255},
	{0, 0, 255, 255},
	{255, 0, 0, 255}
};

static vbo_layout_vc raw_data[] = {
	{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}},
	{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}},
	{{0.5f, 0.5f, 0.0f}, {0, 0, 255, 255}},
	{{-0.5f, 0.5f, 0.0f}, {255, 0, 0, 255}}
};

static uint8_t indices[] = {
	0,1,2,2,3,0
};

static void destroy(HgEntity* e) {
	e->destroy();
}

//instanced render data
static std::shared_ptr<RenderData> trd;


static void SetupRenderData() {
	trd = RenderData::Create();

	auto rec = HgVbo::GenerateFrom(raw_data, NUM_ARRAY_ELEMENTS(raw_data));
	trd->VertexVboRecord(rec);

	auto iRec = HgVbo::GenerateFrom(indices, NUM_ARRAY_ELEMENTS(indices));
	trd->indexVboRecord(iRec);
}

static std::shared_ptr<RenderData> init_render_data() {
	if (trd == NULL) SetupRenderData();
	return trd;
}

void* change_to_square(HgEntity* entity) {
	//create an instance of the render data for all triangles to share
	entity->setRenderData( init_render_data() );
	return nullptr;
}

void Square::getInstanceData(Instancing::GPUTransformationMatrix* instanceData)
{
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

void Square::init() {
	IUpdatableInstance<Instancing::GPUTransformationMatrix>::init();

	HgEntity* e = &getEntity();
	change_to_square(e);

	e->setRenderData(init_render_data());
}

REGISTER_LINKTIME(square,change_to_square)
REGISTER_LINKTIME2(square, Square);