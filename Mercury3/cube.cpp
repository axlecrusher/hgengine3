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

namespace RotatingCube
{
	SpinningCubeTables SpinningCubes;

	void initRotatingCubes(const EntityIdList& list)
	{
		SpinningCubes.entityId.reserve(SpinningCubes.entityId.size() + list.size());
		SpinningCubes.cubeState.reserve(SpinningCubes.cubeState.size() + list.size());
		for (auto id : list)
		{
			initRotatingCube(id);
			//SpinningCubes.entityId.push_back(id);
			//CubeState state;
			//state.rotationTime = HgTime::msec(10000 + ((rand() % 10000) - 5000));
			//SpinningCubes.cubeState.push_back(state);
		}
	}

	void initRotatingCube(EntityIdType id)
	{
		SpinningCubes.entityId.push_back(id);
		CubeState state;
		state.rotationTime = HgTime::msec(10000 + ((rand() % 10000) - 5000));
		SpinningCubes.cubeState.push_back(state);
	}

	void gcRotatingCubes()
	{
		//SpinningCubes
	}

	void updateRotatingCubes(HgTime dt)
	{
		using namespace HgMath;

		auto& idTable = EntityIdTable::Singleton();
		auto& entityTable = EntityTable::Singleton();
		for (int i = 0; i < SpinningCubes.entityId.size(); i++)
		{
			auto& state = SpinningCubes.cubeState[i];
			
			state.age += dt;
			while (state.age > state.rotationTime) {
				state.age -= state.rotationTime;
			}

			const double degTime = 360.0 / state.rotationTime.msec();

			const double deg = degTime * state.age.msec();
			const quaternion rotation = quaternion::fromEuler(angle::ZERO, angle::deg(deg), angle::ZERO);
			entityTable.getPtr(&idTable, SpinningCubes.entityId[i])->orientation(rotation);
		}
	}
}

namespace Cube {

void Cube::update(HgTime dt) {
}

void Cube::getInstanceData(Instancing::GPUTransformationMatrix* instanceData) {
	const auto mat = getEntity().computeWorldSpaceMatrix();
	mat.store(instanceData->matrix);
}

void Cube::init() {
	IUpdatableInstance<Instancing::GPUTransformationMatrix>::init();

	HgEntity* e = &getEntity();
	change_to_cube(e);

	RenderDataPtr rd = std::make_shared<RenderData>(*crd);
	e->setRenderData(rd);
}

void changeToCube(const EntityIdList& list)
{
	for (auto id : list)
	{
		auto cube = EntityTable::Singleton().getPtr(id);
		change_to_cube(cube);
	}
}

} //namespace Cubee

namespace RotatingCube {

	void DataOrientedStruct::update(HgTime dt)
	{
		for (uint32_t i = 0; i < cubeState.size(); i++)
		{
			using namespace HgMath;

			auto& state = cubeState[i];

			state.age += dt;
			const double degTime = 360.0 / state.rotationTime.msec();

			while (state.age > state.rotationTime) {
				state.age -= state.rotationTime;
			}
			const double deg = degTime * state.age.msec();
			//rotation[i] = quaternion::fromEuler(angle::ZERO, angle::deg(deg), angle::ZERO);
			const auto r = quaternion::fromEuler(angle::ZERO, angle::deg(deg), angle::ZERO);

			spacial[i].orientation = r;
			//const auto mat = computeTransformMatrix(spacial[i]); //don't want to do this here, update can be called multiple times per frame
		}

	}

	RotatingCube2* DataOrientedStruct::newItem()
	{
		size_t idx = cubeState.size();

		cubeState.emplace_back();
		//rotation.emplace_back();
		spacial.emplace_back();
		entities.emplace_back();
		gpuStructArray.emplace_back();
		renderData.emplace_back();
		glueClass.emplace_back(this, idx);

		cubeState[idx].rotationTime = HgTime::msec(10000 + ((rand() % 10000) - 5000));

		auto& e = entities[idx];
		e.init();
		change_to_cube(&e);
		//RenderDataPtr rd = std::make_shared<RenderData>(*crd);
		//e.setRenderData(rd);

		return &glueClass[idx];
	}

	void DataOrientedStruct::EnqueueForRender(RenderQueue* queue, HgTime dt)
	{
		auto rd = entities[0].getRenderDataPtr();

		for (uint32_t i = 0; i < spacial.size(); i++)
		{
			const auto mat = computeTransformMatrix(spacial[i]);
			mat.store(gpuStructArray[i].matrix);
		}

		gpuBuffer->setDataSource(gpuStructArray);
		gpuBuffer->setNeedsLoadToGPU(true);

		Instancing::InstancingMetaData imd;
		imd.instanceCount = gpuStructArray.size();
		imd.renderData = rd.get();
		imd.instanceData = gpuBuffer;

		queue->Enqueue(imd);
	}

	void RotatingCube2::setPosition(vertex3f p) { m_data->spacial[m_instanceIndex].position = p; }
	vertex3f RotatingCube2::getPosition() const { return m_data->spacial[m_instanceIndex].position; }

	void RotatingCube2::setScale(float s) { m_data->spacial[m_instanceIndex].scale = s; }
	float RotatingCube2::getScale() const { return m_data->spacial[m_instanceIndex].scale; }

	HgEntity& RotatingCube2::getEntity()
	{
		return m_data->entities[m_instanceIndex];
	}
	
	const HgEntity&RotatingCube2::getEntity() const
	{
		return m_data->entities[m_instanceIndex];
	}


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

	void RotatingCube::getInstanceData(Instancing::GPUTransformationMatrix* instanceData) {
		const auto mat = getEntity().computeWorldSpaceMatrix();
		mat.store(instanceData->matrix);
	}

	void RotatingCube::init() {
		IUpdatableInstance<Instancing::GPUTransformationMatrix>::init();

		HgEntity* e = &getEntity();
		change_to_cube(e);

		m_rotationTime = HgTime::msec(10000 + ((rand() % 10000) - 5000));

		//RenderDataPtr rd = std::make_shared<RenderData>(*crd);
		//e->setRenderData(rd);
	}
}

//REGISTER_LINKTIME3(rotating_cube, RotatingCube::RotatingCube, RotatingCube::RotatingCube::factory);
REGISTER_LINKTIME3(rotating_cube2, RotatingCube::RotatingCube2, RotatingCube::RotatingCube2InstanceCollection);
REGISTER_LINKTIME2(rotating_cube, RotatingCube::RotatingCube);
REGISTER_LINKTIME2(cube, Cube::Cube);
