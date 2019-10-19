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
#include <HgEngine.h>

EntityIdType HgEntity::m_nextEntityId(1);

void RegisterEntityType(const char* c, factory_clbk factory) {
	Engine::entity_factories[c] = factory;
}

void EntityLocator::RegisterEntity(HgEntity* entity)
{
	std::lock_guard<std::mutex> m_lock(m_mutex);

	const auto id = entity->getEntityId();
	if (!id.isValid()) return;

	m_entities[id] = entity;
}

void EntityLocator::RemoveEntity(EntityIdType id)
{
	if (!id.isValid()) return;

	std::lock_guard<std::mutex> m_lock(m_mutex);

	auto itr = m_entities.find(id);
	if (itr != m_entities.end())
	{
		m_entities.erase(itr);
	}
}

EntityLocator::SearchResult EntityLocator::Find(EntityIdType id) const
{
	SearchResult result;
	if (id.isValid())
	{
		std::lock_guard<std::mutex> m_lock(m_mutex);

		auto itr = m_entities.find(id);
		if (itr != m_entities.end())
		{
			result.entity = itr->second;
		}
	}
	return result;
}

EntityLocator& HgEntity::Locator()
{
	static EntityLocator locator;
	return locator;
}

EntityLocator::SearchResult HgEntity::Find(EntityIdType id)
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

	m_extendedData = std::make_unique<ExtendedEntityData>();

	auto tmp = EntityIdType();
	m_parentId = tmp;
	m_updateNumber = 0;

	m_drawOrder = 0;

	m_entityId = m_nextEntityId++;

	Locator().RegisterEntity(this);
	EventSystem::PublishEvent(EntityCreated(this, m_entityId));
}

HgEntity::~HgEntity() {
	destroy();
}

HgEntity::HgEntity(HgEntity &&rhs)
{
	m_spacialData = std::move(rhs.m_spacialData);
	m_renderData = std::move(rhs.m_renderData);
	m_logic = std::move(rhs.m_logic);
	m_extendedData = std::move(rhs.m_extendedData);
	m_parentId = std::move(rhs.m_parentId);
	m_updateNumber = std::move(rhs.m_updateNumber);
	m_drawOrder = std::move(rhs.m_drawOrder);
	flags = std::move(rhs.flags);

	setLogic(std::move(m_logic)); //reset logic pointer

	std::swap(m_entityId, rhs.m_entityId);
	Locator().RegisterEntity(this);

	rhs.destroy();
}


void HgEntity::destroy()
{
	if (m_entityId.isValid())
	{
		EventSystem::PublishEvent(EntityDestroyed(this, m_entityId));
		Locator().RemoveEntity(m_entityId);
	}
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
	if (parent.isValid()) {
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

REGISTER_EVENT_TYPE(EntityCreated)
REGISTER_EVENT_TYPE(EntityDestroyed)