#include <HgEntity.h>
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <assert.h>

#include <HgMath.h>
#include <unordered_map>

#include <HgTimer.h>

#include <UpdatableCollection.h>

#include <EventSystem.h>
#include <HgEngine.h>
#include <EntityIdType.h>

#include <OrderedVector.h>

EntityInfo* EntityInfoTable::getInfo(EntityIdType id)
{
	//What happens to EntityInfo if the map is rebalanced?
	EntityInfo& t = m_data[id]; //default construct if it doesn't exist
	return &t; //is this ok to do?
}

EntityIdLookupTable<fMatrix4>& getEntityMatrixTable()
{
	static EntityIdLookupTable<fMatrix4> table;
	return table;
}


EntityInfoTable& EntityInfoTable::Manager()
{
	static std::unique_ptr<EntityInfoTable> instance;
	if (instance == nullptr)
	{
		instance = std::make_unique<EntityInfoTable>();
	}

	return *instance.get();
}

PreviousPositionTable& PreviousPositionTable::Manager()
{
	static std::unique_ptr<PreviousPositionTable> instance;
	if (instance == nullptr)
	{
		instance = std::make_unique<PreviousPositionTable>();
	}

	return *instance.get();
}

RenderDataTable& RenderDataTable::Manager()
{
	static std::unique_ptr<RenderDataTable> instance;
	if (instance == nullptr)
	{
		instance = std::make_unique<RenderDataTable>();
	}

	return *instance.get();
}

EntityTable& EntityTable::Singleton()
{
	static std::unique_ptr<EntityTable> instance;
	if (instance == nullptr)
	{
		instance = std::make_unique<EntityTable>();
	}

	return *instance.get();
}

void RegisterEntityType(const char* c, factory_clbk factory) {
	Engine::entity_factories[c] = factory;
}

void EntityLocator::RegisterEntity(HgEntity* entity)
{
	std::lock_guard<std::mutex> m_lock(m_mutex);

	const auto id = entity->getEntityId();
	if (!EntityIdTable::Singleton().exists(id)) return;

	m_entities[id] = entity;
}

void EntityLocator::RemoveEntity(EntityIdType id)
{
	if (!EntityIdTable::Singleton().exists(id)) return;

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
	if (EntityIdTable::Singleton().exists(id))
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

//EntityLocator& HgEntity::Locator()
//{
//	static EntityLocator locator;
//	return locator;
//}

EntityLocator::SearchResult HgEntity::Find(EntityIdType id)
{
	EntityLocator::SearchResult r;
	r.entity = EntityTable::Singleton().getPtr(id);
	return r;
	//return Locator().Find(id);
}

namespace Events
{

	void UpdateSPIData::execute(SPI& spi) const
	{
		if (validFlags.orientation)
			spi.orientation = spiValues.orientation;

		if (validFlags.position)
			spi.position = spiValues.position;

		if (validFlags.scale)
			spi.scale = spiValues.scale;

		if (validFlags.origin)
			spi.origin = spiValues.origin;
	}

}

void RenderDataTable::insert(EntityIdType id, const RenderDataPtr& rd)
{
	const auto idx = id.index();

	if (m_entityGeneration.size() <= idx)
	{
		const uint32_t newSize = idx + 10000;
		m_entityGeneration.resize(newSize);
		m_renderData.resize(newSize);
	}

	m_entityGeneration[idx] = id.generation();
	m_renderData[idx] = rd;
}

uint32_t RenderDataTable::getRenderDataForEntities(EntityIdType* entityList, int32_t numEntities, EntityRDPair* out) const
{
	//This function seems to scale well with a huge number of renderable entities

	//If we had an ordered entity list ordered by entityid.index, we could cap the
	//loop at max size and make the if statement only check the generation

	const auto maxIndex = m_entityGeneration.size();
	uint32_t rc = 0;
	for (int i = 0; i < numEntities; i++)
	{
		const auto entityid = entityList[i];
		const auto idx = entityid.index();

		if (idx < maxIndex &&
			( m_entityGeneration[idx] == entityid.generation() ))
		{
			out[rc].entityId = entityid;
			out[rc].rd = m_renderData[idx].get();
			rc++;
		}
	}

	return rc;
}

void RenderDataTable::GarbageCollectInvalidEntities(EntityIdTable* idTable)
{
	const auto count = m_entityGeneration.size();
	for (int32_t i = 0; i < count; i++)
	{
		if (!idTable->exists(i, m_entityGeneration[i]))
		{
			m_renderData[i] = nullptr;
		}
	}
}

RenderDataPtr RenderDataTable::get(EntityIdType id) const
{
	auto idx = id.index();
	if ((idx < m_renderData.size())
		&& (m_entityGeneration[idx] == id.generation()))
	{
		return m_renderData[idx];
	}
	return nullptr;
}

void PreviousPositionTable::insert(EntityIdType id, vertex3f p)
{
	m_positions[id] = p;
}

bool PreviousPositionTable::get(EntityIdType id, vertex3f& pp)
{
	const auto itr = m_positions.find(id);
	if (itr == m_positions.end())
	{
		return false;
	}
	pp = itr->second;
	return true;
}

HgEntity::HgEntity()
{
}

void HgEntity::init(EntityIdType id)
{
	EntityIdTable::Singleton().destroy(m_entityId);

	EntityTable::Singleton().initEntity(id);

	if (EntityIdTable::Singleton().exists(id))
	{
		m_entityId = id;
	}
	else
	{
		m_entityId = EntityIdTable::Singleton().create();
	}

	//Locator().RegisterEntity(this);
	EventSystem::PublishEvent(Events::EntityCreated(this, m_entityId));
}

HgEntity::~HgEntity() {
	destroy();
}

//HgEntity::HgEntity(HgEntity &&rhs)
//{
//	m_spacialData = std::move(rhs.m_spacialData);
//	//m_renderData = std::move(rhs.m_renderData);
//	//m_logic = std::move(rhs.m_logic);
//	//m_parentId = std::move(rhs.m_parentId);
//	//m_updateNumber = std::move(rhs.m_updateNumber);
//	m_drawOrder = std::move(rhs.m_drawOrder);
//	flags = std::move(rhs.flags);
//
//	//setLogic(std::move(m_logic)); //reset logic pointer
//
//	std::swap(m_entityId, rhs.m_entityId);
//	Locator().RegisterEntity(this);
//
//	rhs.destroy();
//}


void HgEntity::destroy()
{
	EventSystem::PublishEvent(Events::EntityDestroyed(this, m_entityId));
	//Locator().RemoveEntity(m_entityId);

	//m_renderData.reset();
	//EntityIdTable::Singleton().destroy(m_entityId);
}

HgMath::mat4f HgEntity::computeWorldSpaceMatrix(const bool applyScale, bool applyRotation, bool applyTranslation) const {
	EntityIdType parentId = HgEntity::getParentId(m_entityId);

	if (parentId.isValid() && EntityIdTable::Singleton().exists(parentId))
	{
		return computeWorldSpaceMatrixIncludeParent(applyScale, applyRotation, applyTranslation);
	}
	else
	{
		return HgMath::computeTransformMatrix(m_spacialData.getSPI(), applyScale, applyRotation, applyTranslation);
	}
}

HgMath::mat4f HgEntity::computeWorldSpaceMatrixIncludeParent(const bool applyScale, bool applyRotation, bool applyTranslation) const {
	HgMath::mat4f modelMatrix = HgMath::computeTransformMatrix(m_spacialData.getSPI(), applyScale, applyRotation, applyTranslation);

	const auto flags = EntityTable::Singleton().getFlags(getEntityId());

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

void HgEntity::clone(HgEntity* other) const
{
	other->m_spacialData = m_spacialData;

	EntityTable::Singleton().clone(getEntityId(), other->getEntityId());

	auto tmp = getRenderDataPtr();
	other->setRenderData(tmp);
}

//void EntityTable::destroy(EntityIdType id)
//{
//	//	m_entities
//}

//Transform point p into world space of HgEntity e
//I'm not 100% sure this matches the functionality of computeWorldSpaceMatrix so remove for now
//point toWorldSpace(const HgEntity* e, const point* p) {
//	vector3 v1 = (*p - e->origin()).scale(e->scale()).rotate(e->orientation()) + e->position();
//	return v1;
//}

REGISTER_EVENT_TYPE(Events::EntityCreated)
REGISTER_EVENT_TYPE(Events::EntityDestroyed)


REGISTER_EVENT_TYPE(Events::UpdateSPIData)