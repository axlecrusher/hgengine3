#include <HgElement.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <str_utils.h>
#include <HgMath.h>
#include <map>
/*
hgstring HGELEMENT_TYPE_NAMES = { 0, 0 };
uint32_t HGELEMENT_TYPE_NAME_OFFSETS[MAX_ELEMENT_TYPES] = { 0 };

extern HgElement_vtable HGELEMT_VTABLES[MAX_ELEMENT_TYPES] = { 0 };
*/

/*
hgstring HGELEMENT_TYPE_NAMES = { 0, 0 };
uint32_t HGELEMENT_TYPE_NAME_OFFSETS[MAX_ELEMENT_TYPES] = { 0 };
*/
RenderData* (*new_RenderData)() = NULL;
/*
vtable_index RegisterElementType(const char* c) {
	static vtable_index ElementTypeCounter = 1; //0 is reserved for undefined
	if (ElementTypeCounter==1) {
		HGELEMENT_TYPE_NAME_OFFSETS[0] = hgstring_append(&HGELEMENT_TYPE_NAMES, "UndefinedType");
	}

	printf("Registering %s, type %d \n", c, ElementTypeCounter);
	HGELEMENT_TYPE_NAME_OFFSETS[ElementTypeCounter] = hgstring_append(&HGELEMENT_TYPE_NAMES, c);
	return ElementTypeCounter++;
}
*/

std::map<std::string, factory_clbk> element_factories;
/*
vtable_index RegisterElementType(const char* c) {
	static vtable_index ElementTypeCounter = 1; //0 is reserved for undefined
	if (ElementTypeCounter == 1) {
		HGELEMENT_TYPE_NAME_OFFSETS[0] = hgstring_append(&HGELEMENT_TYPE_NAMES, "UndefinedType");
	}

	printf("Registering %s, type %d \n", c, ElementTypeCounter);
	HGELEMENT_TYPE_NAME_OFFSETS[ElementTypeCounter] = hgstring_append(&HGELEMENT_TYPE_NAMES, c);
	return ElementTypeCounter++;
}
*/
void RegisterElementType(const char* c, factory_clbk factory) {
	element_factories[c] = factory;
}

RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:blendMode(BlendMode::BLEND_NORMAL)
{

}

RenderData::~RenderData()
{
	destroy();
}

void RenderData::destroy() {
	if (shader) HgShader::release(shader);
	shader = nullptr;
}

void HgElement::init()
{
	flags = 0;
	m_renderData = NULL;
	position = vector3_zero;
	m_logic = nullptr;
	m_renderData = nullptr;
	scale = 1;
	origin = vector3_zero;

	quaternion_init(&rotation);
}

void HgElement::destroy()
{
}

void HgElement::render() {

}


/*
vtable_index hgelement_get_type_index(char* type) {
	for (vtable_index i = 0; i < MAX_ELEMENT_TYPES; ++i) {
		char* str = HGELEMENT_TYPE_NAMES.str + HGELEMENT_TYPE_NAME_OFFSETS[i];
		if (strcmp(type, str) == 0) return i;
	}

	return 0;
}
*/
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

