#pragma once

#include <HgTypes.h>
#include <oglDisplay.h>
#include <assert.h>
#include <RenderBackend.h>

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

struct OGLVboId {
	GLuint vbo_id;
	GLuint vao_id;
};

union gpuHandles {
	struct OGLVboId ogl;
};

class HgVboBase {
public:

	//	Copy data straight into VBO. Ensure that data, is using the correct vbo layout for your data
	virtual uint32_t add_data(void* data, uint16_t vertex_count) = 0;

	//	Clear data from RAM. Will not upload new data to GPU until new data is added.
	virtual void clear() = 0;

	virtual void destroy() = 0;

	virtual void use() = 0;

	virtual void draw(uint32_t count, uint32_t offset) = 0;

	inline VBO_TYPE VboType() const { return m_type;  }
	inline auto& getHandle() { return handle; }

protected:
	VBO_TYPE m_type;
	gpuHandles handle;
};

template<typename T>
class HgVboMemory : public HgVboBase{
public:
	HgVboMemory();
	virtual ~HgVboMemory();

	uint32_t add_data(void* data, uint16_t vertex_count);
	void clear();
	void destroy();

	void use();

	virtual void draw(uint32_t count, uint32_t offset) { ::draw_vbo<T>(this, count, offset); }
	T* getBuffer() { return buffer;  }
	void setNeedsUpdate(bool x) { needsUpdate = x; }

	inline uint32_t getCount() const { return count; }

	virtual VBO_TYPE VboType() { return HgVboMemory<T>::Type(); }
	static constexpr inline uint8_t Stride() { return sizeof(T); }

	static constexpr VBO_TYPE Type() {
		//looks stupid but is compile time evaluated
		if (std::is_same<T, vbo_layout_vc>::value) { return VBO_VC; }
		if (std::is_same<T, vbo_layout_vn>::value) { return VBO_VN; }
		if (std::is_same<T, vbo_layout_vnu>::value) { return VBO_VNU; }
		if (std::is_same<T, vbo_layout_vnut>::value) { return VBO_VNUT; }
		if (std::is_same<T, uint8_t>::value) { return VBO_INDEX8; }
		if (std::is_same<T, uint16_t>::value) { return VBO_INDEX16; }
		if (std::is_same<T, color>::value) { return VBO_COLOR8; }
		return VBO_TYPE_INVALID;
	}

private:

	inline void use_common() {
		if (needsUpdate) {
			RENDERER->sendToGPU(this);
			needsUpdate = false;
		}

		RENDERER->bind(this);
	}

	T* buffer;

	uint32_t count;
	bool needsUpdate;

	T* HgVboMemory::resize(uint32_t count);
//	void sendToGPU();
};

template<typename T>
HgVboMemory<T>::HgVboMemory()
	:buffer(nullptr), count(0), needsUpdate(0)
{
	constexpr VBO_TYPE type = Type();
	m_type = type;
	memset(&handle, 0, sizeof(handle));
	static_assert(Type() != VBO_TYPE_INVALID, "Invalid VBO Type");
}

template<typename T>
HgVboMemory<T>::~HgVboMemory() {
	if (buffer != nullptr) free(buffer);
	buffer = nullptr;

	destroy();
}

template<typename T>
void HgVboMemory<T>::destroy() {
	RENDERER->destroy(this);
	handle.ogl.vao_id = handle.ogl.vbo_id = 0;
	clear();
}

template<typename T>
T* HgVboMemory<T>::resize(uint32_t count) {
	T* buf = (T*)realloc(buffer, count * sizeof(*buf));
	assert(buf != NULL);
	buffer = buf;

	return buf;
}

template<typename T>
void HgVboMemory<T>::clear() {
	if (buffer != nullptr) free(buffer);
	buffer = NULL;
	count = 0;
}

template<typename T>
uint32_t HgVboMemory<T>::add_data(void* data, uint16_t vertex_count) {
	T* buf = buffer = resize(count + vertex_count);
	buf = buf + count;

	memcpy(buf, data, sizeof(*buf)*vertex_count);

	uint32_t offset = count;
	count += vertex_count;
	needsUpdate = true;

	return offset;
}

template<typename T>
inline void draw_vbo(HgVboMemory<T>* vbo, uint32_t count, uint32_t offset) {}

//NOTE: THESE ARE INLINE. THEY NEED TO BE IN THE HEADER, NOT CPP
template<>
inline void draw_vbo(HgVboMemory<uint8_t>* vbo, uint32_t count, uint32_t offset) {
	//count is indice count
	glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_BYTE, 0, offset);
}

template<>
inline void draw_vbo(HgVboMemory<uint16_t>* vbo, uint32_t count, uint32_t offset) {
	//count is indice count
	glDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, 0, offset);
}


extern HgVboMemory<vbo_layout_vc> staticVbo;
extern HgVboMemory<vbo_layout_vnu> staticVboVNU;
extern HgVboMemory<vbo_layout_vnut> staticVboVNUT;