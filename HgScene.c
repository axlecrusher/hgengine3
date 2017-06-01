#include <stdlib.h>
#include <assert.h>

#include <Windows.h>

#include <HgScene.h>

void scene_resize(HgScene* scene, uint32_t size) {
//	uint32_t size = scene->_size + 1000;
	HgElement* e = scene->elements;
	e = realloc(e, size * sizeof *e);
	assert(e != NULL);

	uint32_t i;
	for (i = scene->_size; i < size; ++i) {
		init_hgelement(e + i);
	}
	scene->_size = size;
	scene->elements = e;
}

void scene_init(HgScene* scene, uint32_t size) {
	scene->elements = NULL;
	scene->_size = 0;
	scene->_next_empty = 0;
	scene_resize(scene, size);
}
/*
void scene_add_element(HgScene* scene, HgElement* element) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		if (check_flag(&scene->elements[i], HGE_USED) == 0) {
			scene->elements[i] = *element;
			return;
		}
	}

	//resize
	scene_resize(scene, scene->_size+1000);
	scene_add_element(scene, element);
}
*/
HgElement* scene_newElement(HgScene* scene) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		HgElement* e = scene->elements + i;
		if (CHECK_FLAG(e, HGE_USED) == 0) {
			quaternion_init(&e->rotation);
			set_flag(e, HGE_USED);
			return e;
		}
	}

	//resize
	scene_resize(scene, scene->_size + 1000);
	return scene_newElement(scene);
}

void scene_clearUpdate(HgScene* scene) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		clear_flag(scene->elements + i, HGE_UPDATED);
	}
}

/* using this iterator is just as fast as manually using a for loop */
HgElement* scene_next_element(HgScene_iterator* itr) {
	uint32_t x;
	for (x = itr->_current; x < itr->s->_size; ++x) {
		if (CHECK_FLAG((itr->s->elements + x), HGE_USED) > 0) {
			++itr->_current;
			return itr->s->elements + x;
		}
	}
	return NULL;
}

void scene_delete_element(HgScene_iterator* i) {
	//
}
