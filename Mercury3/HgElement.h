#pragma once

#include <stdint.h>
#include <quaternion.h>

#include <HgTypes.h>
#include <HgShader.h>

#include <HgCamera.h>
#include <str_utils.h>

#ifdef __cplusplus
extern "C" {
#endif

	enum HgElementFlag {
		HGE_USED = 0x01, //used in scene graph
		HGE_ACTIVE = 0x02,
		HGE_HIDDEN = 0x04,
		HGE_UPDATED = 0x08,
		HGE_DESTROY = 0x10
	};

extern viewport view_port[];
extern HgCamera* _camera;
extern float* _projection;

typedef enum BlendMode {
	BLEND_NORMAL = 0,
	BLEND_ADDITIVE,
	BLEND_ALPHA
} BlendMode;

typedef struct RenderData {
	void (*renderFunc)(struct RenderData* render_data);
	void (*destroy)(struct RenderData* e);
	HgShader* shader;
	uint8_t blendMode;
} RenderData;

//typedef void(*SignalHandler)(int signum);
typedef void(*hgelement_function)(struct HgElement* e);
//typedef void (*hgelement_update_function)(struct HgElement* e, uint32_t tdelta); //strange warnings with this....

typedef struct HgElement_vtable {
	hgelement_function create;
	hgelement_function destroy;
	void (*updateFunc)(struct HgElement* e, uint32_t tdelta);
} HgElement_vtable;

#define MAX_ELEMENT_TYPES 255
typedef uint8_t vtable_index;
extern HgElement_vtable HGELEMT_VTABLES[MAX_ELEMENT_TYPES];
extern hgstring HGELEMENT_TYPE_NAMES;
extern uint32_t HGELEMENT_TYPE_NAME_OFFSETS[MAX_ELEMENT_TYPES];

/* NOTES: Try to avoid pointers, especially on 64 bit.
The entity that allocates memory for render data should
be responsible for destroying it. This is more clear than
having HgElement free render data by default and then
handling special cases.
*/
typedef struct HgElement{
//	HgElement_vtable* vptr;
	vtable_index vptr_idx; //1
	uint8_t flags; //1
	point position; float scale; //16
	point origin; //origin (0,0,0) in local space
	quaternion rotation; //16
	RenderData* m_renderData; //can be shared //4, whoever whoever populates this must clean it up.
	void* extraData; //whoever whoever populates this must clean it up.
} HgElement;

void init_hgelement(HgElement* element);
//void HgElement_destroy(HgElement* element);

//inline uint16_t check_flag(HgElement* element, uint8_t x) { return element->flags & x; }
//inline void set_flag(HgElement* element, uint16_t f) { element->flags |= f; }
//inline void clear_flag(HgElement* element, uint16_t f) { element->flags |= ~f; }

//inline uint16_t is_destroyed(HgElement* e) { return CHECK_FLAG(e, HGE_DESTROY); }

inline const char* hgelement_get_type_str(HgElement* e) { return HGELEMENT_TYPE_NAMES.str + HGELEMENT_TYPE_NAME_OFFSETS[e->vptr_idx]; }
vtable_index hgelement_get_type_index(char* type);

extern void* (*new_RenderData)();

#define CHECK_FLAG(e,x) ((e)->flags&(x))
#define CLEAR_FLAG(e,x) ((e)->flags &= ~(x))
#define SET_FLAG(e,x) ((e)->flags |= (x))

//#define VCALL(e,function,...) if (e && e->vptr && e->vptr->function) e->vptr->function(e,__VA_ARGS__)

#define VCALL(e,function,...) if (e->vptr->function) e->vptr->function(e,__VA_ARGS__)
#define VCALL_F(e,function,...) e->vptr->function(e,__VA_ARGS__)
#define SCALL(x,function,...) x->function(x,__VA_ARGS__)
#define VCALL_IDX(e,function,...) if (HGELEMT_VTABLES[e->vptr_idx].function) HGELEMT_VTABLES[e->vptr_idx].function(e,__VA_ARGS__)


vtable_index RegisterElementType(const char* c);

#define REGISTER_ELEMENT_TYPE(str) TestRegistration(str);

#ifdef _MSC_VER
#define REGISTER_LINKTIME( func ) \
	__pragma(comment(linker,"/export:_REGISTER_ELEMENT"#func)); \
	void REGISTER_ELEMENT##func() { VTABLE_INDEX = RegisterElementType(#func); HGELEMT_VTABLES[VTABLE_INDEX] = vtable; }
#else
#define REGISTER_LINKTIME( func ) \
	void __attribute__((constructor)) REGISTER##func() { TestRegistration(#func, &func); }
#endif

#define SAFE_FREE(ptr) if (NULL != ptr) { free(ptr); ptr=NULL; }
#define SAFE_DESTROY(func,ptr) if (NULL != ptr) { func(ptr); free(ptr); ptr=NULL; }

#ifdef __cplusplus
};
#endif