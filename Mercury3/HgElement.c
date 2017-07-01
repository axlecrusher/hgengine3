#include <HgElement.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#define MAX_ELEMENT_TYPE_LEN 16

extern char HGELEMT_TYPE_NAMES[255][MAX_ELEMENT_TYPE_LEN] = { 0 };
extern HgElement_vtable HGELEMT_VTABLES[255] = { 0 };

void* (*new_RenderData)() = NULL;

vtable_index RegisterElementType(const char* c) {
	static vtable_index ElementTypeCounter = 0;
	printf("Registering %s, type %d \n", c, ElementTypeCounter);
	strncpy(HGELEMT_TYPE_NAMES[ElementTypeCounter], c, MAX_ELEMENT_TYPE_LEN);
	return ElementTypeCounter++;
}

void init_hgelement(HgElement* element) {
	element->flags = 0;
	element->vptr_idx = 0;
	element->m_renderData = NULL;
	element->position.components.x = 0;
	element->position.components.y = 0;
	element->position.components.z = 0;
	element->extraData = NULL;
	element->m_renderData = NULL;
	element->scale = 1;

	quaternion_init(&element->rotation);
}

void create_element(char* type, HgElement* e) {
	for (uint16_t i = 0; i < 255; ++i) {
		if (strncmp(type, HGELEMT_TYPE_NAMES[i], MAX_ELEMENT_TYPE_LEN) == 0) {
			e->vptr_idx = i;
			VCALL_IDX(e, create);
			return;
		}
	}
	fprintf(stderr, "Unable to find element type \"%s\"\n", type);
	assert(type==0);
}

/*
void HgElement_destroy(HgElement* e) {
	if (e->m_renderData && e->m_renderData->destroy) e->m_renderData->destroy(e->m_renderData);
	free(e->m_renderData);
	e->m_renderData = NULL;
}
*/
/*
void HgElement_destroy(HgElement* element) {
	VCALL_IDX(element, destroy);
	if (element->m_renderData) {
		if (element->m_renderData->shader) {
			HGShader_release(element->m_renderData->shader);
			element->m_renderData->shader = NULL;
		}
		if (element->m_renderData->destroy) element->m_renderData->destroy(element->m_renderData);

		free(element->m_renderData);
		element->m_renderData = NULL;
	}
}
*/

