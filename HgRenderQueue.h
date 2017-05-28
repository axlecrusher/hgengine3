#pragma once

#include <glew.h>
#include <quaternion.h>
#include <stdint.h>

#include <HgElement.h>

#include <HgTypes.h>

#include <Windows.h>

typedef struct render_packet {
	point poisition;
	quaternion rotation;
	HgCamera camera;
	
	HgElement* element;
	uint8_t viewport_idx;
} render_packet;

int8_t draw_render_packet(const render_packet* p);

typedef struct HgRenderQueue {
	render_packet* rp;
	struct HgRenderQueue* next;
} HgRenderQueue;

typedef struct render_queue {
	volatile HgRenderQueue* queue_head;
	volatile HgRenderQueue* queue_tail;
	volatile uint32_t count;
	volatile LONG wait;
} render_queue;

volatile uint32_t hgRenderQueue_length();
void hgRenderQueue_push(render_packet* p);
render_packet* hgRenderQueue_pop();

render_packet* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera);

