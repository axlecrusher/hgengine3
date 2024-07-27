#include <EntityTable.h>
#include <HgEntity.h>

EntityIdType EntityTable::create()
{
	const auto id = EntityIdTable::Singleton().create();
	const auto idx = id.index();

	if (idx >= m_entities.size())
	{
		m_entities.resize(idx + 1000);
	}

	m_entities[idx].init(id);

	return id;
}

//create multiple entities from an EntityIdList
void EntityTable::createMultiple(const EntityIdList& list)
{
	allocateId(list.back());
	for (auto id : list)
	{
		allocateId(id);

		const auto idx = id.index();
		m_entities[idx].init(id);
	}
}

void EntityTable::store(const HgEntity& e)
{
	const auto id = e.getEntityId();
	if (EntityIdTable::Singleton().exists(id))
	{
		const auto idx = id.index();
		if (idx >= m_entities.size())
		{
			m_entities.resize(idx + 1000);
		}

		m_entities[idx] = e;
	}
}

HgEntity* EntityTable::getPtr(EntityIdTable* table, EntityIdType id)
{
	const auto exist = table->exists(id);
	const auto idx = id.index();

	if (exist)
	{
		if (idx < m_entities.size())
		{
			HgEntity* ptr = &m_entities[idx];
			if (ptr->getEntityId() == id)
			{
				return ptr;
			}
		}
	}

	return nullptr;
}