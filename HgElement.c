#include <HgElement.h>
#include <stdlib.h>

void init_hgelement(HgElement* element) {
	element->flags = 0;
//	element->index = 0;
	element->vptr = NULL;
//	element->renderFunc = NULL;
	element->m_renderData = NULL;
	element->position.components.x = 0;
	element->position.components.y = 0;
	element->position.components.z = 0;

	quaternion_init(&element->rotation);
}

void HgElement_destroy(HgElement* element) {
	VCALL(element, destroy);
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