#pragma once

#include <stdint.h>
#include <quaternion.h>

#include <HgTypes.h>

#define HGE_USED	0x01 //used in scene graph
#define HGE_ACTIVE	0x02
#define HGE_HIDDEN	0x04
#define HGE_UPDATED	0x08
#define HGE_DESTROY	0x10

typedef struct RenderData {
	void (*renderFunc)(struct HgElement* e);
	void (*destroy)(struct HgElement* e);
} RenderData;

typedef struct HgElement_vtable {
	void(*updateFunc)(struct HgElement* e, uint32_t tdelta);
} HgElement_vtable;

typedef struct HgElement{
	HgElement_vtable* vptr;
	uint16_t flags;
//	uint32_t index;
//	uint32_t parent;
	point position;
	quaternion rotation;
	RenderData* m_renderData;
} HgElement;

void init_hgelement(HgElement* element);

inline uint16_t check_flag(HgElement* element, uint8_t x) { return element->flags & x; }
inline void set_flag(HgElement* element, uint16_t f) { element->flags |= f; }
inline void clear_flag(HgElement* element, uint16_t f) { element->flags |= ~f; }

inline uint16_t is_destroyed(HgElement* e) { return check_flag(e, HGE_DESTROY); }

#define VCALL(e,function,...) if (e->vptr && e->vptr->function) e->vptr->function(e,__VA_ARGS__)