#pragma once

#include <HgElement.h>

typedef struct HgScene {
	HgElement* elements;
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
HgElement* scene_newElement(HgScene* scene);
void scene_clearUpdate(HgScene* scene);

inline void scene_init_iterator(HgScene_iterator* i, HgScene* scene) { i->_current = 0; i->s = scene; }
HgElement* scene_next_element(HgScene_iterator* i);

void scene_delete_element_itr(HgScene_iterator* i);
void scene_delete_element(HgScene* scene, uint32_t index);