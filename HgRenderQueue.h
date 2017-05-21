#pragma once

#include <glew.h>
#include <quaternion.h>
#include <stdint.h>

#include <HgElement.h>

typedef struct render_packet {
	float position[3];
	quaternion rotation;
//	GLuint vao;
	HgElement* element;
} render_packet;

typedef struct HgRenderQueue {
	render_packet* rp;
	struct HgRenderQueue* next;
} HgRenderQueue;

volatile uint32_t hgRenderQueue_length();
void hgRenderQueue_push(render_packet* p);
HgRenderQueue* hgRenderQueue_pop();

