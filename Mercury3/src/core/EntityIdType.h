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

	EntityIdType(uint32_t index, uint32_t generation)
	{
		m_id = combine_bytes(index, generation);
	}

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

	bool isValid() const { return index() > 0; }

	inline int32_t index() const { return (INDEX_MASK & m_id); }
	inline int32_t generation() const { return (m_id >> INDEX_BITS) & GENERATION_MASK; }

	static inline EntityIdType combine_bytes(uint32_t idx, uint8_t generation)
	{
		const uint32_t id = (generation << EntityIdType::INDEX_BITS) | idx;
		return id;
	}

private:
	uint32_t m_id;
};

using EntityIdList = std::vector<EntityIdType>;

struct GenerationType
{
	static const uint8_t INVALID = 0;

	GenerationType(uint32_t v = INVALID)
		:value(v)
	{}

	bool operator==(GenerationType rhs) const { return value == rhs.value; }

	uint8_t value;
};

class EntityIdTable
{
	//Based on http://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html
public:
	static const uint32_t MAX_ENTRIES = (1 << EntityIdType::INDEX_BITS);
	//static const uint32_t MAX_ENTRIES = 5; //for testing create() assert
	EntityIdTable();

	//create a new entity id
	EntityIdType create();

	//Return a list of contiguous IDs
	EntityIdList createContiguous(uint32_t count);

	//check if an entity exists
	inline bool exists(uint32_t index, uint32_t generation) const
	{
		if ( index < m_generation.size() )
		{
			return (m_generation[index] == generation);
		}
		return false;
	}

	//check if an entity exists
	inline bool exists(EntityIdType id) const
	{
		const auto idx = id.index();
		const auto gen = id.generation();

		return exists(idx, gen);
	}

	//destroy an entity
	void destroy(EntityIdType id);

	//total number of entites ever created
	uint32_t totalEntities() const { return m_entityTotal; }

	//current number of entities in existance
	uint32_t numberOfEntitiesExisting() const { return m_entityActive; }

	static EntityIdTable& Singleton();

private:

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

template<typename T>
class EntityIdLookupTable
{
public:
	void store(EntityIdType id, const T& x)
	{
		const auto i = id.index();
		if (m_gen.size() <= i)
		{
			m_gen.resize(i + 1);
			m_data.resize(i + 1);
		}

		m_gen[i] = id.generation();
		m_data[i] = x;
	}

	bool lookup(EntityIdType id, T& out) const
	{
		const auto i = id.index();
		if (i < m_gen.size())
		{
			if (m_gen[i] == id.generation())
			{
				out = m_data[i];
				return true;
			}
		}
		return false;
	}

	bool lookup(EntityIdType id, const T*& out) const
	{
		const auto i = id.index();
		if (i < m_gen.size())
		{
			if (m_gen[i] == id.generation())
			{
				out = &m_data[i];
				return true;
			}
		}
		return false;
	}
	void invalidate(EntityIdType id)
	{
		T tmp;
		if (lookup(id, tmp))
		{
			m_gen[id.index()] = 0;
		}
	}

private:
	std::vector<GenerationType> m_gen;
	std::vector<T> m_data;
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