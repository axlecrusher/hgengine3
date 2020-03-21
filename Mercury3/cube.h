#pragma once

#include <HgEntity.h>
#include <HgVbo.h>
#include <InstancedCollection.h>

#include <core/DataOrientedCollection.h>

extern vbo_layout_vnut raw_cube_data[24];
extern uint8_t cube_indices[36];

struct CubeVertices {
	vbo_layout_vnut vertices[24];
};

struct CubeIndices {
	uint8_t indices[36];
};

namespace Cube {
	constexpr uint32_t vertexCount = 24;
	constexpr uint32_t indiceCount = 36;

	class Cube : public IUpdatableInstance<Instancing::GPUTransformationMatrix> {
	public:
		using InstanceCollection = InstancedCollection<Cube, Instancing::GPUTransformationMatrix, 1>;
		using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		virtual void update(HgTime tdelta);
		virtual void getInstanceData(Instancing::GPUTransformationMatrix* instanceData);

		void init();
	private:
	};
}

namespace RotatingCube {
	struct CubeState
	{
		HgTime age;
		HgTime rotationTime;
	};

	struct DataOrientedStruct; //forward declare

	class RotatingCube2
	{
	public:
		RotatingCube2(DataOrientedStruct* data, uint32_t idx)
			:m_data(data), m_instanceIndex(idx)
		{}

		HgEntity& getEntity();
		const HgEntity& getEntity() const;

		void setPosition(vertex3f p);
		vertex3f getPosition() const;

		void setScale(float s);
		float getScale() const;

		DataOrientedStruct* m_data;
		uint32_t m_instanceIndex;
	};

	struct DataOrientedStruct
	{
		DataOrientedStruct()
		{
			gpuBuffer = std::make_shared< HgGPUBufferSegment<Instancing::GPUTransformationMatrix> >();
		}

		void update(HgTime dt);
		RotatingCube2& newItem();
		void EnqueueForRender(RenderQueue* queue, HgTime dt);

		std::vector<CubeState> cubeState;
		//std::vector<quaternion> rotation;
		std::vector<SPI> spacial;
		std::vector<HgEntity> entities;
		std::vector<RenderData> renderData;
		std::vector<Instancing::GPUTransformationMatrix> gpuStructArray;
		std::vector<RotatingCube2> glueClass;

		std::shared_ptr< HgGPUBufferSegment<Instancing::GPUTransformationMatrix> > gpuBuffer;
	};

	using RotatingCube2InstanceCollection = DataOrientedCollection<RotatingCube2, DataOrientedStruct>;
	using RotatingCube2InstanceCollectionPtr = std::shared_ptr<RotatingCube2InstanceCollection>;

	class RotatingCube : public IUpdatableInstance<Instancing::GPUTransformationMatrix> {
	public:
		using InstanceCollection = InstancedCollection<RotatingCube, Instancing::GPUTransformationMatrix, 1>;
		using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		virtual void update(HgTime tdelta);
		virtual void getInstanceData(Instancing::GPUTransformationMatrix* instanceData);

		void init();
	private:
		HgTime m_age;
		HgTime m_rotationTime;
	};
}
