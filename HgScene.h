#pragma once

#include <HgElement.h>

typedef struct HgScene {
	HgElement* elements;
	uint8_t* used;
	uint32_t _size;
	uint32_t _next_empty;
} HgScene;

typedef struct HgScene_iterator {
	uint32_t _current;
	HgScene* s;
} HgScene_iterator;

//void scene_resize(HgScene* scene);
void scene_init(HgScene* scene, uint32_t size);
//void scene_add_element(HgScene* scene, HgElement* element);


/*	Do not store the pointer for long periods of time. Creating new elements can cause a resize, invalidating pointers.
	Store the returned index as an alternative. */
uint32_t scene_newElement(HgScene* scene, HgElement** element);

void scene_clearUpdate(HgScene* scene);

/*	Returns greater than 0 if the index has a valid element */
uint8_t is_used(HgScene* s, uint32_t index);

inline void scene_init_iterator(HgScene_iterator* i, HgScene* scene) { i->_current = 0; i->s = scene; }
HgElement* scene_next_element(HgScene_iterator* i);

void scene_delete_element_itr(HgScene_iterator* i);
void scene_delete_element(HgScene* scene, uint32_t index);

#define IS_USED(scene,index) is_used(scene,index)