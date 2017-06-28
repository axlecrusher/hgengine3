#pragma once

#include <HgElement.h>

/* HgScene could be reworked into a linked list (or array of pointers to list items) with each
list item containing 512 elements and a used map. We could still index into the linked list
by using the first 9 least significant bits to represent 512 elements. The last 7 MSB could
index into the array of list items. This would allow both indexing and valid HgElement
pointers for the duration of program execution.
*/

#define SCENE_CHUNK_SIZE		512

typedef struct SceneChunk {
	HgElement elements[SCENE_CHUNK_SIZE];
	uint8_t used[SCENE_CHUNK_SIZE / 8];
} SceneChunk;

typedef struct HgScene {
	SceneChunk* chunks[128];
	uint16_t chunk_count;
	/*
	HgElement* elements;
	uint8_t* used;
	uint32_t _size;
	uint32_t _next_empty;

	*/
	uint32_t _size;
	uint32_t size_used;
} HgScene;

typedef struct HgScene_iterator {
	uint32_t _current;
	HgScene* s;
} HgScene_iterator;

//void scene_resize(HgScene* scene);
void scene_init(HgScene* scene);
//void scene_add_element(HgScene* scene, HgElement* element);



/*	Do not store the pointer for long periods of time. Creating new elements can cause a resize, invalidating pointers.
	Store the returned index as an alternative. */
uint32_t scene_newElement(HgScene* scene, HgElement** element);

void scene_clearUpdate(HgScene* scene);

/*	Returns greater than 0 if the index has a valid element */
uint8_t is_used(HgScene* s, uint32_t index);

inline HgElement* get_element(HgScene* s, uint32_t index) { return &s->chunks[(index >> 9) & 0x7F]->elements[index & 0x1FF]; }
//#define get_element(s,index) (&(s)->chunks[(index >> 9) & 0x7F]->elements[index & 0x1FF])

inline void scene_init_iterator(HgScene_iterator* i, HgScene* scene) { i->_current = 0; i->s = scene; }
HgElement* scene_next_element(HgScene_iterator* i);

void scene_delete_element(HgScene* scene, uint32_t index);
inline void scene_delete_element_itr(HgScene_iterator* i) { scene_delete_element(i->s, i->_current); }

#define IS_USED(scene,index) is_used(scene,index)