#pragma once

#include <stdint.h>
#include <quaternion.h>

#include <HgTypes.h>
#include <HgShader.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HGE_USED	0x01 //used in scene graph
#define HGE_ACTIVE	0x02
#define HGE_HIDDEN	0x04
#define HGE_UPDATED	0x08
#define HGE_DESTROY	0x10

typedef enum BlendMode {
	BLEND_NORMAL = 0,
	BLEND_ADDITIVE
} BlendMode;

typedef struct RenderData {
	void (*renderFunc)(struct RenderData* render_data);
	void (*destroy)(struct RenderData* e);
	HgShader* shader;
	uint8_t blendMode;
} RenderData;

typedef struct HgElement_vtable {
	void(*destroy)(struct HgElement* e);
	void(*updateFunc)(struct HgElement* e, uint32_t tdelta);
} HgElement_vtable;

extern HgElement_vtable HGELEMT_VTABLES[255];
typedef uint8_t vtable_index;

//try to avoid pointers, especially on 64 bit
typedef struct HgElement{
//	HgElement_vtable* vptr;
	vtable_index vptr_idx; //1
	uint8_t flags; //1
//	uint32_t index;
//	uint32_t parent;
	point position; float scale; //16
	quaternion rotation; //16
	RenderData* m_renderData; //4
	void* extraData;
} HgElement;

void init_hgelement(HgElement* element);
//void HgElement_destroy(HgElement* element);

//inline uint16_t check_flag(HgElement* element, uint8_t x) { return element->flags & x; }
//inline void set_flag(HgElement* element, uint16_t f) { element->flags |= f; }
//inline void clear_flag(HgElement* element, uint16_t f) { element->flags |= ~f; }

//inline uint16_t is_destroyed(HgElement* e) { return CHECK_FLAG(e, HGE_DESTROY); }

extern void* (*new_RenderData)();

#define CHECK_FLAG(e,x) ((e)->flags&(x))
#define CLEAR_FLAG(e,x) ((e)->flags &= ~(x))
#define SET_FLAG(e,x) ((e)->flags |= (x))

//#define VCALL(e,function,...) if (e && e->vptr && e->vptr->function) e->vptr->function(e,__VA_ARGS__)
#define VCALL(e,function,...) if (e->vptr->function) e->vptr->function(e,__VA_ARGS__)
#define VCALL_F(e,function,...) e->vptr->function(e,__VA_ARGS__)

#define VCALL_IDX(e,function,...) if (HGELEMT_VTABLES[e->vptr_idx].function) HGELEMT_VTABLES[e->vptr_idx].function(e,__VA_ARGS__)


vtable_index RegisterElementType(const char* c);


#define REGISTER_ELEMENT_TYPE(str) TestRegistration(str);

#ifdef _MSC_VER
#define REGISTER_LINKTIME( func ) \
	__pragma(comment(linker,"/export:_REGISTER"#func)); \
	void REGISTER##func() { VTABLE_INDEX = RegisterElementType(#func); HGELEMT_VTABLES[VTABLE_INDEX] = vtable; }
#else
#define REGISTER_LINKTIME( func ) \
	void __attribute__((constructor)) REGISTER##func() { TestRegistration(#func, &func); }
#endif


#ifdef __cplusplus
};
#endif