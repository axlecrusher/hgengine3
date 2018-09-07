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

std::map<std::string, factory_clbk> element_factories;

void RegisterElementType(const char* c, factory_clbk factory) {
	element_factories[c] = factory;
}


void HgElement::init()
{
	m_renderData = NULL;
	m_logic = nullptr;
	m_renderData = nullptr;

	m_extendedData = std::make_unique<HgElementExtended>();

	m_parent = nullptr;
	m_updateNumber = 0;
}

HgElement::~HgElement() {
	destroy();
}

void HgElement::destroy()
{
	m_logic.reset();
	m_renderData.reset();
}

HgMath::mat4f HgElement::computeWorldSpaceMatrix(bool applyScale, bool applyRotation, bool applyTranslation) const {
	//translate to origin, scale, rotate, apply local translation, apply parent transforms
	HgMath::mat4f modelMatrix;
	const auto origin_vec = -vectorial::vec3f(origin().raw());
	//modelMatrix = HgMath::mat4f::translation(origin_vec);

	const float scaleFactor = (scale()*applyScale) + (!applyScale * 1.0f); //Integral promotion of bool, avoid branching

	//const auto correct = HgMath::mat4f::scale(scaleFactor) * modelMatrix;
	//I think this is the same result with less math
	modelMatrix = HgMath::mat4f::scale(scaleFactor);
	modelMatrix.value.w = (origin_vec * scaleFactor).xyz1().value;

	if (applyRotation) {
		modelMatrix = orientation().toMatrix4() * modelMatrix;
	}

	const float translationScalar = applyTranslation * 1.0f; //Integral promotion of bool, avoid branching
	//if (applyTranslation) {
		//auto correct = HgMath::mat4f::translation(vectorial::vec3f(position().raw())) * modelMatrix;
		//I think this is the same result with less math
		const auto tmp = vectorial::vec4f(modelMatrix.value.w)
			+ vectorial::vec3f(position().raw()).xyz0() * translationScalar;
		modelMatrix.value.w = tmp.value;
	//}

	if (m_parent) {
		modelMatrix = m_parent->computeWorldSpaceMatrix(flags.inheritParentScale,
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
