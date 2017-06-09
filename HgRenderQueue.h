#pragma once

#include <glew.h>
#include <quaternion.h>
#include <stdint.h>

#include <HgElement.h>

#include <HgTypes.h>
#include <windows.h>

#include <HgScene.h>

typedef struct render_packet {
	point position;
	quaternion rotation;
	HgCamera camera;
	float scale;
	
//	HgElement* element;
	uint16_t element_idx;
	HgScene* scene;
	uint8_t viewport_idx;
} render_packet;

int8_t draw_render_packet(const render_packet* p);

volatile uint32_t hgRenderQueue_length();
void hgRenderQueue_push(render_packet* p);
render_packet* hgRenderQueue_pop();

render_packet* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera, HgScene* scene, uint16_t idx);

