#pragma once

#include <glew.h>
#include <quaternion.h>
#include <stdint.h>

#include <HgElement.h>

#include <HgTypes.h>
#include <windows.h>

#include <HgScene.h>

/* render_packet should contain everything that is needed to render.
This is to decouple render data from HgEntity data and update loops.
WHen the update loop is running async to the render thread, it is entirely possible
for HgEntity to be deleted before its last state was rendered.  Rendering should
be possible with render_packet, unless something in RenderData was destroyed when
deleting HgEntity.
*/
typedef struct render_packet {
	point position;
	quaternion rotation;
	HgCamera camera;
	float scale;
	
//	HgEntity* element;
//	uint16_t element_idx;
//	HgScene* scene;
	RenderData* renderData;
	uint8_t viewport_idx;
} render_packet;

int8_t draw_render_packet(const render_packet* p);

volatile uint32_t hgRenderQueue_length();
void hgRenderQueue_push(render_packet* p);
render_packet* hgRenderQueue_pop();

render_packet* create_render_packet(HgEntity* e, uint8_t viewport_idx, HgCamera* camera, HgScene* scene, uint16_t idx);

