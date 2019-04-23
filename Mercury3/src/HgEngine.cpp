#include <HgEngine.h>
#include <HgEntity.h>
#include <EventSystem.h>

#include <HgScene.h>

namespace ENGINE
{

GlobalInitalizer::GlobalInitalizer()
{
	HgEntity::Find(1); //force EntityLocator initilization, ewwwww
}

}

namespace Engine
{

std::unordered_map<std::string, factory_clbk> entity_factories;

std::vector<std::shared_ptr<IUpdatableCollection>>& collections() {
	static std::vector<std::shared_ptr<IUpdatableCollection>> c;
	return c;
}

void updateCollections(HgTime dtime) {
	auto c = collections();
	for (auto i : c) {
		i->update(dtime);
	}
}

void EnqueueForRender(std::vector<std::shared_ptr<IUpdatableCollection>>& c, RenderQueue* queue) {
	for (auto i : c) {
		i->EnqueueForRender(queue);
	}
}

bool create_entity(char* type, HgScene* scene, HgEntity** entity)
{
	auto factory = entity_factories.find(type);

	if (factory == entity_factories.end()) {
		fprintf(stderr, "Unable to find entity type \"%s\"\n", type);
		return false;
	}
	factory_clbk clbk = factory->second;
	scene->getNewEntity(entity);
	clbk(*entity);

	return true;
}

bool create_entity(const char* type, HgEntity** entity)
{
	auto factory = entity_factories.find(type);

	if (factory == entity_factories.end()) {
		fprintf(stderr, "Unable to find entity type \"%s\"\n", type);
		return false;
	}
	factory_clbk clbk = factory->second;
	*entity = (HgEntity*)clbk(*entity);

	return true;
}

} //ENGINE
