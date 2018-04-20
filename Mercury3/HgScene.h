#pragma once

#include <memory>
#include <vector>

#include <HgElement.h>

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
		bool isUsed(uint16_t i);
		void set_used(uint16_t i);
		void clear_used(uint16_t idx);

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

		bool isUsed(uint32_t idx);
		inline HgElement* get_element(uint32_t index) { return &chunks[(index >> 9) & 0x7F]->elements[index & 0x1FF]; }

		inline uint32_t usedCount() const { return used_count; }
		inline uint32_t chunkCount() const { return chunks.size(); }
		inline uint32_t maxItems() const { return chunks.size() * 512; }

		inline uint32_t nextUpdateNumber() { return ++m_updateCount; }

		void update(uint32_t dtime);

//		void(*submit_for_render)(HgElement* e);
private:
		void allocate_chunk();
		std::vector< std::unique_ptr<SceneChunk> > chunks;
		uint32_t used_count;

		uint32_t m_updateCount;
};


uint8_t create_element(char* type, HgScene* scene, HgElement** element);

/*
typedef struct HgScene_iterator {
	uint32_t _current;
	HgScene* s;
} HgScene_iterator;

*/