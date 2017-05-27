#pragma once

#include <glew.h>
#include <quaternion.h>
#include <stdint.h>

#include <HgElement.h>

#include <HgTypes.h>

class RenderPacket {
	public:
		virtual bool render() = 0;
	private:
};

class EndOfFrame : public RenderPacket {
	public:
		bool render();
};

class RenderElement : public RenderPacket {
public:
	float position[3];
	quaternion rotation;
	HgCamera camera;
	
	HgElement* element;
	uint8_t viewport_idx;

	bool render();
};
/*
typedef struct render_packet {
	float position[3];
	float cam_position[3];
	quaternion rotation;
	uint8_t eye;
//	GLuint vao;
	HgElement* element;
} render_packet;
*/
typedef struct HgRenderQueue {
	RenderPacket* rp;
	struct HgRenderQueue* next;
} HgRenderQueue;

volatile uint32_t hgRenderQueue_length();
void hgRenderQueue_push(RenderPacket* p);
HgRenderQueue* hgRenderQueue_pop();

RenderPacket* create_render_packet(HgElement* e, uint8_t viewport_idx, HgCamera* camera);

