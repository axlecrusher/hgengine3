#pragma once

#include <HgElement.h>
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
}

namespace RotatingCube {
	struct gpuStruct {
		quaternion rotation;
	};

	class RotatingCube : public IUpdatableInstance<gpuStruct> {
	public:
		static RotatingCube& RotatingCube::Generate();

		virtual void update(HgTime tdelta) {}; //ew
		virtual void update(HgTime tdelta, gpuStruct* instanceData);
		void init();
	private:
		HgTime m_age;
	};
}
