#include <stdint.h>
#include <vertex.h>
#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <assert.h>

#include <memory.h>

#include <HgVbo.h>
#include <HgElement.h>
#include <HgUtils.h>

#include <InstancedCollection.h>
#include <cube.h>

vertex3f test__ = { 1.0,0.0,0.0 };

//dump from a hgmdl model, VNUT format.
//So many vertices because of different normals
vbo_layout_vnut raw_cube_data[24] = {
	{{0.5, -0.5, 0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, -0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},	//left bottom 1
	{{0.5, -0.5, -0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, -0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, 0.5, 0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},		//top right 4
	{{0.5, 0.5, -0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, 0.5, -0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, -0.5, 0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, -0.5, -0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, 0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, -0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, 0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, -0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, -0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, -0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, 0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, 0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, 0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{0.5, 0.5, 0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, 0.5, 0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767},
	{{-0.5, -0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 32767, 32767}
};

uint8_t cube_indices[36] = {
	0, 1, 2,		3, 4, 5,
	0, 18, 1,		3, 19, 4,

	6, 7, 8,		9, 10, 11,	
	6, 20, 7,		9, 21, 10,

	12, 13, 14,		15, 16, 17,	
	12, 22, 13,		15, 23, 16
};

static auto Collection = InstancedCollection<RotatingCube::RotatingCube, RotatingCube::gpuStruct, 1>::Collection;

//instanced render data
static std::shared_ptr<RenderData> crd;

static void destroy(HgElement* e) {
	e->destroy();
}

//Draw vertices directly. We aren't using indices here,
static void cube_render(RenderData* rd) {
	setRenderAttributes(rd->blendMode, rd->renderFlags);
	rd->hgVbo()->use();
	rd->hgVbo()->draw(rd);
}

static void SetupRenderData() {
	crd = OGLRenderData::Create();

	auto rec = HgVbo::GenerateFrom(raw_cube_data, NUM_ARRAY_ELEMENTS(raw_cube_data));
	crd->VertexVboRecord(rec);

	auto iRec = HgVbo::GenerateFrom(cube_indices, NUM_ARRAY_ELEMENTS(cube_indices));
	crd->indexVboRecord(iRec);
}

void* change_to_cube(HgElement* element) {
	//create an instance of the render data for all triangles to share
	if (crd == nullptr) SetupRenderData();

	element->setRenderData(crd);
	return nullptr;
}

namespace RotatingCube {
	void RotatingCube::update(HgTime dt, gpuStruct* instanceData) {
		using namespace HgMath;
		m_age += dt;
		if (m_age.msec() > 10000) {
			m_age -= HgTime::msec(10000);
		}
		const quaternion rotation = quaternion::fromEuler(angle::ZERO, angle::deg(m_age.msec() / 27.777777777777777777777777777778), angle::ZERO);
		getElement().orientation(rotation);
		instanceData->rotation = rotation;
	}

	void RotatingCube::init() {
		HgElement* e = &getElement();
		change_to_cube(e);
	}

	RotatingCube& RotatingCube::Generate() {
		static bool init = false;
		if (init == false) {
			Engine::collections().push_back(&Collection());
			init = true;
		}
		return Collection().newItem();
	}
}

static void* generate_rotating_cube(HgElement* element) {
	RotatingCube::RotatingCube& p = RotatingCube::RotatingCube::Generate();
	p.init();
	return &p;
}

REGISTER_LINKTIME(rotating_cube, generate_rotating_cube);
REGISTER_LINKTIME(cube, change_to_cube)