#pragma once

#include <cassert>
#include <EntityIdType.h>
#include <memory>

//static EntityIdTable SingletonInstance;

EntityIdTable& EntityIdTable::Singleton()
{
	static std::unique_ptr<EntityIdTable> instance;
	if (instance == nullptr)
	{
		instance = std::make_unique<EntityIdTable>();
	}

	return *instance.get();
}

EntityIdTable::EntityIdTable()
{
	m_generation.reserve(MAX_ENTRIES);
	m_generation.emplace_back(0); // consider id 0 invalid

	m_entityTotal = 0;
	m_entityActive = 0;
}

EntityIdType EntityIdTable::create()
{
	uint32_t idx = 0;
	if (hasIdPool())
	{
		idx = m_freeIndices.front();
		m_freeIndices.pop_front();
	}
	else
	{
		idx = (uint32_t)m_generation.size();
		assert(idx < MAX_ENTRIES);
		m_generation.push_back(1); //what happens when we reach 2^22 ?
	}

	m_entityTotal++;
	m_entityActive++;

	return combine_bytes(idx, m_generation[idx]);
}

int32_t EntityIdTable::checkFreeContiguous(uint32_t count)
{
	return -1;
}

EntityIdList EntityIdTable::createContiguous(uint32_t count)
{
	EntityIdList list;
	list.resize(count);

	if (hasIdPool())
	{
		if (checkFreeContiguous(count) >= 0 && false)
		{
			////check for contiguous free
			//idx = m_freeIndices.front();
			//m_freeIndices.pop_front();
			return list;
		}
	}

	//allocate entire new chunk at once
	uint32_t idx = (uint32_t)m_generation.size();	//beginning index
	m_generation.resize(idx + count, 1); //expand and initalize generation to 1

	for (uint32_t i = 0; i < count; i++)
	{
		list[i] = combine_bytes(idx + i, m_generation[idx]);
	}

	m_entityTotal += count;
	m_entityActive += count;


	return list;
}

void EntityIdTable::destroy(EntityIdType id)
{
	const auto idx = id.index();

	if (exists(id))
	{
		//increment generation to invalidate current generation in the wild.
		m_generation[idx]++;
		if (m_generation[idx] == 0)
		{
			m_generation[idx] = 1;
		}
		m_freeIndices.push_back(idx);
		m_entityActive--;
	}
}