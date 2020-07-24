#pragma once

#include <stdint.h>
#include <vector>
#include <deque>

class EntityIdType
{
public:
	static const uint32_t INDEX_BITS = 22; //~4 million
	static const uint32_t INDEX_MASK = (1 << INDEX_BITS) - 1; //~4 million
	static const uint32_t GENERATION_MASK = (1 << 8) - 1;

	EntityIdType(uint32_t id = 0)
		:m_id(id)
	{}

	EntityIdType(const EntityIdType&) = default;
	EntityIdType(EntityIdType &&rhs)
	{
		m_id = rhs.m_id;
		rhs.m_id = 0;
	}

	EntityIdType operator=(const EntityIdType& rhs)
	{
		m_id = rhs.m_id;
		return *this;
	}

	operator uint32_t() const { return m_id; }
	EntityIdType& operator++() { ++m_id; return *this; } //pre
	EntityIdType operator++(int)
	{
		auto tmp = *this;
		++m_id;
		return tmp;
	}	//post

	//bool isValid() const { return m_id > 0; }

	inline int32_t index() const { return (INDEX_MASK & m_id); }
	inline int32_t generation() const { return (m_id >> INDEX_BITS) & GENERATION_MASK; }

private:
	uint32_t m_id;
};

using EntityIdList = std::vector<EntityIdType>;

class EntityIdTable
{
	//Based on http://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html
public:
	static const uint32_t MAX_ENTRIES = (1 << EntityIdType::INDEX_BITS);
	//static const uint32_t MAX_ENTRIES = 5; //for testing create() assert
	EntityIdTable()
	{
		m_generation.reserve(MAX_ENTRIES);
		m_generation.emplace_back(0); // consider id 0 invalid

		m_entityTotal = 0;
		m_entityActive = 0;
	}

	//create a new entity id
	EntityIdType create();

	//Return a list of contiguous IDs
	EntityIdList createContiguous(uint32_t count);

	//check if an entity exists
	inline bool exists(EntityIdType id) const
	{
		const auto idx = id.index();
		if (idx == 0) return false;
		if (idx < m_generation.size())
		{
			return (m_generation[idx] == id.generation());
		}
		return false; //this should never happen
	}

	//destroy an entity
	void destroy(EntityIdType id);

	//total number of entites ever created
	uint32_t totalEntities() const { return m_entityTotal; }

	//current number of entities in existance
	uint32_t numberOfEntitiesExisting() const { return m_entityActive; }


	static EntityIdTable Manager;

private:
	inline EntityIdType combine_bytes(uint32_t idx, uint8_t generation)
	{
		const uint32_t id = (generation << EntityIdType::INDEX_BITS) | idx;
		return id;
	}

	bool hasIdPool() const { return m_freeIndices.size() > 1024; }

	//returns the index into m_freeIndices which starts a contiguous block of free ids. Negative indicates no free
	int32_t checkFreeContiguous(uint32_t count);

	std::vector<uint8_t> m_generation;

	/* I think a deque is better than a list here because there
	would be fewer heap allocaitons/fragmentation. */
	std::deque<uint32_t> m_freeIndices;

	uint32_t m_entityTotal; //total number of entities that have ever existed
	uint32_t m_entityActive; //number of entities currently in existance
};

namespace std
{
	template<> struct hash<EntityIdType>
	{
		typedef EntityIdType argument_type;
		typedef std::size_t result_type;
		result_type operator()(argument_type const& s) const noexcept
		{
			return std::hash<uint32_t>{}(s);
		}
	};
}