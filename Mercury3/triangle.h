#pragma once

#include <HgEntity.h>
#include <InstancedCollection.h>

void change_to_triangle(HgEntity* entity);

namespace Triangle {
constexpr uint32_t vertexCount = 3;
constexpr uint32_t indiceCount = 3;

struct gpuStruct {
	float matrix[16];
};

class Triangle : public IUpdatableInstance<gpuStruct> {
public:
	using InstanceCollection = InstancedCollection<Triangle, gpuStruct, 1>;
	using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

	virtual void update(HgTime tdelta);
	virtual void getInstanceData(gpuStruct* instanceData);

	void init();
private:
};
}