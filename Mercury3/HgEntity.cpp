#include <HgEntity.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <str_utils.h>
#include <HgMath.h>
#include <unordered_map>

#include <HgTimer.h>

#include <UpdatableCollection.h>

#include <EventSystem.h>

std::unordered_map<std::string, factory_clbk> entity_factories;

EntityIdType HgEntity::m_nextEntityId(1);

void RegisterEntityType(const char* c, factory_clbk factory) {
	entity_factories[c] = factory;
}


void EntityLocator::RegisterEntity(HgEntity* entity)
{
	auto id = entity->getEntityId();
	if (id == 0) return;

	entities[id] = entity;
}

void EntityLocator::RemoveEntity(HgEntity* entity)
{
	auto id = entity->getEntityId();
	if (id == 0) return;

	auto itr = entities.find(id);
	if (itr != entities.end())
	{
		entities.erase(itr);
	}
}

HgEntity* EntityLocator::Find(EntityIdType id) const
{
	HgEntity* entity = nullptr;
	if (id.isValid())
	{
		auto itr = entities.find(id);
		if (itr != entities.end())
		{
			entity = itr->second;
		}
	}
	return entity;
}

EntityLocator& HgEntity::Locator()
{
	static EntityLocator locator;
	return locator;
}

HgEntity* HgEntity::Find(EntityIdType id)
{
	return Locator().Find(id);
}


void HgEntity::init()
{
	if (getEntityId().isValid())
	{
		destroy();
	}

	m_renderData = nullptr;
	m_logic = nullptr;
	m_renderData = nullptr;

	m_parentId = EntityIdType();
	m_updateNumber = 0;

	m_entityId = m_nextEntityId++;

	Locator().RegisterEntity(this);
	EventSystem::PublishEvent(EntityCreated(this));
}

HgEntity::~HgEntity() {
	destroy();
}

void HgEntity::destroy()
{
	if (m_entityId.isValid())
	{
		EventSystem::PublishEvent(EntityDestroyed(this, m_entityId));
	}
	Locator().RemoveEntity(this);
	m_logic.reset();
	m_renderData.reset();
	m_entityId = EntityIdType(); //reset id
}

HgMath::mat4f HgEntity::computeWorldSpaceMatrix(bool applyScale, bool applyRotation, bool applyTranslation) const {
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

	auto parent = getParent();
	if (parent) {
		modelMatrix = parent->computeWorldSpaceMatrix(flags.inheritParentScale,
			flags.inheritParentRotation, flags.inheritParentTranslation) * modelMatrix;
	}

	return modelMatrix;
}

point HgEntity::computeWorldSpacePosition() const
{
	const vector3f p;
	const auto matrix = computeWorldSpaceMatrix();
	return matrix * p;
}

//Transform point p into world space of HgEntity e
//I'm not 100% sure this matches the functionality of computeWorldSpaceMatrix so remove for now
//point toWorldSpace(const HgEntity* e, const point* p) {
//	vector3 v1 = (*p - e->origin()).scale(e->scale()).rotate(e->orientation()) + e->position();
//	return v1;
//}

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


REGISTER_EVENT_TYPE(EntityCreated)
REGISTER_EVENT_TYPE(EntityDestroyed)