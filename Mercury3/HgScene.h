#pragma once

#include <memory>
#include <vector>

#include <HgEngine.h>
#include <HgEntity.h>

#include <HgTimer.h>

class RenderQueue;

/* HgScene could be reworked into a linked list (or array of pointers to list items) with each
list item containing 512 entities and a used map. We could still index into the linked list
by using the first 9 least significant bits to represent 512 entities. The last 7 MSB could
index into the array of list items. This would allow both indexing and valid HgEntity
pointers for the duration of program execution.
*/

class SceneChunk {
	public:
		typedef uint32_t usedType;

		SceneChunk();
		bool isUsed(uint32_t i);
		void set_used(uint32_t i);
		void clear_used(uint32_t idx);

		HgEntity entites[512];
	private:
		usedType used[512 / (sizeof(usedType)*8)];
};

class HgScene {
	public:
		HgScene();
		void init();

		uint32_t getNewEntity(HgEntity** entity);
		void removeEntity(uint32_t i);

		inline bool isUsed(uint32_t idx) {
			uint32_t a = (idx >> 9) & 0x7F;
			uint32_t b = idx & 0x1FF;
			return chunks[a]->isUsed(b);
		}
		inline HgEntity* get_entity(uint32_t index) { return &chunks[(index >> 9) & 0x7F]->entites[index & 0x1FF]; }

		inline uint32_t usedCount() const { return used_count; }
		inline uint32_t chunkCount() const { return (uint32_t)chunks.size(); }
		inline uint32_t maxItems() const { return (uint32_t)(chunks.size() * 512); }

		inline uint32_t nextUpdateNumber() { return ++m_updateCount; }

		void update(HgTime dtime);
		void EnqueueForRender(RenderQueue* queue, HgTime dt);

//		void(*submit_for_render)(HgEntity* e);
private:
		void allocate_chunk();
		std::vector< std::unique_ptr<SceneChunk> > chunks;
		uint32_t used_count;

		uint32_t m_updateCount;
};


template<typename T>
T* create_entity(char* type, HgScene* scene, HgEntity** entity) {
	//	uint32_t idx = HgEntity_get_type_index(type);

	auto factory = Engine::entity_factories.find(type);

	if (factory == Engine::entity_factories.end()) {
		fprintf(stderr, "Unable to find entity type \"%s\"\n", type);
		return 0;
	}
	factory_clbk clbk = factory->second;
	scene->getNewEntity(entity);
	return (T*)clbk(*entity);
}


template<typename T>
T* create_entity(char* type, HgScene* scene) {
	//	uint32_t idx = HgEntity_get_type_index(type);

	auto factory = Engine::entity_factories.find(type);

	if (factory == Engine::entity_factories.end()) {
		fprintf(stderr, "Unable to find entity type \"%s\"\n", type);
		return 0;
	}
	factory_clbk clbk = factory->second;
	return (T*)clbk(nullptr);
}

/*
typedef struct HgScene_iterator {
	uint32_t _current;
	HgScene* s;
} HgScene_iterator;

*/