#include <triangle.h>
#include <oglDisplay.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <HgVbo.h>
//static vtable_index VTABLE_INDEX;
//instanced render data
static std::shared_ptr<RenderData> trd;

static vbo_layout_vc raw_data[] = {
	{{-0.5f, -0.5f, 0.0f}, {255, 0, 0, 255}},
	{{0.5f, -0.5f, 0.0f}, {0, 255, 0, 255}},
	{{0.0f, 0.5f, 0.0f }, {0, 0, 255, 255}}
};

static uint8_t indices[] = {
	0,1,2
};

static void SetupRenderData() {
	trd = RenderData::Create();

	vertices points;
//	points.points.array = vv;
	points.size = 3;

	auto rec = HgVbo::GenerateFrom(raw_data, points.size);
	trd->VertexVboRecord(rec);

	auto iRec = HgVbo::GenerateFrom(indices, 3);
	trd->indexVboRecord(iRec);
}

std::shared_ptr<RenderData> triangle_init_render_data() {
	if (trd == NULL) SetupRenderData();
	return trd;
}

void change_to_triangle(HgEntity* entity) {
	//create an instance of the render data for all triangles to share
	entity->setRenderData( triangle_init_render_data());
}

static void* triangle_factory(HgEntity* e) {
	change_to_triangle(e);
	return nullptr;
}

REGISTER_LINKTIME(triangle, triangle_factory)