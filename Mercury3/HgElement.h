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

enum HgElementFlag {
	HGE_USED = 0x01, //used in scene graph
	HGE_ACTIVE = 0x02,
	HGE_HIDDEN = 0x04,
	HGE_UPDATED = 0x08,
	HGE_DESTROY = 0x10,
	HGE_UPDATE_TEXTURES = 0x20,
	HGE_TRANSPARENT = 0x40
};

//extern float* _projection;

class HgElement;
class model_data;
class HgScene;

class HgElementLogic {
public:
	HgElementLogic() : element(nullptr) {}
	virtual ~HgElementLogic() {}
	virtual void update(HgTime tdelta) = 0;

	inline void setElement(HgElement* x) { element = x; }
	inline HgElement& getElement() { return *element; }
	inline const HgElement& getElement() const { return *element; }
protected:
	HgElement* element; //just a weak pointer back to the parent
};

//class HgElementExtended {
//public:
//	//HgElement* owner; //what is this for?
//	//std::vector< HgTexture::TexturePtr > textures;
//
//	HgScene* m_scene; //Scene that element is a member of
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

struct ElementFlags {
	ElementFlags() :
		used(false), active(false), hidden(false), updated(false),
		destroy(false), /*update_textures(false),*/ transparent(false),
		inheritParentScale(true), inheritParentRotation(true),
		inheritParentTranslation(true)
	{}
	bool used : 1; //used in scene graph
	bool active : 1;
	bool hidden : 1;
	bool updated : 1;
	bool destroy : 1;
	//bool update_textures : 1;
	bool transparent : 1;
	bool inheritParentScale : 1;
	bool inheritParentRotation : 1;
	bool inheritParentTranslation : 1;
};

class SpacialData {
private:
	quaternion m_orientation; //16
	vertex3f m_position; float m_scale; //16
	vertex3f m_origin;
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


/* NOTES: Try to avoid pointers, especially on 64 bit.
The entity that allocates memory for render data should
be responsible for destroying it. This is more clear than
having HgElement free render data by default and then
handling special cases.
*/
class HgElement {
private:
	SpacialData m_spacialData; //local transormations
public:
		ElementFlags flags;

		HgElement() : m_updateNumber(0), m_renderData(nullptr) {}
		~HgElement();

		void init();
		void destroy();

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
			if ((m_parent != nullptr) && m_parent->needsUpdate(updateNumber)) m_parent->update(dtime, updateNumber);
			m_logic->update(dtime);
		}

		inline void setLogic(std::unique_ptr<HgElementLogic> logic) { m_logic = std::move(logic); m_logic->setElement(this); }
		inline HgElementLogic& logic() { return *(m_logic.get()); }

		//Send texture data to GPU. I don't like this here and it pulls in extended data.
		//void updateGpuTextures();

		inline void setParent(HgElement* parent) { m_parent = parent; }
		HgElement* getParent() const { return m_parent; }

		inline void setChild(HgElement* child) { child->setParent(this); }

		inline void setRenderData(std::shared_ptr<RenderData>& rd) { m_renderData = rd; }
		RenderData* renderData() { return m_renderData.get(); }
		RenderDataPtr& getRenderDataPtr() { return m_renderData; }

		//inline void setScene(HgScene* s) { m_extendedData->m_scene = s; }

		HgMath::mat4f computeWorldSpaceMatrix(bool scale = true, bool rotation = true, bool translation = true) const;

		inline void inheritParentScale(bool x) { flags.inheritParentScale = x; }
		inline void inheritParentRotation(bool x) { flags.inheritParentRotation = x; }
		inline void inheritParentTranslation(bool x) { flags.inheritParentTranslation = x; }


private:
	inline bool hasLogic() const { return m_logic != nullptr; }

	std::shared_ptr<RenderData> m_renderData;

	uint32_t m_updateNumber;
	std::unique_ptr<HgElementLogic> m_logic;
	//std::weak_ptr<HgElement> m_parent;
	HgElement* m_parent; //checked every update.

	friend HgElementLogic;
	friend model_data;
};

typedef HgElement HgEntity;

//Transform point p into world space of HgElement e
point toWorldSpace(const HgElement* e, const point* p);

class IUpdatableCollection;

namespace Engine {
	std::vector<IUpdatableCollection*>& collections();
	void updateCollections(HgTime dtime);
	void EnqueueForRender(std::vector<IUpdatableCollection*>& c);
}

extern RenderData* (*new_RenderData)();

typedef void*(*factory_clbk)(HgElement* e);

void RegisterElementType(const char* c, factory_clbk);

#define REGISTER_ELEMENT_TYPE(str) TestRegistration(str);

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
	__pragma(comment(linker,"/export:"##LINKER_PREFIX##"REGISTER_ELEMENT"#func)); \
	extern "C" { void REGISTER_ELEMENT##func() { RegisterElementType(#func,factory); } }
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
