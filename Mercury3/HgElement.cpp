#include <HgElement.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <str_utils.h>
#include <HgMath.h>
#include <map>

#include <UpdatableCollection.h>

RenderData* (*new_RenderData)() = NULL;

std::map<std::string, factory_clbk> element_factories;

void RegisterElementType(const char* c, factory_clbk factory) {
	element_factories[c] = factory;
}

RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:blendMode(BlendMode::BLEND_NORMAL), renderFunction(nullptr),
	hgVbo(nullptr), indexVbo(nullptr), colorVbo(nullptr),
	vbo_offset(0), vertex_count(0), index_count(0),
	renderFlags(RenderFlags(FACE_CULLING | DEPTH_WRITE))
{

}

RenderData::~RenderData()
{
}

void RenderData::destroy() {
	//FIXME: Do something to clean up hgVbo
	//hgvbo_remove(d->hgvbo, d->vbo_offset, d->vertex_count)

	if (shader) HgShader::release(shader);
	shader = nullptr;
}

void RenderData::init() {
	shader = HgShader::acquire("test_vertex.glsl", "test_frag.glsl");
}


void HgElement::init()
{
	flags = 0;
	m_renderData = NULL;
	position = vector3();
	m_logic = nullptr;
	m_renderData = nullptr;
	scale = 1;
	origin = vector3();

	rotation = quaternion_default;
	m_extendedData = std::make_unique<HgElementExtended>();
	m_extendedData->owner = this;

	m_parent = nullptr;
	m_updateNumber = 0;
}

void HgElement::destroy()
{
	m_logic.reset();
//	if (m_logic) delete(m_logic);
//	m_logic = nullptr;
	if (m_extendedData->m_ownRenderData) delete m_renderData;
	m_renderData = nullptr; //need some way to signal release
}

void HgElement::updateGpuTextures() {
	m_renderData->clearTextureIDs();

	for (auto itr = m_extendedData->textures.begin(); itr != m_extendedData->textures.end(); itr++) {
		auto texture = *itr;
		if (texture->NeedsGPUUpdate()) {
			texture->sendToGPU();
			HgTexture::TextureType type = texture->getType();
		}

		//FIXME: Share texture pointers can cause a problem here. Texture may be updated by another HgElement.
		//refactor into HgTexture making callback into renderData to set texture IDs.
		//NOTE: Not sure this applies anymore. HgTexture is pretty immutable after creation.
		m_renderData->setTexture(texture.get()); 
	}
}

namespace Engine {
	std::vector<IUpdatableCollection*>& collections() {
		static std::vector<IUpdatableCollection*> c;
		return c;
	}

	void updateCollections(uint32_t dtime) {
		auto c = collections();
		for (auto i : c) {
			i->update(dtime);
		}
	}
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

