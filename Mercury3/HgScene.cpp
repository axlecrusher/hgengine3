#include <stdlib.h>
#include <assert.h>

#include <Windows.h>

#include <HgScene.h>

#include <stdio.h>

#include <RenderBackend.h>

//Must be a multiple of 8
//#define CHUNK_SIZE		128

//static uint16_t SceneChunk::CHUNK_SIZE = 512;
SceneChunk::SceneChunk() {
	memset(used, 0, sizeof(used));
}

bool SceneChunk::isUsed(uint32_t i)
{
	uint32_t r = i % (sizeof(uint32_t) * 8);
	uint32_t q = i / (sizeof(usedType) * 8);
//	uint32_t q = i >> 5;
	return 0<(used[q] & (1 << r));
}

void SceneChunk::set_used(uint32_t i) {
	uint32_t q = i / (sizeof(usedType) * 8);
	uint32_t r = i % (sizeof(usedType) * 8);
	used[q] |= (1 << r);
}

void SceneChunk::clear_used(uint32_t idx) {
	uint32_t q = idx / (sizeof(usedType) * 8);
	uint32_t r = idx % (sizeof(usedType) * 8);
	used[q] &= ~(1 << r);
}

static void decode_index(uint32_t index, uint16_t* h, uint16_t* l) { *l = index & 0x1FF; *h = (index >> 9) & 0x7F; }

HgScene::HgScene()
	:used_count(0), m_updateCount(0)
{

}

void HgScene::init()
{
	allocate_chunk();
}

uint32_t HgScene::getNewEntity(HgEntity** entity) {
	uint16_t h, l;
	for (uint32_t i = 0;; ++i) {
		decode_index(i, &h, &l);
		if (h >= chunks.size()) break;
		if (!chunks[h]->isUsed(l)) {
			SceneChunk* chunk = chunks[h].get();
			HgEntity* e = &chunk->entites[l];
			chunk->set_used(l);
			e->init();
			used_count++;
			//e->setScene(this);
			*entity = e;
			return i;
		}
	}

	allocate_chunk();
	return getNewEntity(entity);
}

void HgScene::removeEntity(uint32_t i) {
	//	printf("do destroy\n");
	uint16_t h, l;
	decode_index(i, &h, &l);
	if (chunks[h]->isUsed(l)) {
		HgEntity* e = get_entity(i);
//		printf("Destroying element of type: %s\n", HgEntity_get_type_str(e));
		e->destroy();
		chunks[h]->clear_used(l);
		used_count--;
	}
}


void HgScene::allocate_chunk() {
	chunks.push_back(std::make_unique<SceneChunk>());
}

void HgScene::update(HgTime dtime) {
	uint32_t updateNumber = nextUpdateNumber();
	uint32_t maxCount = maxItems();
	for (uint32_t i = 0; i < maxCount; ++i) {
		if (!isUsed(i)) continue;
		HgEntity* e = get_entity(i);

		if ((dtime.msec() > 0) && e->needsUpdate(updateNumber)) {
			e->update(dtime, updateNumber);
		}

		/* FIXME: WARNING!!! if this loop is running async to the render thread, element deletion can cause a crash!*/
		//shared_ptr my way out of this?
		if (e->flags.destroy) {
			removeEntity(i);
			continue;
		}
	}
}



void HgScene::EnqueueForRender() {
	uint32_t updateNumber = nextUpdateNumber();
	uint32_t maxCount = maxItems();
	for (uint32_t i = 0; i < maxCount; ++i) {
		if (!isUsed(i)) continue;
		HgEntity* e = get_entity(i);

		//I would like to move this out of here for a more perminent, solution
		//where the vectors don't need to be rebuild every time
		if (!e->flags.hidden) { 
			Renderer::Enqueue(*e);
		}

	}
}

uint8_t create_entity(char* type, HgScene* scene, HgEntity** entity) {
//	uint32_t idx = HgEntity_get_type_index(type);

	auto factory = entity_factories.find(type);

	if (factory == entity_factories.end()) {
		fprintf(stderr, "Unable to find element type \"%s\"\n", type);
		return 0;
	}
	factory_clbk clbk = factory->second;
	scene->getNewEntity(entity);
	clbk(*entity);

	//find element type creator
//	(*element)->vptr_idx = idx;
//	VCALL_IDX((*element), create);

	return 1;
}
/*
void scene_clearUpdate(HgScene* scene) {
	uint32_t i = 0;
	for (i = 0; i < scene->_size; ++i) {
		HgEntity* e = get_element(scene, i);
		CLEAR_FLAG(e, HGE_UPDATED);
	}
}
*/
/* using this iterator is just as fast as manually using a for loop */
/*
HgEntity* scene_next_element(HgScene_iterator* itr) {
	uint32_t x;
	for (x = itr->_current; x < itr->s->_size; ++x) {
		if (IS_USED(itr->s, HGE_USED) > 0) {
			itr->_current=x;
			return &itr->s->elements[x];
		}
	}
	return NULL;
}
*/
