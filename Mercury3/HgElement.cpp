#include <HgElement.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <str_utils.h>
#include <HgMath.h>
#include <map>

#include <HgTimer.h>

#include <UpdatableCollection.h>

RenderData* (*new_RenderData)() = NULL;

std::map<std::string, factory_clbk> element_factories;

void RegisterElementType(const char* c, factory_clbk factory) {
	element_factories[c] = factory;
}

RenderData::newRenderDataCallback RenderData::Create = nullptr;

RenderData::RenderData()
	:blendMode(BlendMode::BLEND_NORMAL), renderFunction(nullptr),
	vbo_offset(0), vertex_count(0), index_count(0),
	renderFlags(RenderFlags(FACE_CULLING | DEPTH_WRITE))
{

}

RenderData::~RenderData()
{
	destroy();
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

	rotation = quaternion::IDENTITY;
	m_extendedData = std::make_unique<HgElementExtended>();
	m_extendedData->owner = this;

	m_parent = nullptr;
	m_updateNumber = 0;
}

HgElement::~HgElement() {
	destroy();
}

void HgElement::destroy()
{
	m_logic.reset();
//	if (m_logic) delete(m_logic);
//	m_logic = nullptr;
	if (m_extendedData && m_extendedData->m_ownRenderData) {
		if (m_renderData != nullptr) delete m_renderData;
	}
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

	void updateCollections(HgTime dtime) {
		auto c = collections();
		for (auto i : c) {
			i->update(dtime);
		}
	}

	void EnqueueForRender(std::vector<IUpdatableCollection*>& c) {
		for (auto i : c) {
			i->EnqueueForRender();
		}
	}
}
