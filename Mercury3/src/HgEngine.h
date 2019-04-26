#pragma once
/* This file should only be included once in the main cpp file of the program.*/

#include <UpdatableCollection.h>
#include <InstancedCollection.h>

namespace ENGINE
{

class GlobalInitalizer
{
public:
	GlobalInitalizer();
private:
};

}


namespace Engine
{

extern std::unordered_map<std::string, factory_clbk> entity_factories;

//Create simplistic entities existing in HgScene. Nothing complex. Avoid.
bool create_entity(char* type, ::HgScene* scene, HgEntity** entity);

//Create an entity in a data structure fit to contain entities of type_str.
//Returns true if successful. Sets entity to the created entity.
bool create_entity(const char* type_str, HgEntity** entity);

std::vector<std::shared_ptr<IUpdatableCollection>>& collections();
void updateCollections(HgTime dtime);
void EnqueueForRender(std::vector<std::shared_ptr<IUpdatableCollection>>& c, RenderQueue* queue);

template<typename T>
std::shared_ptr<T> getCollectionOf()
{
	static std::weak_ptr<T> collection;

	std::shared_ptr<T> ptr = collection.lock();
	if (ptr == nullptr)
	{
		collection = ptr = std::make_shared<T>();
		Engine::collections().push_back(ptr);
	}

	return ptr;
}

template<typename T>
std::shared_ptr<UpdatableCollection<T>> getUpdatableCollection()
{
	static std::weak_ptr<UpdatableCollection<T>> collection;

	std::shared_ptr<UpdatableCollection<T>> ptr = collection.lock();
	if (ptr == nullptr)
	{
		collection = ptr = std::make_shared<UpdatableCollection<T>>();
		Engine::collections().push_back(ptr);
	}

	return ptr;
}

} //Engine

static ENGINE::GlobalInitalizer _globalEngineInit;