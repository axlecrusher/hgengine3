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

#define VBO_VC 0
//Vertex,Color
typedef struct vbo_layout_vc {
	vertex v;
	color c;
} vbo_layout_vc;

#define VBO_VN 1
typedef struct vbo_layout_vn {
	vertex v;
	normal c;
} vbo_layout_vn;

#define VBO_VNU 3
typedef struct vbo_layout_vnu {
	vertex v;
	normal c;
	uv_coord uv;
} vbo_layout_vnu;

typedef struct HgVboMemory{
	void* buffer;

	uint32_t count;
	GLuint vbo_id;
	GLuint vao_id;
	uint8_t needsUpdate;

	uint16_t size; //size of the structure pointed to by buffer, stride
	uint8_t type;
//	void(*send_to_ogl)(struct HgVboMemory* vbo);
} HgVboMemory;



uint32_t hgvbo_add_data_vc(HgVboMemory* vbo_mem, vertex* vertices, color* color, uint16_t vertex_count);
//void hgvbo_sendToOGL(HgVboMemory* vbo_mem);
void hgvbo_use(HgVboMemory* vbo_mem);
void hgvbo_init(HgVboMemory* vbo_mem, uint8_t vbo_type);

extern HgVboMemory staticVbo;