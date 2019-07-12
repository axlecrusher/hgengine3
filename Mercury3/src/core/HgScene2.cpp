#pragma once

#include <HgScene2.h>
#include <HgEngine.h>

namespace Engine {

std::unordered_map<std::string, factoryCallback> HgScene::m_entityFactories;

HgEntity* HgScene::create_entity(const char* type_str)
{
	auto factory = m_entityFactories.find(type_str);

	if (factory == m_entityFactories.end()) {
		fprintf(stderr, "Unable to find entity type \"%s\"\n", type_str);
		return nullptr;
	}
	factoryCallback clbk = factory->second;
	HgEntity* entity = clbk(this);

	return entity;
}

void HgScene::RegisterEntityFactory(const char* str, Engine::factoryCallback clbk)
{
	m_entityFactories[str] = clbk;
}

void HgScene::update(HgTime dtime)
{
	for (auto& i : m_collections) {
		i->update(dtime);
	}
}

void HgScene::EnqueueForRender(RenderQueue* queue) {
	for (auto& i : m_collections) {
		i->EnqueueForRender(queue);
	}
}

} //Engine