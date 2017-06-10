#pragma once

#include <HgTypes.h>
#include <oglDisplay.h>

typedef struct vbo_layout1 {
	vertex v;
	color c;
} vbo_layout1;

typedef struct HgVboMemory{
	vbo_layout1* buffer;

	uint32_t size;
	GLuint vbo_id;
	GLuint vao_id;
	uint8_t needsUpdate;
} HgVboMemory;

uint32_t hgvbo_add_data(HgVboMemory* vbo_mem, vertex* vertices, color* color, uint16_t vertex_count);
void hgvbo_sendToOGL(HgVboMemory* vbo_mem);
void hgvbo_use(HgVboMemory* vbo);

extern HgVboMemory staticVbo;