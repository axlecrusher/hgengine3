#include <HgElement.h>

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

