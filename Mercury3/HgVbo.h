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

typedef enum VBO_TYPE {
	VBO_VC = 0,
	VBO_VN,
	VBO_VNU,
	VBO_INDEX8,
	VBO_INDEX16,
	VBO_COLOR8,
	VBO_TYPE_COUNT
} VBO_TYPE;


//Vertex,Color
typedef struct vbo_layout_vc {
	vertex v;
	color c;
} vbo_layout_vc;


//Vertex,Normal
typedef struct vbo_layout_vn {
	vertex v;
	normal n;
} vbo_layout_vn;

//Vertex,Normal,UV
typedef struct vbo_layout_vnu {
	vertex v;
	normal n;
	uv_coord uv;
} vbo_layout_vnu;

class HgVboBase {
public:

	virtual void init() = 0;

	//	Copy data straight into VBO. Ensure that data, is using the correct vbo layout for your data
	virtual uint32_t add_data(void* data, uint16_t vertex_count) = 0;

	//	Clear data from RAM. Will not upload new data to GPU until new data is added.
	virtual void clear() = 0;

	virtual void destroy() = 0;

	virtual void use() = 0;

	virtual void draw(uint32_t offset) = 0;
};

template<typename T>
class HgVboMemory : public HgVboBase{
public:
	HgVboMemory();
	virtual ~HgVboMemory();

	void init();
	uint32_t add_data(void* data, uint16_t vertex_count);
	void clear();
	void destroy();

	void use();

	virtual void draw(uint32_t offset) { ::ogl_draw_vbo<T>(this, offset); }
private:
	T* buffer;

	uint32_t count;
	GLuint vbo_id;
	GLuint vao_id;
	bool needsUpdate;

	uint16_t stride; //stride, size of the structure pointed to by buffer
	uint8_t type;

//	void(*send_to_ogl)(struct HgVboMemory* vbo);


private:
	T* HgVboMemory::resize(uint32_t count);
	void hgvbo_sendogl();

	friend void ogl_draw_vbo(HgVboMemory<T>* vbo, uint32_t offset);
};

template<typename T>
void ogl_draw_vbo(HgVboMemory<T>* vbo, uint32_t offset) {}

extern HgVboMemory<vbo_layout_vc> staticVbo;
extern HgVboMemory<vbo_layout_vnu> staticVboVNU;