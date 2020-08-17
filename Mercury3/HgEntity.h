#pragma once

#include <stdint.h>
#include <quaternion.h>

#include <HgTypes.h>
#include <HgShader.h>

#include <memory>
#include <HgTexture.h>
#include <vector>

#include <HgTimer.h>
//#include <HgVbo.h>
#include <RenderData.h>

#include <unordered_map>
//#include <ServiceLocator.h>
#include <GuardedType.h>
#include <deque>
#include <stdint.h>
#include <unordered_set>
#include <EntityIdType.h>
#include <TransformManager.h>

//#include <core/HgScene2.h>
#include <HgScene2.h> //REGISTER_LINKTIME2 needs this
class HgEntity;
class model_data;
class HgScene;

struct PositionalData {
	//position, and rotation are in global coordinate system
	point position; float scale; //16
	point origin; //origin (0,0,0) in local space
	quaternion rotation; //16
	uint8_t flags; //1
};

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

struct SPI
{
	SPI()
		:scale(1.0)
	{}

	quaternion orientation; //16 bytes
	vertex3f position; float scale; //16 bytes
	vertex3f origin; //12 bytes
};



class SpacialData {
private:
	SPI m_spi;
public:
	SpacialData()
	{}

	inline const vertex3f origin() const { return m_spi.origin; }
	inline void origin(const vertex3f& p) { m_spi.origin = p; }

	//inline bool hasPrevious() const { return m_hasPrevious; }
	//inline const vertex3f& PreviousPosition() const { return m_prevPosition; }
	//inline void PreviousPosition(const vertex3f& p) { m_hasPrevious = true; m_prevPosition = p; }

	//inline vertex3f& position() { return m_position; }
	inline const vertex3f& position() const { return m_spi.position; }
	inline void position(const vertex3f& p) { m_spi.position = p; }

	//inline quaternion& orientation() { return m_orientation; }
	inline const quaternion& orientation() const { return m_spi.orientation; }
	inline void orientation(const quaternion& q) { m_spi.orientation = q; }

	inline float scale() const { return m_spi.scale; }
	inline void scale(float s) { m_spi.scale = s; }

	inline const SPI& getSPI() const { return m_spi; }
	inline SPI& getSPI() { return m_spi; }
};

class PreviousPositionTable
{
	static inline auto notFound() { return std::numeric_limits<int32_t>::max(); }

public:
	void insert(EntityIdType id, vertex3f p);
	bool get(EntityIdType id, vertex3f& pp);

	static PreviousPositionTable& Manager();
private:

	std::unordered_map<EntityIdType, vertex3f> m_positions;
	//std::vector<EntityIdType> m_parents;
};

//typedef uint32_t EntityIdType;

namespace Events
{
	class UpdateSPIData
	{
	public:
		void execute(SPI& spi) const;

		struct ValidityFlags {
			ValidityFlags() :
				orientation(false), position(false), scale(false), origin(false)
			{}

			bool orientation : 1;
			bool position : 1;
			bool scale : 1;
			bool origin : 1;
		};

		void setOrientation(quaternion o) { spiValues.orientation = o; validFlags.orientation = true; }
		void setPosition(vertex3f p) { spiValues.position = p; validFlags.position = true; }
		void setScale(float s) { spiValues.scale = s; validFlags.scale = true; }
		void setOrigin(vertex3f o) { spiValues.origin = o; validFlags.origin = true; }

		EntityIdType entityId;
		ValidityFlags validFlags;
		SPI spiValues;
	};
}

class EntityLocator
{
public:
	class SearchResult
	{
	public:
		SearchResult() : entity(nullptr)
		{}

		bool isValid() const { return entity != nullptr; }

		HgEntity* entity;
		HgEntity* operator->() { return entity; }
	};

	void RegisterEntity(HgEntity* entity);
	void RemoveEntity(EntityIdType id);

	SearchResult Find(EntityIdType id) const;
private:
	std::unordered_map<EntityIdType, HgEntity*> m_entities;
	mutable std::mutex m_mutex;
};

class EntityNameTable
{
	static inline auto notFound() { return std::numeric_limits<int32_t>::max(); }

public:
	void setName(EntityIdType id, const std::string name);
	const std::string& getName(EntityIdType id);

private:
	inline int32_t findName(EntityIdType id);

	struct storage
	{
		EntityIdType id;
		std::string name;

		//inline bool operator<(const storage& rhs) const { return id < rhs.id; }
	};

	std::unordered_map<EntityIdType, int32_t> m_indices;
	std::vector<storage> m_names;
	std::string m_blankName;
};

class EntityParentTable
{
	static inline auto notFound() { return std::numeric_limits<int32_t>::max(); }

public:
	void setParent(EntityIdType id, EntityIdType parentId);
	bool getParentId(EntityIdType id, EntityIdType& parent);

	static EntityParentTable& Manager();
private:

	std::unordered_map<EntityIdType, EntityIdType> m_parents;
	//std::vector<EntityIdType> m_parents;
};

struct EntityRDIdxPair
{
	EntityIdType entity;
	int32_t idx;
};

struct EntityRDPair; //forward declare

class RenderDataTable
{
	static inline auto notFound() { return std::numeric_limits<int32_t>::max(); }

public:
	using RenderDataId = int32_t;

	void insert(EntityIdType id, const RenderDataPtr& rd);
	RenderDataPtr get(EntityIdType id) const;

	//copies EntityRDPairs into [out] and returns number of entries
	uint32_t getRenderDataForEntities(EntityIdType* id, int32_t count, EntityRDPair* out) const;

	void GarbageCollectInvalidEntities(EntityIdTable* idTable);

	static RenderDataTable& Manager();
private:
	std::vector<uint8_t> m_entityGeneration; //generation of entity that stored the render data
	std::vector<RenderDataPtr> m_renderData;
};

//Compute local transformation matrix
HgMath::mat4f computeTransformMatrix(const SPI& sd, const bool applyScale = true, bool applyRotation = true, bool applyTranslation = true);

/* NOTES: Try to avoid pointers, especially on 64 bit.
The entity that allocates memory for render data should
be responsible for destroying it. This is more clear than
having HgEntity free render data by default and then
handling special cases.
*/
class HgEntity {
	public:
		HgEntity();
		~HgEntity();

		//HgEntity(const HgEntity &other) = delete;
		//HgEntity(HgEntity &&); //move operator

		void init(EntityIdType id = EntityIdType());
		void destroy();

		inline EntityIdType getEntityId() const { return m_entityId; }

		inline const point origin() const { return m_spacialData.origin(); }
		inline void origin(const point& p) { m_spacialData.origin(p); }

		//inline point& position() { return m_spacialData.position(); }
		inline const point& position() const { return m_spacialData.position(); }
		inline void position(const point& p) { m_spacialData.position(p); }

		//inline quaternion& orientation() { return m_spacialData.orientation(); }
		inline const quaternion& orientation() const { return m_spacialData.orientation(); }
		inline void orientation(const quaternion& q) { m_spacialData.orientation(q); }

		inline float scale() const { return m_spacialData.scale(); }
		inline void scale(float s) { m_spacialData.scale(s); }

		const SpacialData& getSpacialData() const { return m_spacialData; }
		SpacialData& getSpacialData() { return m_spacialData; }
		void setSpacialData(const SpacialData& x) { m_spacialData = x; }

		inline bool isRenderable() const { return getRenderDataPtr() != nullptr; }
		//inline void render() { if (isRenderable()) m_renderData->render();  }

		//inline bool needsUpdate(uint32_t updateNumber) const { return ((m_updateNumber != updateNumber)); }
		//inline void update(HgTime dtime, uint32_t updateNumber) {
		//	m_updateNumber = updateNumber;
		//	//require parents to be updated first
		//	auto parent = getParent();
		//	if ((parent.isValid()) && parent->needsUpdate(updateNumber)) parent->update(dtime, updateNumber);
		//	//m_logic->update(dtime);
		//}

		//Send texture data to GPU. I don't like this here and it pulls in extended data.
		//void updateGpuTextures();

		inline void setParent(EntityIdType id) { EntityParentTable::Manager().setParent(m_entityId, id); }
		
		inline EntityLocator::SearchResult getParent() const
		{
			EntityLocator::SearchResult r;

			EntityIdType parentId;
			if (EntityParentTable::Manager().getParentId(m_entityId, parentId))
			{
				r = Find(parentId);
			}

			return r;
		}

		//inline void setChild(HgEntity* child) { child->setParent(this); }

		inline void setRenderData(std::shared_ptr<RenderData>& rd) { RenderDataTable::Manager().insert(m_entityId, rd); }

		//RenderData* renderData() { return m_renderData.get(); }
		//const RenderData* renderData() const { return m_renderData.get(); }

		RenderDataPtr getRenderDataPtr() { return RenderDataTable::Manager().get(m_entityId); }
		const RenderDataPtr getRenderDataPtr() const { return RenderDataTable::Manager().get(m_entityId); }

		//inline void setScene(HgScene* s) { m_extendedData->m_scene = s; }

		HgMath::mat4f computeWorldSpaceMatrix(bool scale = true, bool rotation = true, bool translation = true) const;
		HgMath::mat4f computeWorldSpaceMatrixIncludeParent(bool scale = true, bool rotation = true, bool translation = true) const;
		point computeWorldSpacePosition() const;

		inline void setInheritParentScale(bool x) { flags.inheritParentScale = x; }
		inline void setInheritParentRotation(bool x) { flags.inheritParentRotation = x; }
		inline void setInheritParentTranslation(bool x) { flags.inheritParentTranslation = x; }

		inline void setDestroy(bool x) { flags.destroy = x; }
		inline void setHidden(bool x) { flags.hidden = x; }

		//Lower numbers draw first. default draw order is 0
		inline void setDrawOrder(int8_t order) { m_drawOrder = order; }
		inline int8_t getDrawOrder() const { return m_drawOrder; }

		inline void setName(const std::string& name) { EntityNames().setName(m_entityId, name); }
		inline std::string& getName() const { EntityNames().getName(m_entityId); }

		inline EntityFlags getFlags() const { return flags; }

		inline void clone(HgEntity* other) const
		{
			other->m_spacialData = m_spacialData;
			other->m_drawOrder = m_drawOrder;
			other->flags = flags;

			auto tmp = getRenderDataPtr();
			other->setRenderData(tmp);
		}

		/*	Find an existing entity by id. Returned pointer is managed, do not delete.
			Return nullptr if the entity does not exist.
		*/
		static EntityLocator::SearchResult Find(EntityIdType id);
private:

	static EntityNameTable& EntityNames();

	//static EntityLocator& Locator();

	//RenderDataPtr m_renderData;
	SpacialData m_spacialData; //local transormations
	EntityIdType m_entityId;

	int8_t m_drawOrder;

	EntityFlags flags;
};


class EntityTable
{
public:

	EntityTable()
	{
	}

	//create a new entity
	inline EntityIdType create()
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
	inline void createMultiple(const EntityIdList& list)
	{
		allocateId(list.back());
		for (auto id : list)
		{
			allocateId(id);

			const auto idx = id.index();
			m_entities[idx].init(id);
		}
	}

	inline void store(const HgEntity& e)
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

	inline HgEntity* getPtr(EntityIdTable* table, EntityIdType id)
	{
		if (table->exists(id))
		{
			const auto idx = id.index();
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

	inline HgEntity* getPtr(EntityIdType id)
	{
		return getPtr(&EntityIdTable::Singleton(), id);
	}

	//destroy an entity
	void destroy(EntityIdType id);

	static EntityTable& Singleton();

private:

	//allocate entity storage for id
	inline void allocateId(EntityIdType id)
	{
		const auto idx = id.index();
		if (idx >= m_entities.size())
		{
			m_entities.resize(idx + 1000);
		}
	}

	std::vector<HgEntity> m_entities;
};

namespace EntityHelpers
{
	//Create a contiguous block of entities. Allocates IDs and Entity storage.
	inline EntityIdList createContiguous(uint32_t count)
	{
		auto idList = EntityIdTable::Singleton().createContiguous(count);
		EntityTable::Singleton().createMultiple(idList);
		return idList;
	}

	inline EntityIdType createSingle()
	{
		return EntityTable::Singleton().create();
	}
}

namespace Events
{
	class EntityCreated
	{
	public:
		EntityCreated(HgEntity* e, EntityIdType id)
			:entity(e)
		{}
		HgEntity* entity;
		EntityIdType entityId;
	};

	class EntityDestroyed
	{
	public:
		EntityDestroyed(HgEntity* e, EntityIdType id)
			:entity(e), entityId(id)
		{}
		HgEntity* entity;
		EntityIdType entityId;
	};

}
//Transform point p into world space of HgEntity e
//point toWorldSpace(const HgEntity* e, const point* p);

class IUpdatableCollection;
class RenderQueue;


extern RenderData* (*new_RenderData)();

typedef void*(*factory_clbk)(HgEntity* e);

void RegisterEntityType(const char* c, factory_clbk);

#define REGISTER_ENTITY_TYPE(str) TestRegistration(str);

// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define LINKER_PREFIX
#else
#define LINKER_PREFIX _
#endif
#endif

// Check GCC
#if __GNUC__
//#if __x86_64__ || __ppc64__
//#define ENVIRONMENT64
//#else
//#define ENVIRONMENT32
//#endif
#define LINKER_PREFIX _
#endif

#ifdef _MSC_VER
#define REGISTER_LINKTIME( func, factory ) \
	__pragma(comment(linker,"/export:"##LINKER_PREFIX##"REGISTER_ENTITY"#func)); \
	extern "C" { void REGISTER_ENTITY##func() { RegisterEntityType(#func,factory); } }

#define REGISTER_LINKTIME2( func, type ) \
	__pragma(comment(linker,"/export:"##LINKER_PREFIX##"REGISTER_ENTITY2"#func)); \
	extern "C" { void REGISTER_ENTITY2##func() { Engine::HgScene::RegisterEntityFactory(#func, Engine::HgScene::generate_entity<type>); } }

#define REGISTER_LINKTIME3( func, type, CollectionType ) \
	__pragma(comment(linker,"/export:"##LINKER_PREFIX##"REGISTER_ENTITY3"#func)); \
	extern "C" { void REGISTER_ENTITY3##func() { Engine::HgScene::RegisterEntityFactory(#func, Engine::HgScene::generate_entity2<type, CollectionType>); } }

#else
#define REGISTER_LINKTIME( func ) \
	void __attribute__((constructor)) REGISTER##func() { TestRegistration(#func, &func); }
#endif

#ifdef _MSC_VER
//#define REGISTER_GLOBAL_DESTROY( func ) \
	//__pragma(comment(linker, "/export:_GLOBAL_DESTROY"#func)); \
	//void GLOBAL_DESTROY##func() { ##func(); }
#else
#define REGISTER_LINKTIME( func ) \
	void __attribute__((constructor)) REGISTER##func() { TestRegistration(#func, &func); }
#endif

#define SAFE_DESTROY(func,ptr) if (NULL != (ptr)) { func(ptr); free(ptr); ptr=NULL; }
