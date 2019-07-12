#pragma once

#include <HgEntity.h>
#include <HgVbo.h>
#include <InstancedCollection.h>

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

	struct gpuStruct {
		float matrix[16];
	};

	class Cube : public IUpdatableInstance<gpuStruct> {
	public:
		using InstanceCollection = InstancedCollection<Cube, gpuStruct, 1>;
		using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		virtual void update(HgTime tdelta) {}; //ew
		virtual void update(HgTime tdelta, gpuStruct* instanceData);
		void init();
	private:
	};
}

namespace RotatingCube {
	struct gpuStruct {
		float matrix[16];
	};

	class RotatingCube : public IUpdatableInstance<gpuStruct> {
	public:
		using InstanceCollection = InstancedCollection<RotatingCube, gpuStruct, 1>;
		using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

		virtual void update(HgTime tdelta) {}; //ew
		virtual void update(HgTime tdelta, gpuStruct* instanceData);
		void init();
	private:
		HgTime m_age;
		HgTime m_rotationTime;
	};
}
