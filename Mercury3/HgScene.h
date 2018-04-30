#pragma once

#include <memory>
#include <vector>

#include <HgElement.h>

#include <HgTimer.h>

/* HgScene could be reworked into a linked list (or array of pointers to list items) with each
list item containing 512 elements and a used map. We could still index into the linked list
by using the first 9 least significant bits to represent 512 elements. The last 7 MSB could
index into the array of list items. This would allow both indexing and valid HgElement
pointers for the duration of program execution.
*/

class SceneChunk {
	public:
		typedef uint32_t usedType;

		SceneChunk();
		bool isUsed(uint32_t i);
		void set_used(uint32_t i);
		void clear_used(uint32_t idx);

		HgElement elements[512];
	private:
		usedType used[512 / (sizeof(usedType)*8)];
};

class HgScene {
	public:
		HgScene();
		void init();

		uint32_t getNewElement(HgElement** element);
		void removeElement(uint32_t i);

		inline bool isUsed(uint32_t idx) {
			uint32_t a = (idx >> 9) & 0x7F;
			uint32_t b = idx & 0x1FF;
			return chunks[a]->isUsed(b);
		}
		inline HgElement* get_element(uint32_t index) { return &chunks[(index >> 9) & 0x7F]->elements[index & 0x1FF]; }

		inline uint32_t usedCount() const { return used_count; }
		inline uint32_t chunkCount() const { return (uint32_t)chunks.size(); }
		inline uint32_t maxItems() const { return (uint32_t)(chunks.size() * 512); }

		inline uint32_t nextUpdateNumber() { return ++m_updateCount; }

		void update(HgTime dtime);

//		void(*submit_for_render)(HgElement* e);
private:
		void allocate_chunk();
		std::vector< std::unique_ptr<SceneChunk> > chunks;
		uint32_t used_count;

		uint32_t m_updateCount;
};


uint8_t create_element(char* type, HgScene* scene, HgElement** element);

//class HgScene;

extern std::map<std::string, factory_clbk> element_factories;

template<typename T>
T* create_element(char* type, HgScene* scene, HgElement** element) {
	//	uint32_t idx = hgelement_get_type_index(type);

	auto factory = element_factories.find(type);

	if (factory == element_factories.end()) {
		fprintf(stderr, "Unable to find element type \"%s\"\n", type);
		return 0;
	}
	factory_clbk clbk = factory->second;
	scene->getNewElement(element);
	return (T*)clbk(*element);
}

/*
typedef struct HgScene_iterator {
	uint32_t _current;
	HgScene* s;
} HgScene_iterator;

*/