#include <HgElement.h>

void init_hgelement(HgElement* element) {
	element->flags = 0;
//	element->index = 0;
	element->updateFunc = NULL;
//	element->renderFunc = NULL;
	element->m_renderData = NULL;
	element->position[0] = 0;
	element->position[1] = 0;
	element->position[2] = 0;

	quaternion_init(&element->rotation);
}

