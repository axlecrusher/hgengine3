#pragma once

#include <HgTypes.h>
#include <oglDisplay.h>

/*	Interleaved vertex layout because it is faster to resize when adding
	more mesh data. New mesh data can just be appened to the end. If it
	were stored VVVNNNCCC, it would be much more difficult to add new
	mesh data as all other mesh data would need to be shifted around. I'm
	unsure of the impact taht interleaved data has on rendering performance.
	It doesn't seem any slower than separate VBO buffers for each type.

	We do gain performance from calling glVertexAttribPointer, glBindBuffer
	and glBindVertexArray less.
*/

/*	NOTE: We should try to keep 4 by alignment.
	Opengl wiki says "The alignment of any attribute's data should be no
	less than 4 bytes. So if you have a vec3 of GLushorts, you can't use
	that 4th component for a new attribute (such as a vec2 of GLbytes).
	If you want to pack something into that instead of having useless
	padding, you need to make it a vec4 of GLushorts."
*/

//Vertex,Color
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