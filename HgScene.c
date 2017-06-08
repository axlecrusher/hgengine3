#include <stdlib.h>
#include <assert.h>

#include <Windows.h>

#include <HgScene.h>

//Must be a multiple of 8
#define CHUNK_SIZE		128

void scene_resize(HgScene* scene, uint32_t size) {
//	uint32_t size = scene->_size + 1000;
	HgElement* e = scene->elements;

	e = realloc(e, size * sizeof *e);
	assert(e != NULL);

	uint32_t i;
	for (i = scene->_size; i < size; ++i) {
		init_hgelement(e + i);
	}

//	uint32_t usize = (size / 8)+1;
	uint32_t usize = size/8;
	uint8_t* u = scene->used;
	u = realloc(u, usize * sizeof *u);
	assert(u != NULL);

//	for (i = (scene->_size/8)+(scene->_size%8>0); i < usize; ++i) {
	for (i = scene->_size/8; i < usize; ++i) {
			u[i] = 0;
	}

	scene->_size = size;
	scene->elements = e;
	scene->used = u;
}

void scene_init(HgScene* scene) {
	scene->elements = NULL;
	scene->used = NULL;
	scene->_size = 0;
	scene->_next_empty = 0;
	scene_resize(scene, CHUNK_SIZE);
}

static void set_used(HgScene* s, uint32_t idx) {
	uint32_t q = idx / 8;
	uint32_t r = idx % 8;
	s->used[q] |= (1 << r);
}

static void clear_used(HgScene* s, uint32_t idx) {
	uint32_t q = idx / 8;
	uint32_t r = idx % 8;
	s->used[q] &= ~(1 << r);
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
uint32_t scene_newElement(HgScene* scene,HgElement** element) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		if (IS_USED(scene, i) == 0) {
			init_hgelement(scene->elements + i);
			set_used(scene,i);
//			scene->used[i] = 1;
			*element = scene->elements + i;
			scene->size_used++;
			return i;
		}
	}

	//resize
	scene_resize(scene, scene->_size + CHUNK_SIZE);
	return scene_newElement(scene, element);
}

void scene_clearUpdate(HgScene* scene) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		CLEAR_FLAG(scene->elements + i, HGE_UPDATED);
	}
}

/* using this iterator is just as fast as manually using a for loop */
HgElement* scene_next_element(HgScene_iterator* itr) {
	uint32_t x;
	for (x = itr->_current; x < itr->s->_size; ++x) {
		if (IS_USED(itr->s, HGE_USED) > 0) {
			itr->_current=x;
			return itr->s->elements + x;
		}
	}
	return NULL;
}

void scene_delete_element_itr(HgScene_iterator* i) {
	HgElement* e = i->s->elements + i->_current;
	VCALL_IDX(e, destroy);
	init_hgelement(e);
	clear_used(i->s,i->_current);
//	i->s->used[i->_current] = 0;
}

void scene_delete_element(HgScene* scene, uint32_t idx) {
//	printf("do destroy\n");
	if (IS_USED(scene, idx) > 0) {
		HgElement* e = scene->elements + idx;
		VCALL_IDX(e, destroy);
		init_hgelement(e);
		//	scene->used[idx] = 0;
		clear_used(scene, idx);
		scene->size_used--;
	}
}

uint8_t is_used(HgScene* s, uint32_t index)
{
	uint32_t q = index / 8;
	uint32_t r = index % 8;
	return s->used[q] & (1 << r);
}