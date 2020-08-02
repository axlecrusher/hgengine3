#pragma once

#include <HgEntity.h>
#include <InstancedCollection.h>

void change_to_triangle(HgEntity* entity);
void change_to_triangle(EntityIdType id);

namespace Triangle {
constexpr uint32_t vertexCount = 3;
constexpr uint32_t indiceCount = 3;

struct gpuStruct {
	float matrix[16];
};

//Initialize a list of entites to triangle models
void init(const EntityIdList& list);

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