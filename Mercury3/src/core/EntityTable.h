#pragma once

#include <EntityIdType.h>
#include <spi.h>

class HgEntity; //forward declare

struct EntityFlags {
	EntityFlags() :
		used(false), active(false), hidden(false), updated(false),
		destroy(false), /*update_textures(false),*/
		inheritParentScale(true), inheritParentRotation(true),
		inheritParentTranslation(true)
	{}
	bool used : 1; //used in scene graph
	bool active : 1;
	bool hidden : 1;
	bool updated : 1;
	bool destroy : 1;
	//bool update_textures : 1;
	bool inheritParentScale : 1;
	bool inheritParentRotation : 1;
	bool inheritParentTranslation : 1;
};

struct fMatrix4
{
	float m[16];
};

class EntityTable
{
public:
	EntityTable() = default;

	//create a new entity
	EntityIdType create();

	//create multiple entities from an EntityIdList
	void createMultiple(const EntityIdList& list);

	void store(const HgEntity& e);

	HgEntity* getPtr(EntityIdTable* table, EntityIdType id);

	inline HgEntity* getPtr(EntityIdType id)
	{
		return getPtr(&EntityIdTable::Singleton(), id);
	}

	inline EntityFlags getFlags(EntityIdType id)
	{
		const auto idx = id.index();
		return m_flags[idx];
	}

	inline void setHidden(EntityIdType id, bool t)
	{
		const auto idx = id.index();
		m_flags[idx].hidden = t;
	}

	inline void setFlags(EntityIdType id, EntityFlags flags)
	{
		const auto idx = id.index();
		m_flags[idx] = flags;
	}

	inline int8_t getDrawOrder(EntityIdType id)
	{
		const auto idx = id.index();
		return m_drawOrder[idx];
	}

	inline void setDrawOrder(EntityIdType id, int8_t drawOrder)
	{
		const auto idx = id.index();
		m_drawOrder[idx] = drawOrder;
	}

	inline void clone(EntityIdType src, EntityIdType dest)
	{
		const auto sidx = src.index();
		const auto didx = dest.index();

		m_dirtyTransform[didx] = m_dirtyTransform[sidx];
		m_spi[didx] = m_spi[sidx];
		m_drawOrder[didx] = m_drawOrder[sidx];
		m_flags[didx] = m_flags[sidx];
		m_worldSpaceMatrix[didx] = m_worldSpaceMatrix[sidx];
	}

	void initEntity(EntityIdType id)
	{
		const auto idx = id.index();

		if (idx < m_entities.size())
		{
			m_dirtyTransform[idx] = false;
			m_spi[idx] = SPI();
			m_drawOrder[idx] = 0;
			m_flags[idx] = EntityFlags();
			memset(&m_worldSpaceMatrix[idx], 0, sizeof(float) * 16);
		}
	}


	//destroy an entity
	//void destroy(EntityIdType id);

	static EntityTable& Singleton();

private:

	//allocate entity storage for id
	inline void allocateId(EntityIdType id)
	{
		const auto idx = id.index();
		if (idx >= m_entities.size())
		{
			//m_entities.resize(idx + 1000);
			resize(idx + 1000);
		}
	}

	inline void resize(size_t newSize)
	{
		m_entities.resize(newSize);
		m_dirtyTransform.resize(newSize, false);
		m_spi.resize(newSize);
		m_drawOrder.resize(newSize, 0);
		m_flags.resize(newSize);
		m_worldSpaceMatrix.resize(newSize);
	}


	std::vector<HgEntity> m_entities;

	std::vector<bool> m_dirtyTransform;
	std::vector<SPI> m_spi;
	std::vector<int8_t> m_drawOrder;
	std::vector<EntityFlags> m_flags;
	std::vector<fMatrix4> m_worldSpaceMatrix;
};
