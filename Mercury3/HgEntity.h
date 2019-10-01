#pragma once

#include <stdint.h>
#include <quaternion.h>

#include <HgTypes.h>
#include <HgShader.h>

#include <str_utils.h>
#include <memory>
#include <HgTexture.h>
#include <vector>

#include <HgTimer.h>
//#include <HgVbo.h>
#include <RenderData.h>

#include <unordered_map>
#include <ServiceLocator.h>
#include <GuardedType.h>

//#include <core/HgScene2.h>

enum HgEntityFlag {
	HGE_USED = 0x01, //used in scene graph
	HGE_ACTIVE = 0x02,
	HGE_HIDDEN = 0x04,
	HGE_UPDATED = 0x08,
	HGE_DESTROY = 0x10,
	HGE_UPDATE_TEXTURES = 0x20,
	HGE_TRANSPARENT = 0x40
};

//extern float* _projection;

class HgEntity;
class model_data;
class HgScene;

class HgEntityLogic {
public:
	HgEntityLogic() : m_entity(nullptr) {}
	virtual ~HgEntityLogic() {}
	virtual void update(HgTime tdelta) = 0;

	inline void setEntity(HgEntity* x) { m_entity = x; }
	inline HgEntity& getEntityt() { return *m_entity; }
	inline const HgEntity& getEntity() const { return *m_entity; }
protected:
	HgEntity* m_entity; //just a weak pointer back to the parent
};

//class HgEntityExtended {
//public:
//	//HgEntity* owner; //what is this for?
//	//std::vector< HgTexture::TexturePtr > textures;
//
//	HgScene* m_scene; //Scene that entity is a member of
//
//	bool m_ownRenderData;
//};

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

class SpacialData {
private:
	quaternion m_orientation; //16 bytes
	vertex3f m_position; float m_scale; //16 bytes
	vertex3f m_origin; //12 bytes
public:
	SpacialData() : m_scale(1.0f) {}

	inline const vertex3f origin() const { return m_origin; }
	inline void origin(const vertex3f& p) { m_origin = p; }

	//inline vertex3f& position() { return m_position; }
	inline const vertex3f& position() const { return m_position; }
	inline void position(const vertex3f& p) { m_position = p; }

	//inline quaternion& orientation() { return m_orientation; }
	inline const quaternion& orientation() const { return m_orientation; }
	inline void orientation(const quaternion& q) { m_orientation = q; }

	inline float scale() const { return m_scale; }
	inline void scale(float s) { m_scale = s; }
};

//typedef uint32_t EntityIdType;
class EntityIdType
{
public:
	EntityIdType(uint32_t id = 0)
		:m_id(id)
	{}

	EntityIdType(const EntityIdType&) = default;
	EntityIdType(EntityIdType &&rhs)
	{
		m_id = rhs.m_id;
		rhs.m_id = 0;
	}

	EntityIdType& operator=(const EntityIdType& rhs)
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

	bool isValid() const { return m_id > 0; }
private:
	uint32_t m_id;
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

class ExtendedEntityData
{
public:
	void setName(std::string name) { m_name = std::move(name); }
	const std::string& getName() const { return m_name; }
private:
	std::string m_name;
};

/* NOTES: Try to avoid pointers, especially on 64 bit.
The entity that allocates memory for render data should
be responsible for destroying it. This is more clear than
having HgEntity free render data by default and then
handling special cases.
*/
class HgEntity {
public:
		HgEntity()
			: m_updateNumber(0), m_renderData(nullptr)
		{}

		~HgEntity();

		HgEntity(const HgEntity &other) = delete;
		HgEntity(HgEntity &&); //move operator

		void init();
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

		inline bool isRenderable() const { return m_renderData != nullptr; }
		inline void render() { if (isRenderable()) m_renderData->render();  }

		inline bool needsUpdate(uint32_t updateNumber) const { return (hasLogic() && (m_updateNumber != updateNumber)); }
		inline void update(HgTime dtime, uint32_t updateNumber) {
			m_updateNumber = updateNumber;
			//require parents to be updated first
			auto parent = getParent();
			if ((parent.isValid()) && parent->needsUpdate(updateNumber)) parent->update(dtime, updateNumber);
			m_logic->update(dtime);
		}

		inline void setLogic(std::unique_ptr<HgEntityLogic> logic) { m_logic = std::move(logic); if (m_logic) m_logic->setEntity(this); }
		inline HgEntityLogic& logic() { return *(m_logic.get()); }

		//Send texture data to GPU. I don't like this here and it pulls in extended data.
		//void updateGpuTextures();

		inline void setParent(HgEntity* parent) { m_parentId = parent->getEntityId(); }
		
		inline EntityLocator::SearchResult getParent() const
		{
			EntityLocator::SearchResult r;
			if (m_parentId.isValid()) r = Find(m_parentId);
			return r;
		}

		inline void setChild(HgEntity* child) { child->setParent(this); }

		inline void setRenderData(std::shared_ptr<RenderData>& rd) { m_renderData = rd; }
		RenderData* renderData() { return m_renderData.get(); }
		RenderDataPtr& getRenderDataPtr() { return m_renderData; }
		const RenderDataPtr& getRenderDataPtr() const { return m_renderData; }

		//inline void setScene(HgScene* s) { m_extendedData->m_scene = s; }

		HgMath::mat4f computeWorldSpaceMatrix(bool scale = true, bool rotation = true, bool translation = true) const;
		point computeWorldSpacePosition() const;

		inline void inheritParentScale(bool x) { flags.inheritParentScale = x; }
		inline void inheritParentRotation(bool x) { flags.inheritParentRotation = x; }
		inline void inheritParentTranslation(bool x) { flags.inheritParentTranslation = x; }

		//Lower numbers draw first. default draw order is 0
		inline void setDrawOrder(int8_t order) { m_drawOrder = order; }
		inline int8_t getDrawOrder() const { return m_drawOrder; }

		inline void setName(const std::string& name) { m_extendedData->setName(name); }
		inline std::string& getName() const { m_extendedData->getName(); }

		/*	Find an existing entity by id. Returned pointer is managed, do not delete.
			Return nullptr if the entity does not exist.
		*/
		static EntityLocator::SearchResult Find(EntityIdType id);
private:
	inline bool hasLogic() const { return m_logic != nullptr; }

	static EntityIdType m_nextEntityId;
	static EntityLocator& Locator();

	SpacialData m_spacialData; //local transormations
	EntityIdType m_entityId;

	RenderDataPtr m_renderData;
	std::unique_ptr<HgEntityLogic> m_logic;
	std::unique_ptr<ExtendedEntityData> m_extendedData; //data we don't really care about and access infrequently

	EntityIdType m_parentId;
	uint32_t m_updateNumber;
	int8_t m_drawOrder;
public:
	EntityFlags flags;
};

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
	extern "C" { void REGISTER_ENTITY2##func() { Engine::HgScene::RegisterEntityFactory(#func, Engine::generate_entity<type::InstanceCollection>); } }
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
