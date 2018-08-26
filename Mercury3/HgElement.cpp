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


void HgElement::init()
{
	m_renderData = NULL;
	m_position = vector3();
	m_logic = nullptr;
	m_renderData = nullptr;
	m_scale = 1;
	m_origin = vector3();

	m_orientation = quaternion::IDENTITY;
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

HgMath::mat4f HgElement::getWorldSpaceMatrix(bool applyScale, bool applyRotation, bool applyTranslation) const {
	HgMath::mat4f modelMatrix;
	const vectorial::vec3f origin(m_origin.raw());
	modelMatrix = HgMath::mat4f::translation(-origin);

	if (applyScale) {
		modelMatrix = HgMath::mat4f::scale(scale()) * modelMatrix;
	}

	if (applyRotation) {
		modelMatrix = m_orientation.toMatrix4() * modelMatrix;
	}

	if (applyTranslation) {
		const auto tmp = vectorial::vec3f(m_position.raw());// +origin;
		modelMatrix = HgMath::mat4f::translation(tmp) * modelMatrix;
	}

	if (m_parent) {
		modelMatrix = m_parent->getWorldSpaceMatrix(flags.inheritParentScale,
			flags.inheritParentRotation, flags.inheritParentTranslation) * modelMatrix;
	}

	return modelMatrix;
}



//Transform point p into world space of HgElement e
point toWorldSpace(const HgElement* e, const point* p) {
	vector3 v1 = (*p - e->origin()).scale(e->scale()).rotate(e->orientation()) + e->position();
	return v1;
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
