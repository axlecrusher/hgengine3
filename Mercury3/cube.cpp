#include <stdint.h>
#include <vertex.h>
//#include <shapes.h>

#include <stdlib.h>
#include <stdio.h>
#include <oglShaders.h>

#include <assert.h>

#include <memory.h>

#include <HgVbo.h>
#include <HgEntity.h>
#include <HgUtils.h>

#include <InstancedCollection.h>
#include <cube.h>
#include <core/HgScene2.h>

vertex3f test__ = { 1.0,0.0,0.0 };

//dump from a hgmdl model, VNUT format.
//So many vertices because of different normals
vbo_layout_vnut raw_cube_data[24] = {
	{{0.5, -0.5, 0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, -0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},	//left bottom 1
	{{0.5, -0.5, -0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, -0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, 0.5, 0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},		//top right 4
	{{0.5, 0.5, -0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, 0.5, -0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, -0.5, 0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, -0.5, -0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, 0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, -0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, 0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, -0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, -0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, -0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, 0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, 0.5}, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, 0.5}, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{0.5, 0.5, 0.5}, 1.0, -0.0, 0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, 0.5}, 0.0, -0.0, 1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, 0.5, 0.5}, -1.0, -0.0, -0.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}},
	{{-0.5, -0.5, -0.5}, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 0.0, {32767, 32767}}
};

uint8_t cube_indices[36] = {
	0, 1, 2,		3, 4, 5,
	0, 18, 1,		3, 19, 4,

	6, 7, 8,		9, 10, 11,	
	6, 20, 7,		9, 21, 10,

	12, 13, 14,		15, 16, 17,	
	12, 22, 13,		15, 23, 16
};

//instanced render data
static std::shared_ptr<RenderData> crd;

static void SetupRenderData() {
	crd = RenderData::Create();

	auto rec = HgVbo::GenerateFrom(raw_cube_data, NUM_ARRAY_ELEMENTS(raw_cube_data));
	crd->VertexVboRecord(rec);

	auto iRec = HgVbo::GenerateFrom(cube_indices, NUM_ARRAY_ELEMENTS(cube_indices));
	crd->indexVboRecord(iRec);
}

void* change_to_cube(HgEntity* entity) {
	//create an instance of the render data for all triangles to share
	if (crd == nullptr) SetupRenderData();

	entity->setRenderData(crd);
	return nullptr;
}

namespace Cube {

void Cube::update(HgTime dt) {
}

void Cube::getInstanceData(gpuStruct* instanceData) {
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

void Cube::init() {
	IUpdatableInstance<gpuStruct>::init();

	HgEntity* e = &getEntity();
	change_to_cube(e);

	RenderDataPtr rd = std::make_shared<RenderData>(*crd);
	e->setRenderData(rd);
}
}

namespace RotatingCube {

	void RotatingCube::update(HgTime dt) {
		using namespace HgMath;
		m_age += dt;

		const double degTime = 360.0 / m_rotationTime.msec();

		while (m_age > m_rotationTime) {
			m_age -= m_rotationTime;
		}

		const double deg = degTime * m_age.msec();
		const quaternion rotation = quaternion::fromEuler(angle::ZERO, angle::deg(deg), angle::ZERO);
		getEntity().orientation(rotation);
	}

	void RotatingCube::getInstanceData(gpuStruct* instanceData) {
		const auto mat = getEntity().computeWorldSpaceMatrix();
		mat.store(instanceData->matrix);
	}

	void RotatingCube::init() {
		IUpdatableInstance<gpuStruct>::init();

		HgEntity* e = &getEntity();
		change_to_cube(e);

		m_rotationTime = HgTime::msec(10000 + ((rand() % 10000) - 5000));

		RenderDataPtr rd = std::make_shared<RenderData>(*crd);
		e->setRenderData(rd);
	}
}

REGISTER_LINKTIME2(rotating_cube, RotatingCube::RotatingCube);
REGISTER_LINKTIME2(cube, Cube::Cube);
