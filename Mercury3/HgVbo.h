#pragma once

#include <memory>
#include <stdint.h>
#include <HgTypes.h>

//#include <HgTypes.h>
//#include <oglDisplay.h>
//#include <assert.h>
//#include <RenderBackend.h>

/*	Interleaved vertex layout because it is faster to resize when adding
	more mesh data. New mesh data can just be appened to the end. If it
	were stored VVVNNNCCC, it would be much more difficult to add new
	mesh data as all other mesh data would need to be shifted around. I'm
	unsure of the impact that interleaved data has on rendering performance.
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

static void* _currentVbo;

enum VBO_TYPE : uint8_t {
	VBO_TYPE_INVALID = 0,
	VBO_VC,
	VBO_VN,
	VBO_VNU,
	VBO_VNUT,
	VBO_INDEX8,
	VBO_INDEX16,
	VBO_COLOR8
};

enum VBO_USE_TYPE : uint8_t {
	VBO_DRAW_STATIC = 0,
	VBO_DRAW_DYNAMIC,
	VBO_DRAW_STREAM
};

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

typedef struct vbo_layout_vnut {
	vertex v;
	normal n;
	tangent tan;
	uv_coord uv;
} vbo_layout_vnut;

class IHgVbo {
public:
	IHgVbo() : m_useType(VBO_DRAW_STATIC) {}
	virtual ~IHgVbo() {}

	//	Copy data straight into VBO. Ensure that data, is using the correct vbo layout for your data
	virtual uint32_t add_data(void* data, uint16_t vertex_count) = 0;

	//	Clear data from RAM. Will not upload new data to GPU until new data is added.
	virtual void clear() = 0;

	virtual void destroy() = 0;

	virtual void use() = 0;

	virtual void draw(uint32_t count, uint32_t vertex_offset, uint32_t idx_offset) = 0;

	inline VBO_TYPE VboType() const { return m_type;  }

	inline void UseType(VBO_USE_TYPE t) { m_useType = t; }
	inline VBO_USE_TYPE UseType() const { return m_useType; }

	virtual void setNeedsUpdate(bool t) = 0;

	virtual void* getBuffer() = 0;
protected:
	VBO_TYPE m_type;
	VBO_USE_TYPE m_useType;
};

namespace HgVbo {
	//Factory Function
	template<typename T> std::unique_ptr<IHgVbo> Create();
}

extern std::shared_ptr<IHgVbo> staticVbo;
extern std::shared_ptr<IHgVbo> staticVboVNU;
extern std::shared_ptr<IHgVbo> staticVboVNUT;
extern std::shared_ptr<IHgVbo> staticIndice8;
extern std::shared_ptr<IHgVbo> staticIndice16;