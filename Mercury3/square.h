#pragma once

#include <HgVbo.h>
#include <HgEntity.h>

#include <string>
#include <IniLoader.h>
#include <InstancedCollection.h>

class Square : public IUpdatableInstance<Instancing::GPUTransformationMatrix> {
public:
	using InstanceCollection = InstancedCollection<Square, Instancing::GPUTransformationMatrix, 1>;
	using InstanceCollectionPtr = std::shared_ptr<InstanceCollection>;

	virtual void update(HgTime tdelta) {}
	virtual void getInstanceData(Instancing::GPUTransformationMatrix* instanceData);

	void init();
};

void* change_to_square(HgEntity* entity);
void change_to_square(EntityIdType id);
